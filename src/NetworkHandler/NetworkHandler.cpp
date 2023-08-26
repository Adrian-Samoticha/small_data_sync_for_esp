#include "NetworkHandler.h"

#include "DataFormat/DataFormat_util.h"

std::shared_ptr<NetworkMessage> ActiveNetworkMessage::get_network_message()
    const {
  return message;
}

udp_interface::Endpoint ActiveNetworkMessage::get_endpoint() const {
  return endpoint;
}

std::shared_ptr<Codec> ActiveNetworkMessage::get_codec() const {
  return codec;
};

unsigned int ActiveNetworkMessage::get_message_id() const { return message_id; }

unsigned int ActiveNetworkMessage::get_retries_left() const {
  return retries_left;
}

bool ActiveNetworkMessage::decrement_retries() {
  if (retries_left == 0) {
    return false;
  }

  retries_left -= 1;

  return true;
}

unsigned int NetworkHandler::get_next_active_message_id() {
  auto to_be_returned = next_active_message_id;

  next_active_message_id = (next_active_message_id + 1) & 0xffffff;

  return to_be_returned;
}

bool NetworkHandler::send_active_message(ActiveNetworkMessage& message) {
  auto endpoint = message.get_endpoint();

  auto message_data_object = message.to_data_object();
  auto packet_data_object = data_object::create_array({
      data_object::create_string_value("msg"),
      data_object::create_number_value((int)message.get_message_id()),
      message_data_object,
  });

  auto codec = message.get_codec();
  auto serialized_packet = codec->encode(packet_data_object);

  auto format = codec->get_format();
  auto format_byte = get_format_byte_from_data_format(format);
  auto packet = (char)format_byte + serialized_packet;

  return udp_interface->send_packet(endpoint, packet);
}

void NetworkHandler::send_active_messages() {
  auto it = active_messages.begin();
  while (it != active_messages.end()) {
    send_active_message(*it);
    it->decrement_retries();

    if (it->get_retries_left() == 0) {
      it->get_network_message()->on_send_failed();
      delegate->on_message_discarded(it->get_message_id());

      it = active_messages.erase(it);
      continue;
    }

    it++;
  }
}

tl::optional<std::shared_ptr<Codec>>
NetworkHandler::get_codec_from_incoming_message(
    udp_interface::IncomingMessage incoming_message) const {
  if (incoming_message.data.length() == 0) {
    return {};
  }

  auto codec_byte = incoming_message.data.at(0);
  auto codec_optional = get_data_format_from_format_byte(codec_byte);

  if (codec_optional.has_value()) {
    auto codec_enum = codec_optional.value();

    return create_codec_from_format(codec_enum);
  }

  return {};
};

tl::optional<std::shared_ptr<data_object::GenericValue>>
NetworkHandler::get_data_object_from_incoming_message(
    udp_interface::IncomingMessage incoming_message,
    std::shared_ptr<Codec> codec) const {
  if (incoming_message.data.length() == 0) {
    return {};
  }

  auto encoding = incoming_message.data.substr(1);

  std::string error_string;
  auto decoded_message = codec->decode(encoding, error_string);
  if (!error_string.empty()) {
    delegate->on_decode_failed(error_string, codec);
    return decoded_message;
  }

  return decoded_message;
}

void NetworkHandler::on_received_acknowledgement(unsigned int message_id) {
  auto it = active_messages.begin();
  while (it != active_messages.end()) {
    if (it->get_message_id() == message_id) {
      it->get_network_message()->on_send_succeeded();
      delegate->on_ack_received(it->get_message_id());

      it = active_messages.erase(it);
      continue;
    }

    it++;
  }
}

void NetworkHandler::send_ack(unsigned int message_id,
                              udp_interface::Endpoint& endpoint,
                              std::shared_ptr<Codec> codec) {
  auto data = data_object::create_array({
      data_object::create_string_value("ack"),
      data_object::create_number_value((int)message_id),
  });
  auto encoded = codec->encode(data);
  auto format = codec->get_format();
  auto format_byte = get_format_byte_from_data_format(format);
  auto packet = (char)format_byte + encoded;
  udp_interface->send_packet(endpoint, packet);
}

void NetworkHandler::handle_decoded_message(
    std::shared_ptr<data_object::GenericValue> decoded_message,
    udp_interface::Endpoint& endpoint, std::shared_ptr<Codec> codec) {
  if (decoded_message->is_array()) {
    auto array_items = decoded_message->array_items().value();
    if (array_items->size() == 0) {
      return;
    }

    auto type = array_items->at(0)->string_value().value_or("");

    if (type == "ack") {
      if (array_items->size() < 2) {
        return;
      }
      auto message_id = array_items->at(1)->int_value().value_or(-1);
      if (message_id < 0) {
        return;
      }
      on_received_acknowledgement((unsigned int)message_id);

    } else if (type == "msg") {
      if (array_items->size() < 3) {
        return;
      }
      auto message_id = array_items->at(1)->int_value().value_or(-1);
      if (message_id < 0) {
        return;
      }

      send_ack(message_id, endpoint, codec);

      // TODO: prevent callback from being called multiple times for same
      // message
      auto decoded_message =
          IncomingDecodedMessage(endpoint, array_items->at(2));
      delegate->on_message_received(decoded_message);
    }
  }
}

void NetworkHandler::handle_packet_reception() {
  if (!udp_interface->is_incoming_packet_available()) {
    return;
  }

  auto incoming_message_optional = udp_interface->receive_packet();

  if (!incoming_message_optional.has_value()) {
    return;
  }

  auto incoming_message = incoming_message_optional.value();
  auto codec_optional = get_codec_from_incoming_message(incoming_message);

  if (!codec_optional.has_value()) {
    return;
  }

  auto codec = codec_optional.value();
  auto decoded_message_optional =
      get_data_object_from_incoming_message(incoming_message, codec);

  if (!decoded_message_optional.has_value()) {
    return;
  }

  auto decoded_message = decoded_message_optional.value();

  handle_decoded_message(decoded_message, incoming_message.endpoint, codec);
}

void NetworkHandler::send_message(std::shared_ptr<NetworkMessage> message,
                                  udp_interface::Endpoint endpoint,
                                  unsigned int max_retries,
                                  std::shared_ptr<Codec> codec) {
  auto active_network_message = ActiveNetworkMessage(
      message, endpoint, codec, get_next_active_message_id(), max_retries);
  active_messages.push_back(active_network_message);

  send_active_message(active_network_message);
}

void NetworkHandler::send_message(std::shared_ptr<NetworkMessage> message,
                                  udp_interface::Endpoint endpoint,
                                  unsigned int max_retries) {
  send_message(message, endpoint, max_retries,
               create_codec_from_format(default_codec));
}

void NetworkHandler::set_delegate(
    std::shared_ptr<NetworkHandlerDelegate> new_delegate) {
  delegate = new_delegate;
}

void NetworkHandler::set_udp_interface(
    std::shared_ptr<udp_interface::UDPInterface> new_interface) {
  udp_interface = new_interface;
}

void NetworkHandler::set_default_codec(DataFormat codec_enum) {
  default_codec = codec_enum;
}

void NetworkHandler::on_100_ms_passed() { send_active_messages(); }

void NetworkHandler::heartbeat() { handle_packet_reception(); }
