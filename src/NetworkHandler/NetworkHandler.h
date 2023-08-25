#pragma once

#include <vector>

#include "Codec/Codec.h"
#include "DataFormat/DataFormat.h"
#include "NetworkHandlerDelegate/NetworkHandlerDelegate.h"
#include "NetworkMessage/NetworkMessage.h"
#include "UDPInterface/UDPInterface.h"
#include "optional/include/tl/optional.hpp"

struct ActiveNetworkMessage {
 private:
  std::shared_ptr<NetworkMessage> message;
  Endpoint endpoint;
  std::shared_ptr<Codec> codec;
  unsigned int message_id;
  unsigned int retries_left;

 public:
  ActiveNetworkMessage(std::shared_ptr<NetworkMessage> message,
                       Endpoint endpoint, std::shared_ptr<Codec> codec,
                       unsigned int message_id, unsigned int retries_left)
      : message(message),
        endpoint(endpoint),
        codec(codec),
        message_id(message_id),
        retries_left(retries_left) {}

  std::shared_ptr<DataObject::GenericValue> to_data_object() const {
    return message->to_data_object();
  }

  std::shared_ptr<NetworkMessage> get_network_message() const;

  Endpoint get_endpoint() const;

  std::shared_ptr<Codec> get_codec() const;

  unsigned int get_message_id() const;

  unsigned int get_retries_left() const;

  bool decrement_retries();
};

struct NetworkHandler {
 private:
  std::shared_ptr<NetworkHandlerDelegate> delegate =
      std::make_shared<EmptyNetworkHandlerDelegate>();
  std::shared_ptr<UDPInterface> udp_interface;
  DataFormat default_codec;
  std::vector<ActiveNetworkMessage> active_messages;
  unsigned int next_active_message_id = 0;

  unsigned int get_next_active_message_id();

  bool send_active_message(ActiveNetworkMessage& message);

  void send_active_messages();

  tl::optional<std::shared_ptr<Codec>> get_codec_from_incoming_message(
      IncomingMessage incoming_message) const;

  tl::optional<std::shared_ptr<DataObject::GenericValue>>
  get_data_object_from_incoming_message(IncomingMessage incoming_message,
                                        std::shared_ptr<Codec> codec) const;

  void on_received_acknowledgement(unsigned int message_id);

  void send_ack(unsigned int message_id, Endpoint& endpoint,
                std::shared_ptr<Codec> codec);

  void handle_decoded_message(
      std::shared_ptr<DataObject::GenericValue> decoded_message,
      Endpoint& endpoint, std::shared_ptr<Codec> codec);

  void handle_packet_reception();

 public:
  void send_message(std::shared_ptr<NetworkMessage> message, Endpoint endpoint,
                    unsigned int max_retries, std::shared_ptr<Codec> codec);

  void send_message(std::shared_ptr<NetworkMessage> message, Endpoint endpoint,
                    unsigned int max_retries = 100);

  void set_delegate(std::shared_ptr<NetworkHandlerDelegate> new_delegate);

  void set_udp_interface(std::shared_ptr<UDPInterface> new_interface);

  void set_default_codec(DataFormat codec_enum);

  void on_100_ms_passed();
};