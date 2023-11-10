#include "NetworkHandler.h"

#include "DataFormat/DataFormat_util.h"
#include "MessageType/MessageType_util.h"

/**
 * Returns a data object containing any information contained in this message.
 */
std::shared_ptr<data_object::GenericValue>
ActiveNetworkMessage::to_data_object() const {
  return message->to_data_object();
}

/**
 * Returns the NetworkMessage.
 */
std::shared_ptr<NetworkMessage> ActiveNetworkMessage::get_network_message()
    const {
  return message;
}

/**
 * Returns the recipient of this ActiveNetworkMessage.
 */
udp_interface::Endpoint ActiveNetworkMessage::get_endpoint() const {
  return endpoint;
}

/**
 * Returns the codec.
 */
std::shared_ptr<Codec> ActiveNetworkMessage::get_codec() const {
  return codec;
};

/**
 * Returns the ID.
 */
unsigned int ActiveNetworkMessage::get_message_id() const { return message_id; }

/**
 * Returns how many retries are left before this message is discarded.
 */
unsigned int ActiveNetworkMessage::get_retries_left() const {
  return retries_left;
}

/**
 * Decrements the number of retries left.
 */
bool ActiveNetworkMessage::decrement_retries() {
  if (retries_left == 0) {
    return false;
  }

  retries_left -= 1;

  return true;
}

/**
 * Returns the ID of the next active message to send.
 */
unsigned int NetworkHandler::get_next_active_message_id() {
  const auto to_be_returned = next_active_message_id;

  next_active_message_id = (next_active_message_id + 1) & 0xffffff;

  return to_be_returned;
}

/**
 * Sends the given active message over the network.
 * Throws an exception if no UDP interface is provided.
 */
bool NetworkHandler::send_active_message(
    const ActiveNetworkMessage& message) const {
  const auto endpoint = message.get_endpoint();

  const auto message_data_object = message.to_data_object();
  const auto message_type = message.get_network_message()->get_message_type();
  const auto message_type_string = get_string_from_message_type(message_type);
  const auto packet_data_object = data_object::create_array({
      data_object::create_string_value(message_type_string),
      data_object::create_number_value((int)message.get_message_id()),
      message_data_object,
  });

  const auto codec = message.get_codec();
  const auto serialized_packet = codec->encode(packet_data_object);

  const auto format = codec->get_format();
  const auto format_byte = get_format_byte_from_data_format(format);
  const auto packet = (char)format_byte + serialized_packet;

  delegate->on_message_emitted(message.get_network_message());

  if (udp_interface == nullptr) {
    throw std::runtime_error(
        "Network handler was not provided a UDP interface.");
  }

  return udp_interface->send_packet(endpoint, packet);
}

/**
 * Sends all active messages that are currently queued for sending.
 */
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

/**
 * Returns the codec of an incoming message, if its format byte is valid.
 */
tl::optional<std::shared_ptr<Codec>>
NetworkHandler::get_codec_from_incoming_message(
    const udp_interface::IncomingMessage incoming_message) const {
  if (incoming_message.data.length() == 0) {
    return {};
  }

  const auto codec_byte = incoming_message.data.at(0);
  const auto codec_optional = get_data_format_from_format_byte(codec_byte);

  if (codec_optional.has_value()) {
    const auto codec_enum = codec_optional.value();

    return create_codec_from_format(codec_enum);
  }

  return {};
};

/**
 * Returns the data object representation of an incoming message, if decoding
 * succeeds.
 */
