#pragma once

#include <functional>
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
  udp_interface::Endpoint endpoint;
  std::shared_ptr<Codec> codec;
  unsigned int message_id;
  unsigned int retries_left;

 public:
  ActiveNetworkMessage(const std::shared_ptr<NetworkMessage> message,
                       const udp_interface::Endpoint endpoint,
                       const std::shared_ptr<Codec> codec,
                       unsigned int message_id, unsigned int retries_left)
      : message(message),
        endpoint(endpoint),
        codec(codec),
        message_id(message_id),
        retries_left(retries_left) {}

  std::shared_ptr<data_object::GenericValue> to_data_object() const {
    return message->to_data_object();
  }

  std::shared_ptr<NetworkMessage> get_network_message() const;

  udp_interface::Endpoint get_endpoint() const;

  std::shared_ptr<Codec> get_codec() const;

  unsigned int get_message_id() const;

  unsigned int get_retries_left() const;

  bool decrement_retries();
};

struct NetworkHandler {
 private:
  std::shared_ptr<NetworkHandlerDelegate> delegate =
      std::make_shared<EmptyNetworkHandlerDelegate>();
  std::shared_ptr<udp_interface::UDPInterface> udp_interface;
  DataFormat default_data_format;
  std::vector<ActiveNetworkMessage> active_messages;
  unsigned int next_active_message_id = 0;
  uint32_t time_in_deciseconds = 0;  // a decisecond is 100 ms
  std::map<std::pair<udp_interface::Endpoint, unsigned int>, uint32_t>
      message_receive_times;
  uint32_t max_message_receive_time_in_deciseconds = 600;

  unsigned int get_next_active_message_id();

  bool send_active_message(const ActiveNetworkMessage& message) const;

  void send_active_messages();

  tl::optional<std::shared_ptr<Codec>> get_codec_from_incoming_message(
      const udp_interface::IncomingMessage incoming_message) const;

  tl::optional<std::shared_ptr<data_object::GenericValue>>
  get_data_object_from_incoming_message(
      const udp_interface::IncomingMessage incoming_message,
      const std::shared_ptr<Codec> codec) const;

  void on_received_ack(const unsigned int message_id);

  void send_ack(const unsigned int message_id,
                const udp_interface::Endpoint& endpoint,
                const std::shared_ptr<Codec> codec) const;

  void handle_decoded_message(
      const std::shared_ptr<data_object::GenericValue> decoded_message,
      const udp_interface::Endpoint& endpoint,
      const std::shared_ptr<Codec> codec);

  void handle_packet_reception();

  tl::optional<uint32_t> get_message_receive_time(
      const udp_interface::Endpoint endpoint,
      const unsigned int message_id) const;

  void update_message_receive_time(const udp_interface::Endpoint endpoint,
                                   const unsigned int message_id);

  void remove_expired_message_receive_times();

 public:
  void send_message(const std::shared_ptr<NetworkMessage> message,
                    const udp_interface::Endpoint endpoint,
                    const unsigned int max_retries,
                    const std::shared_ptr<Codec> codec);

  void send_message(const std::shared_ptr<NetworkMessage> message,
                    const udp_interface::Endpoint endpoint,
                    const unsigned int max_retries = 100);

  void set_delegate(const std::shared_ptr<NetworkHandlerDelegate> new_delegate);

  void set_udp_interface(
      const std::shared_ptr<udp_interface::UDPInterface> new_interface);

  void set_default_data_format(const DataFormat new_default_data_format);

  void set_max_message_receive_time_in_deciseconds(const uint32_t new_max_time);

  void cancel_active_messages(
      const std::function<
          bool(const std::shared_ptr<data_object::GenericValue> info)>
          filter);

  void on_100_ms_passed();
  void heartbeat();
};