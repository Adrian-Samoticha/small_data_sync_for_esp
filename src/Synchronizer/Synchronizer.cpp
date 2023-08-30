#include "Synchronizer.h"

#include "NetworkHandlerDelegateImpl/NetworkHandlerDelegateImpl.h"
#include "network_messages/SynchronizationMessage.h"

namespace synchronizer {
bool Synchronizer::is_message_related_to_synchronizable(
    const std::shared_ptr<data_object::GenericValue> message_info,
    const std::shared_ptr<Synchronizable> synchronizable) const {
  if (!message_info->is_array()) {
    return false;
  }

  auto array_items = message_info->array_items().value();

  if (array_items->size() < 3) {
    return false;
  }

  if (array_items->at(0)->string_value().value_or("") != "sync") {
    return false;
  }

  if (array_items->at(2)->string_value().value_or("") !=
      synchronizable->get_name()) {
    return false;
  }

  return true;
}

void Synchronizer::init() {
  auto network_handler_delegate =
      std::make_shared<NetworkHandlerDelegateImpl>(shared_from_this());
  network_handler.set_delegate(network_handler_delegate);
}

std::shared_ptr<Synchronizer> Synchronizer::create() {
  auto result = std::make_shared<Synchronizer>();
  result->init();

  return result;
}

void Synchronizer::synchronize(
    const std::shared_ptr<Synchronizable> synchronizable) {
  network_handler.cancel_active_messages(
      [this,
       synchronizable](std::shared_ptr<data_object::GenericValue> data_object) {
        return is_message_related_to_synchronizable(data_object,
                                                    synchronizable);
      });

  for_each_endpoint(
      [synchronizable, this](const udp_interface::Endpoint endpoint) {
        const std::shared_ptr<NetworkMessage> message =
            std::make_shared<SynchronizationMessage>(synchronizable, endpoint,
                                                     shared_from_this());

        network_handler.cancel_active_messages(
            [this, synchronizable,
             message](std::shared_ptr<data_object::GenericValue> data_object) {
              return data_object == message->to_data_object();
            });

        network_handler.send_message(message, endpoint, 100u);
      });
}

void Synchronizer::handle_synchronization_message(
    const udp_interface::Endpoint endpoint,
    const std::string synchronizable_name,
    std::shared_ptr<data_object::GenericValue> data_object) {
  if (!is_endpoint_known(endpoint)) {
    const auto initial_container =
        delegate->create_initial_synchronizables_container();
    endpoint_to_synchronizables[endpoint] = initial_container;
  }

  const auto synchronizable =
      get_synchronizable_for_endpoint(endpoint, synchronizable_name);
  if (synchronizable.has_value()) {
    const auto value = synchronizable.value();
    value->apply_from_data_object(data_object);
  }
}

bool Synchronizer::is_endpoint_known(
    const udp_interface::Endpoint endpoint) const {
  return endpoint_to_synchronizables.count(endpoint) > 0;
}

void Synchronizer::for_each_endpoint(
    const std::function<void(const udp_interface::Endpoint)>& func) const {
  for (const auto& endpoint_synchronizables : endpoint_to_synchronizables) {
    func(endpoint_synchronizables.first);
  }
}

tl::optional<std::shared_ptr<Synchronizable>>
Synchronizer::get_synchronizable_for_endpoint(
    const udp_interface::Endpoint endpoint,
    const std::string synchronizable_name) {
  auto it = endpoint_to_synchronizables.find(endpoint);
  if (it == endpoint_to_synchronizables.end()) {
    return {};
  }

  for (const auto& synchronizable : it->second) {
    if (synchronizable->get_name() == synchronizable_name) {
      return synchronizable;
    }
  }

  return {};
}

const std::shared_ptr<SynchronizerDelegate> Synchronizer::get_delegate() const {
  return delegate;
};

void Synchronizer::set_delegate(
    const std::shared_ptr<SynchronizerDelegate> new_delegate) {
  delegate = new_delegate;
}

void Synchronizer::set_udp_interface(
    const std::shared_ptr<udp_interface::UDPInterface> udp_interface) {
  network_handler.set_udp_interface(udp_interface);
}

const NetworkHandler& Synchronizer::get_network_handler() const {
  return *(&network_handler);
}

void Synchronizer::remove_endpoint(const udp_interface::Endpoint endpoint) {
  auto it = endpoint_to_synchronizables.find(endpoint);
  if (it != endpoint_to_synchronizables.end()) {
    endpoint_to_synchronizables.erase(it);
  }
}

void Synchronizer::on_100_ms_passed() { network_handler.on_100_ms_passed(); }

void Synchronizer::heartbeat() { network_handler.heartbeat(); }
}  // namespace synchronizer