#pragma once

#include "../MessageType/MessageType.h"
#include "DataObject/DataObject.h"
#include "NetworkMessage/NetworkMessage.h"
#include "interfaces//UDPInterface/UDPInterface.h"

struct IncomingDecodedMessage {
  udp_interface::Endpoint sender;
  std::shared_ptr<data_object::GenericValue> data_object;
  MessageType message_type;

  IncomingDecodedMessage(udp_interface::Endpoint sender,
                         std::shared_ptr<data_object::GenericValue> data_object,
                         MessageType message_type)
      : sender(sender), data_object(data_object), message_type(message_type) {}
};

struct NetworkHandlerDelegate {
  virtual void on_message_received(IncomingDecodedMessage message) const {};

  virtual void on_message_emitted(
      std::shared_ptr<NetworkMessage> message) const {};

  virtual void on_ack_received(unsigned int message_id) const {};

  virtual void on_message_discarded(unsigned int message_id) const {};

  virtual void on_decode_failed(std::string error,
                                std::shared_ptr<Codec> codec) const {};

  virtual ~NetworkHandlerDelegate() = default;
};

struct EmptyNetworkHandlerDelegate : public NetworkHandlerDelegate {};