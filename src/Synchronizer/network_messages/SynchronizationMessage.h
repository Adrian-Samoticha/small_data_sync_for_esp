#pragma once

#include "../Synchronizer.h"
#include "NetworkMessage/NetworkMessage.h"
#include "Synchronizable/Synchronizable.h"
#include "interfaces//UDPInterface/UDPInterface.h"

struct SynchronizationMessage : public NetworkMessage {
 private:
  std::shared_ptr<Synchronizable> synchronizable;
  udp_interface::Endpoint endpoint;
  std::shared_ptr<synchronizer::Synchronizer> synchronizer;

 public:
  SynchronizationMessage(
      std::shared_ptr<Synchronizable> synchronizable,
      udp_interface::Endpoint endpoint,
      std::shared_ptr<synchronizer::Synchronizer> synchronizer)
      : synchronizable(synchronizable),
        endpoint(endpoint),
        synchronizer(synchronizer) {}

  std::shared_ptr<data_object::GenericValue> to_data_object() const override {
    return data_object::create_array({
        data_object::create_number_value(synchronizer->get_group_name_hash()),
        data_object::create_string_value(synchronizable->get_name()),
        synchronizable->to_data_object(),
    });
  }

  MessageType get_message_type() const override { return MessageType::SYNC; }

  void on_send_succeeded() const override {}

  void on_send_failed() const override {
    synchronizer->remove_endpoint(endpoint);
  }

  void on_cancelled() const override {}

  std::shared_ptr<data_object::GenericValue> get_info() const override {
    return data_object::create_array({
        data_object::create_string_value("sync"),
        data_object::create_string_value(endpoint.to_string()),
        data_object::create_string_value(synchronizable->get_name()),
    });
  }
};