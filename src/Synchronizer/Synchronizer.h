#pragma once

#include <functional>
#include <memory>

#include "./EndpointInfo/EndpointInfo.h"
#include "./MDNSHandler/MDNSHandler.h"
#include "NetworkHandler/NetworkHandler.h"
#include "Synchronizable/Synchronizable.h"
#include "SynchronizerDelegate/SynchronizerDelegate.h"
#include "interfaces/UDPInterface/UDPInterface.h"
#include "optional/include/tl/optional.hpp"

namespace synchronizer {
struct Synchronizer : public std::enable_shared_from_this<Synchronizer> {
 private:
  std::shared_ptr<SynchronizerDelegate> delegate;
  NetworkHandler network_handler;
  mdns_handler::MDNSHandler mdns_handler;
  std::map<udp_interface::Endpoint,
           std::vector<std::shared_ptr<Synchronizable>>>
      endpoint_to_synchronizables;
  std::map<udp_interface::Endpoint, endpoint_info::EndpointInfo>
      endpoint_to_endpoint_info;
  std::deque<std::shared_ptr<Synchronizable>> own_synchronizables;
  uint32_t group_name_hash;

  bool is_message_related_to_synchronizable(
      const std::shared_ptr<data_object::GenericValue> message_info,
      const std::shared_ptr<Synchronizable> synchronizable) const;

  tl::optional<std::shared_ptr<Synchronizable>>
  get_synchronizable_instance_for_endpoint(
      const udp_interface::Endpoint endpoint,
      const std::string synchronizable_name) const;

  void add_or_update_own_synchronizable(
      const std::shared_ptr<Synchronizable> synchronizable);

 public:
  static std::shared_ptr<Synchronizer> create(const char* hostname);

  void init();

  void synchronize(const std::shared_ptr<Synchronizable> synchronizable);

  void handle_synchronization_message(
      const uint32_t group_name_hash, const udp_interface::Endpoint endpoint,
      const std::string synchronizable_name,
      std::shared_ptr<data_object::GenericValue> data_object);

  void perform_initial_synchronization(const udp_interface::Endpoint endpoint);

  void request_initial_synchronization_from_endpoint(
      const udp_interface::Endpoint endpoint);

  bool is_endpoint_known(const udp_interface::Endpoint endpoint) const;

  void for_each_endpoint(
      const std::function<void(const udp_interface::Endpoint)>& func) const;

  template <typename T = Synchronizable>
  tl::optional<std::shared_ptr<T>> get_synchronizable_for_endpoint(
      const udp_interface::Endpoint endpoint,
      const std::string synchronizable_name) const {
    const auto instance =
        get_synchronizable_instance_for_endpoint(endpoint, synchronizable_name);

    if (instance.has_value()) {
#ifdef ALLOW_DYNAMIC_CAST
      const auto cast_value = std::dynamic_pointer_cast<T>(instance.value());
      if (cast_value) {
        return cast_value;
      }
#endif
      return std::static_pointer_cast<T>(instance.value());
    }

    return {};
  }

  const std::shared_ptr<SynchronizerDelegate> get_delegate() const;

  void set_delegate(const std::shared_ptr<SynchronizerDelegate> new_delegate);

  void set_udp_interface(
      const std::shared_ptr<udp_interface::UDPInterface> udp_interface);

  void set_mdns_interface(
      const std::shared_ptr<mdns_interface::MDNSInterface> mdns_interface);

  void set_default_data_format(const DataFormat new_default_data_format);

  const NetworkHandler& get_network_handler() const;
  const mdns_handler::MDNSHandler& get_mdns_handler() const;

  void add_endpoint(const udp_interface::Endpoint endpoint);

  bool set_endpoint_info(const udp_interface::Endpoint endpoint,
                         const endpoint_info::EndpointInfo& info);

  tl::optional<const endpoint_info::EndpointInfo&> get_endpoint_info(
      const udp_interface::Endpoint endpoint) const;

  void remove_endpoint(const udp_interface::Endpoint endpoint);

  void set_group_name(const std::string group_name);

  uint32_t get_group_name_hash() const;

  void deregister_all_endpoints();

  unsigned int get_time_between_scans() const;
  void set_time_between_scans(unsigned int new_time_in_deciseconds);

  unsigned int get_time_until_next_scan() const;
  unsigned int get_time_until_next_commit() const;

  unsigned int get_scan_duration() const;
  void set_scan_duration(unsigned int new_scan_duration_in_deciseconds);

  void perform_mdns_service_query_now();

  bool is_scanning() const;

  void on_100_ms_passed();
  void heartbeat();
};
}  // namespace synchronizer