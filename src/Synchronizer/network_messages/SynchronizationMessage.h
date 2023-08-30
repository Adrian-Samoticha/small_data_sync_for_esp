#pragma once

#include "../Synchronizer.h"
#include "NetworkHandler/UDPInterface/UDPInterface.h"
#include "NetworkMessage/NetworkMessage.h"
#include "Synchronizable/Synchronizable.h"

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
        // TODO: fix this
        data_object::create_string_value("sync"),
        data_object::create_string_value(synchronizable->get_name()),
        synchronizable->to_data_object(),
    });
  }

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