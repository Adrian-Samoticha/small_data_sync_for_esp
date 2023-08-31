#pragma once

#include <functional>
#include <memory>

#include "NetworkHandler/NetworkHandler.h"
#include "NetworkHandler/UDPInterface/UDPInterface.h"
#include "Synchronizable/Synchronizable.h"
#include "SynchronizerDelegate/SynchronizerDelegate.h"
#include "optional/include/tl/optional.hpp"

namespace synchronizer {
struct Synchronizer : public std::enable_shared_from_this<Synchronizer> {
 private:
  std::shared_ptr<SynchronizerDelegate> delegate;
  NetworkHandler network_handler;
  std::map<udp_interface::Endpoint,
           std::vector<std::shared_ptr<Synchronizable>>>
      endpoint_to_synchronizables;

  bool is_message_related_to_synchronizable(
      const std::shared_ptr<data_object::GenericValue> message_info,
      const std::shared_ptr<Synchronizable> synchronizable) const;

  tl::optional<std::shared_ptr<Synchronizable>>
  get_synchronizable_instance_for_endpoint(
      const udp_interface::Endpoint endpoint,
      const std::string synchronizable_name) const;

  void init();

 public:
  static std::shared_ptr<Synchronizer> create();

  void synchronize(const std::shared_ptr<Synchronizable> synchronizable);

  void handle_synchronization_message(
      const udp_interface::Endpoint endpoint,
      const std::string synchronizable_name,
      std::shared_ptr<data_object::GenericValue> data_object);

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
      const auto cast_value = std::dynamic_pointer_cast<T>(instance.value());
      if (cast_value) {
        return cast_value;
      }
    }

    return {};
  }

  const std::shared_ptr<SynchronizerDelegate> get_delegate() const;

  void set_delegate(const std::shared_ptr<SynchronizerDelegate> new_delegate);

  void set_udp_interface(
      const std::shared_ptr<udp_interface::UDPInterface> udp_interface);

  void set_default_data_format(const DataFormat new_default_data_format);

  const NetworkHandler& get_network_handler() const;

  void add_endpoint(const udp_interface::Endpoint endpoint);

  void remove_endpoint(const udp_interface::Endpoint endpoint);

  void on_100_ms_passed();
  void heartbeat();
};
}  // namespace synchronizer