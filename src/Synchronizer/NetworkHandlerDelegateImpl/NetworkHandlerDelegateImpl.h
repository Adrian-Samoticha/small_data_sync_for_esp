#pragma once

#include "../Synchronizer.h"
#include "NetworkHandler/NetworkHandlerDelegate/NetworkHandlerDelegate.h"

struct NetworkHandlerDelegateImpl : public NetworkHandlerDelegate {
 private:
  std::shared_ptr<synchronizer::Synchronizer> synchronizer;

 public:
  NetworkHandlerDelegateImpl(
      std::shared_ptr<synchronizer::Synchronizer> synchronizer)
      : synchronizer(synchronizer) {}

  void on_message_received(IncomingDecodedMessage message) const {
    auto synchronizer_delegate = synchronizer->get_delegate();
    if (synchronizer_delegate != nullptr) {
      synchronizer_delegate->on_message_received(message);
    }

    if (message.message_type == MessageType::SYNC) {
      auto data = message.data_object;
      if (data->is_array()) {
        auto array_items = data->array_items().value();

        if (array_items->size() >= 3) {
          auto group_name_hash = array_items->at(0)->int_value().value_or(0);
          auto name = array_items->at(1)->string_value().value_or("");
          synchronizer->handle_synchronization_message(
              group_name_hash, message.sender, name, array_items->at(2));
        }
      }

      return;
    }

    if (message.message_type == MessageType::DEREG) {
      synchronizer->remove_endpoint(message.sender);

      return;
    }
  };

  void on_message_emitted(std::shared_ptr<NetworkMessage> message) const {
    auto synchronizer_delegate = synchronizer->get_delegate();
    if (synchronizer_delegate != nullptr) {
      synchronizer_delegate->on_message_emitted(message);
    }
  };

  void on_ack_received(unsigned int message_id) const {
    auto synchronizer_delegate = synchronizer->get_delegate();
    if (synchronizer_delegate != nullptr) {
      synchronizer_delegate->on_ack_received(message_id);
    }
  };

  void on_message_discarded(unsigned int message_id) const {
    auto synchronizer_delegate = synchronizer->get_delegate();
    if (synchronizer_delegate != nullptr) {
      synchronizer_delegate->on_message_discarded(message_id);
    }
  };

  void on_decode_failed(std::string error, std::shared_ptr<Codec> codec) const {
    auto synchronizer_delegate = synchronizer->get_delegate();
    if (synchronizer_delegate != nullptr) {
      synchronizer_delegate->on_decode_failed(error, codec);
    }
  };
};