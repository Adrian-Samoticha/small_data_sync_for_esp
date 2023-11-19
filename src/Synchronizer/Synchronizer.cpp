#include "Synchronizer.h"

#include "ErriezCRC32/ErriezCRC32.h"
#include "NetworkHandlerDelegateImpl/NetworkHandlerDelegateImpl.h"
#include "network_messages/DeregistrationMessage.h"
#include "network_messages/RequestInitialSynchronizationMessage.h"
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

tl::optional<std::shared_ptr<Synchronizable>>
Synchronizer::get_synchronizable_instance_for_endpoint(
    const udp_interface::Endpoint endpoint,
    const std::string synchronizable_name) const {
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

void Synchronizer::add_or_update_own_synchronizable(
    const std::shared_ptr<Synchronizable> synchronizable) {
  for (size_t i = 0; i < own_synchronizables.size(); i += 1) {
    if (own_synchronizables[i]->get_name() == synchronizable->get_name()) {
      own_synchronizables[i] = synchronizable;
      return;
    }
  }

  own_synchronizables.push_back(synchronizable);
}

std::shared_ptr<Synchronizer> Synchronizer::create(const char* hostname) {
  const auto result = std::make_shared<Synchronizer>();
  result->mdns_handler = mdns_handler::MDNSHandler(result, hostname);

  return result;
}

void Synchronizer::init() {
  auto network_handler_delegate =
      std::make_shared<NetworkHandlerDelegateImpl>(shared_from_this());
  network_handler.set_delegate(network_handler_delegate);

  mdns_handler.init();
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

        network_handler.send_message(message, endpoint, 100u);
      });

  add_or_update_own_synchronizable(synchronizable);
}

void Synchronizer::handle_synchronization_message(
    const uint32_t group_name_hash, const udp_interface::Endpoint endpoint,
    const std::string synchronizable_name,
    std::shared_ptr<data_object::GenericValue> data_object) {
  auto is_from_same_group = group_name_hash == this->group_name_hash;

  if (!is_from_same_group) {
    // TODO: send deregistration message
    return;
  }

  if (!is_endpoint_known(endpoint)) {
    add_endpoint(endpoint);
  }

  const auto synchronizable =
      get_synchronizable_for_endpoint(endpoint, synchronizable_name);
  if (synchronizable.has_value()) {
    const auto value = synchronizable.value();
    value->apply_from_data_object(data_object);
  }
}

void Synchronizer::perform_initial_synchronization(
    const udp_interface::Endpoint endpoint) {
  for (const auto& synchronizable : own_synchronizables) {
    network_handler.cancel_active_messages(
        [this, synchronizable](
            std::shared_ptr<data_object::GenericValue> data_object) {
          return is_message_related_to_synchronizable(data_object,
                                                      synchronizable);
        });

    const std::shared_ptr<NetworkMessage> message =
        std::make_shared<SynchronizationMessage>(synchronizable, endpoint,
                                                 shared_from_this());

    network_handler.send_message(message, endpoint, 100u);
  }
}

void Synchronizer::request_initial_synchronization_from_endpoint(
    const udp_interface::Endpoint endpoint) {
  const std::shared_ptr<NetworkMessage> message =
      std::make_shared<RequestInitialSynchronizationMessage>();

  network_handler.send_message(message, endpoint, 100u);
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

void Synchronizer::set_mdns_interface(
    const std::shared_ptr<mdns_interface::MDNSInterface> mdns_interface) {
  mdns_handler.set_mdns_interface(mdns_interface);
}

void Synchronizer::set_default_data_format(
    const DataFormat new_default_data_format) {
  network_handler.set_default_data_format(new_default_data_format);
}

const NetworkHandler& Synchronizer::get_network_handler() const {
  return *(&network_handler);
}

const mdns_handler::MDNSHandler& Synchronizer::get_mdns_handler() const {
  return *(&mdns_handler);
}

void Synchronizer::add_endpoint(const udp_interface::Endpoint endpoint) {
  if (endpoint_to_synchronizables.count(endpoint) != 0) {
    return;
  }

  auto initial_container = delegate->create_initial_synchronizables_container();
  endpoint_to_synchronizables[endpoint] = initial_container;
  endpoint_to_endpoint_info[endpoint] = {};
}

bool Synchronizer::set_endpoint_info(const udp_interface::Endpoint endpoint,
                                     const endpoint_info::EndpointInfo& info) {
  // check if endpoint exists
  if (endpoint_to_endpoint_info.count(endpoint) == 0) {
    return false;
  }

  endpoint_to_endpoint_info[endpoint] = info;

  return true;
}

tl::optional<const endpoint_info::EndpointInfo&>
Synchronizer::get_endpoint_info(const udp_interface::Endpoint endpoint) const {
  if (endpoint_to_endpoint_info.count(endpoint) == 0) {
    return {};
  }

  return endpoint_to_endpoint_info.at(endpoint);
}

void Synchronizer::remove_endpoint(const udp_interface::Endpoint endpoint) {
  {
    auto it = endpoint_to_synchronizables.find(endpoint);
    if (it != endpoint_to_synchronizables.end()) {
      it->second.clear();
      endpoint_to_synchronizables.erase(it);
    }
  }

  {
    auto it = endpoint_to_endpoint_info.find(endpoint);
    if (it != endpoint_to_endpoint_info.end()) {
      endpoint_to_endpoint_info.erase(it);
    }
  }
}

void Synchronizer::set_group_name(const std::string group_name) {
  auto new_group_name_hash = crc32String(group_name.c_str());

  if (new_group_name_hash != group_name_hash) {
    group_name_hash = new_group_name_hash;

    deregister_all_endpoints();
  }

  mdns_handler.set_group_name(group_name);
}

uint32_t Synchronizer::get_group_name_hash() const { return group_name_hash; }

void Synchronizer::deregister_all_endpoints() {
  for_each_endpoint([this](const udp_interface::Endpoint endpoint) {
    auto deregistration_message = std::make_shared<DeregistrationMessage>();

    network_handler.send_message(deregistration_message, endpoint, 100u);
  });

  endpoint_to_synchronizables.clear();
}

unsigned int Synchronizer::get_time_between_scans() const {
  return mdns_handler.get_time_between_scans();
}

void Synchronizer::set_time_between_scans(
    unsigned int new_time_in_deciseconds) {
  mdns_handler.set_time_between_scans(new_time_in_deciseconds);
}

unsigned int Synchronizer::get_time_until_next_scan() const {
  return mdns_handler.get_time_until_next_scan();
}

unsigned int Synchronizer::get_time_until_next_commit() const {
  return mdns_handler.get_time_until_next_commit();
}

unsigned int Synchronizer::get_scan_duration() const {
  return mdns_handler.get_scan_duration();
}
void Synchronizer::set_scan_duration(
    unsigned int new_scan_duration_in_deciseconds) {
  mdns_handler.set_scan_duration(new_scan_duration_in_deciseconds);
}

/**
 * Performs a service query immediately without waiting for the next scheduled
 * scan. The timer will still be reset as if a normal scan occurred, therefore
 * the next scheduled scan will still happen at the normal interval after this
 * call returns.
 */
void Synchronizer::perform_mdns_service_query_now() {
  mdns_handler.perform_service_query_now();
}

bool Synchronizer::is_scanning() const { return mdns_handler.is_scanning(); }

void Synchronizer::on_100_ms_passed() {
  network_handler.on_100_ms_passed();
  mdns_handler.on_100_ms_passed();
}

void Synchronizer::heartbeat() {
  network_handler.heartbeat();
  mdns_handler.heartbeat();
}
}  // namespace synchronizer