tl::optional<std::shared_ptr<data_object::GenericValue>>
NetworkHandler::get_data_object_from_incoming_message(
    const udp_interface::IncomingMessage incoming_message,
    const std::shared_ptr<Codec> codec) const {
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

/**
 * Called when an ack is received for a message. Removes the corresponding
 * active message.
 */
void NetworkHandler::on_received_ack(const unsigned int message_id) {
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

/**
 * Sends an ack for the given message ID to the given endpoint formatted with
 * the provided codec.
 * Throws an exception if no UDP interface is provided.
 */
void NetworkHandler::send_ack(const unsigned int message_id,
                              const udp_interface::Endpoint& endpoint,
                              const std::shared_ptr<Codec> codec) const {
  const auto data = data_object::create_array({
      data_object::create_string_value("ack"),
      data_object::create_number_value((int)message_id),
  });
  const auto encoded = codec->encode(data);
  const auto format = codec->get_format();
  const auto format_byte = get_format_byte_from_data_format(format);
  const auto packet = (char)format_byte + encoded;

  if (udp_interface == nullptr) {
    throw std::runtime_error(
        "Network handler was not provided a UDP interface.");
  }
  udp_interface->send_packet(endpoint, packet);
}

/**
 * Handles an incoming decoded message.
 */
void NetworkHandler::handle_decoded_message(
    const std::shared_ptr<data_object::GenericValue> decoded_message,
    const udp_interface::Endpoint& endpoint,
    const std::shared_ptr<Codec> codec) {
  if (decoded_message->is_array()) {
    const auto array_items = decoded_message->array_items().value();
    if (array_items->size() == 0) {
      return;
    }

    const auto type = array_items->at(0)->string_value().value_or("");

    if (type == "ack") {
      if (array_items->size() < 2) {
        return;
      }
      const auto message_id = array_items->at(1)->int_value().value_or(-1);
      if (message_id < 0) {
        return;
      }
      on_received_ack((unsigned int)message_id);

    } else if (type == "msg" || type == "sync" || type == "req_init_sync") {
      if (array_items->size() < 3) {
        return;
      }
      const auto message_id = array_items->at(1)->int_value().value_or(-1);
      if (message_id < 0) {
        return;
      }

      send_ack(message_id, endpoint, codec);

      const auto message_already_handled =
          get_message_reception_time(endpoint, message_id).has_value();

      update_message_reception_time(endpoint, message_id);

      const auto message_type =
          get_message_type_from_string(type.c_str()).value();

      if (!message_already_handled) {
        const auto decoded_message =
            IncomingDecodedMessage(endpoint, array_items->at(2), message_type);
        delegate->on_message_received(decoded_message);
      }
    }
  }
}

/**
 * Handles any incoming messages.
 * Throws an exception if no UDP interface is provided.
 */
void NetworkHandler::handle_packet_reception() {
  if (udp_interface == nullptr) {
    throw std::runtime_error(
        "Network handler was not provided a UDP interface.");
  }

  if (!udp_interface->is_incoming_packet_available()) {
    return;
  }

  const auto incoming_message_optional = udp_interface->receive_packet();

  if (!incoming_message_optional.has_value()) {
    return;
  }

  const auto incoming_message = incoming_message_optional.value();
  const auto codec_optional = get_codec_from_incoming_message(incoming_message);

  if (!codec_optional.has_value()) {
    return;
  }

  const auto codec = codec_optional.value();
  const auto decoded_message_optional =
      get_data_object_from_incoming_message(incoming_message, codec);

  if (!decoded_message_optional.has_value()) {
    return;
  }

  const auto decoded_message = decoded_message_optional.value();

  handle_decoded_message(decoded_message, incoming_message.endpoint, codec);
}

/**
 * Returns the reception time of the message with the given ID from the given
 * endpoint, if it exists and has not expired.
 */
tl::optional<uint32_t> NetworkHandler::get_message_reception_time(
    const udp_interface::Endpoint endpoint,
    const unsigned int message_id) const {
  const auto key = std::make_pair(endpoint, message_id);

  if (message_reception_times.count(key) == 0) {
    return {};
  }

  return message_reception_times.at(key);
}

/**
 * Updates the reception time of the message with the given ID from the given
 * endpoint.
 */
void NetworkHandler::update_message_reception_time(
    const udp_interface::Endpoint endpoint, const unsigned int message_id) {
  const auto key = std::make_pair(endpoint, message_id);
  message_reception_times[key] = time_in_deciseconds;
}

/**
 * Removes all message reception times that have expired.
 */
void NetworkHandler::remove_expired_message_reception_times() {
  auto it = message_reception_times.begin();
  while (it != message_reception_times.end()) {
    uint32_t age = time_in_deciseconds - it->second;
    if (age > max_message_reception_time_in_deciseconds) {
      it = message_reception_times.erase(it);
    } else {
      it++;
    }
  }
}

/**
 * Sends the given message and adds it to the list of active messages. The
 * message will be retried if it fails to be transmitted.
 */
void NetworkHandler::send_message(const std::shared_ptr<NetworkMessage> message,
                                  const udp_interface::Endpoint endpoint,
                                  const unsigned int max_retries,
                                  const std::shared_ptr<Codec> codec) {
  const auto active_network_message = ActiveNetworkMessage(
      message, endpoint, codec, get_next_active_message_id(), max_retries);
  active_messages.push_back(active_network_message);

  send_active_message(active_network_message);
}

/**
 * Sends the given message and adds it to the list of active messages. The
 * message will be retried if it fails to be transmitted.
 */
void NetworkHandler::send_message(const std::shared_ptr<NetworkMessage> message,
                                  const udp_interface::Endpoint endpoint,
                                  const unsigned int max_retries) {
  send_message(message, endpoint, max_retries,
               create_codec_from_format(default_data_format));
}

/**
 * Sets this NetworkHandler’s NetworkHandlerDelegate.
 */
void NetworkHandler::set_delegate(
    const std::shared_ptr<NetworkHandlerDelegate> new_delegate) {
  delegate = new_delegate;
}

/**
 * Provides this NetworkHandler with a UDP interface.
 */
void NetworkHandler::set_udp_interface(
    const std::shared_ptr<udp_interface::UDPInterface> new_interface) {
  udp_interface = new_interface;
}

/**
 * Sets this NetworkHandler’s default data format.
 */
void NetworkHandler::set_default_data_format(
    const DataFormat new_default_data_format) {
  default_data_format = new_default_data_format;
}

/**
 * Sets this NetworkHandler’s max message reception time in deciseconds.
 * Messages reception times expire after this duration.
 */
void NetworkHandler::set_max_message_reception_time_in_deciseconds(
    const uint32_t new_max_time) {
  max_message_reception_time_in_deciseconds = new_max_time;
}

/**
 * Cancels active messages that match the given filter.
 */
void NetworkHandler::cancel_active_messages(
    const std::function<
        bool(const std::shared_ptr<data_object::GenericValue> info)>
        filter) {
  auto it = active_messages.begin();
  while (it != active_messages.end()) {
    if (filter(it->get_network_message()->get_info())) {
      it->get_network_message()->on_cancelled();
      it = active_messages.erase(it);
    } else {
      it++;
    }
  }
}

/**
 * To be called once every 100 ms.
 */
void NetworkHandler::on_100_ms_passed() {
  time_in_deciseconds += 1;

  send_active_messages();

  if (time_in_deciseconds % 16 == 0) {
    remove_expired_message_reception_times();
  }
}

/**
 * To be called as frequently as possible.
 */
void NetworkHandler::heartbeat() { handle_packet_reception(); }
