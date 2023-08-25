#pragma once

#include "../UDPInterface/UDPInterface.h"
#include "DataObject/DataObject.h"
#include "NetworkMessage/NetworkMessage.h"

struct IncomingDecodedMessage {
  Endpoint sender;
  std::shared_ptr<DataObject::GenericValue> data_object;

  IncomingDecodedMessage(Endpoint sender,
                         std::shared_ptr<DataObject::GenericValue> data_object)
      : sender(sender), data_object(data_object) {}
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