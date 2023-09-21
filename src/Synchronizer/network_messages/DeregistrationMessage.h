#pragma once

#include "../Synchronizer.h"
#include "NetworkMessage/NetworkMessage.h"
#include "Synchronizable/Synchronizable.h"
#include "interfaces//UDPInterface/UDPInterface.h"

struct DeregistrationMessage : public NetworkMessage {
 public:
  DeregistrationMessage() {}

  std::shared_ptr<data_object::GenericValue> to_data_object() const override {
    return data_object::create_null_value();
  }

  MessageType get_message_type() const override { return MessageType::DEREG; }

  void on_send_succeeded() const override {}

  void on_send_failed() const override {}

  void on_cancelled() const override {}

  std::shared_ptr<data_object::GenericValue> get_info() const override {
    return data_object::create_array({
        data_object::create_string_value("dereg"),
    });
  }
};