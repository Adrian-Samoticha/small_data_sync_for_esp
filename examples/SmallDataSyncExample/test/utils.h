#pragma once

#include <unity.h>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <queue>
#include <vector>

#include "foo.h"

#define DEBUG_PRINT false

namespace utils {
std::string generate_random_string() {
  auto length = std::rand() % 32;

  std::string output;
  for (int i = 0; i < length; i += 1) {
    output += 'a' + rand() % 26;
  }

  return output;
}

std::shared_ptr<data_object::GenericValue> generate_random_data_object(
    unsigned int depth) {
  if (depth == 0) {
    auto random_int = std::rand() % 4;

    switch (random_int) {
      case 0:
        return data_object::create_null_value();

      case 1: {
        auto random_float = (std::rand() % 1000000) / 100.0;
        return data_object::create_number_value(random_float);
      }

      case 2:
        return data_object::create_bool_value(std::rand() % 2 == 1);

      case 3:
        return data_object::create_string_value(generate_random_string());
    }
  } else {
    auto random_int = std::rand() % 2;

    switch (random_int) {
      case 0: {
        auto size = std::rand() % 8;
        data_object::GenericValue::array array;
        array.reserve(size);
        for (int i = 0; i < size; i += 1) {
          array.push_back(generate_random_data_object(depth - 1));
        }
        return data_object::create_array(array);
      }

      case 1: {
        auto size = std::rand() % 8;
        data_object::GenericValue::object object;
        for (int i = 0; i < size; i += 1) {
          auto key = generate_random_string();
          object[key] = generate_random_data_object(depth - 1);
        }
        return data_object::create_object(object);
      }
    }
  }

  return data_object::create_null_value();
}

struct IPAddressImpl : public udp_interface::IPAddress {
  unsigned int address;

  IPAddressImpl(unsigned int address) : address(address) {}

  bool operator==(const udp_interface::IPAddress &other) const override {
    return address == ((IPAddressImpl &)other).address;
  }
  bool operator!=(const udp_interface::IPAddress &other) const override {
    return !(this->operator==(other));
  }
  bool operator<(const udp_interface::IPAddress &other) const override {
    return address < ((IPAddressImpl &)other).address;
  }

  std::string to_string() const override { return std::to_string(address); }
};

struct NetworkSimulator {
 private:
  std::map<udp_interface::Endpoint, std::queue<udp_interface::IncomingMessage>>
      endpoint_to_buffer;
  double packet_loss_rate = 0.0;

  double random_double() const { return std::rand() / (RAND_MAX + 1.0); }

 public:
  void register_endpoint(const udp_interface::Endpoint endpoint) {
    endpoint_to_buffer[endpoint] = std::queue<udp_interface::IncomingMessage>();
  }

  void send_packet(const udp_interface::Endpoint sender,
                   const udp_interface::Endpoint receiver,
                   const std::string packet) {
#if DEBUG_PRINT
    TEST_PRINTF("Sending packet \"%s\" from %s to %s...", packet.c_str(),
                sender.to_string().c_str(), receiver.to_string().c_str());
#endif
    if (random_double() < packet_loss_rate) {
#if DEBUG_PRINT
      TEST_PRINTF("lost.\n", -1);
#endif
      return;
    }

    if (endpoint_to_buffer.count(receiver) == 0) {
      throw std::runtime_error("Receiver endpoint is not registered.");
    }

    const auto incoming_message =
        udp_interface::IncomingMessage(sender, packet);
    endpoint_to_buffer.at(receiver).push(incoming_message);

#if DEBUG_PRINT
    TEST_PRINTF("succeeded.\n", -1);
#endif
  }

  bool is_incoming_packet_available(
      const udp_interface::Endpoint receiver) const {
    if (endpoint_to_buffer.count(receiver) == 0) {
      throw std::runtime_error("Receiver endpoint is not registered.");
    }

    auto result = !endpoint_to_buffer.at(receiver).empty();

#if DEBUG_PRINT
    TEST_PRINTF("Checking if packet is available for %s: %s\n",
                receiver.to_string().c_str(), result ? "true" : "false");
#endif

    return result;
  }

  tl::optional<udp_interface::IncomingMessage> receive_packet(
      const udp_interface::Endpoint receiver) {
    if (!is_incoming_packet_available(receiver)) {
#if DEBUG_PRINT
      TEST_PRINTF("Not receiving packet any for %s: (%s, %s)\n",
                  receiver.to_string().c_str());
#endif
      return {};
    }

    const auto result = endpoint_to_buffer.at(receiver).front();
    endpoint_to_buffer.at(receiver).pop();

#if DEBUG_PRINT
    TEST_PRINTF("Receiving packet for %s: (%s, %s)\n",
                receiver.to_string().c_str(), result.data.c_str(),
                result.endpoint.to_string().c_str());
#endif

    return result;
  }

  void set_packet_loss_rate(const double new_rate) {
    packet_loss_rate = new_rate;
  }
};

struct UdpInterfaceImpl : public udp_interface::UDPInterface {
  udp_interface::Endpoint &endpoint;
  NetworkHandler &network_handler;
  NetworkSimulator &network_simulator;

  UdpInterfaceImpl(udp_interface::Endpoint &endpoint,
                   NetworkHandler &network_handler,
                   NetworkSimulator &network_simulator)
      : endpoint(endpoint),
        network_handler(network_handler),
        network_simulator(network_simulator) {}

  bool send_packet(const udp_interface::Endpoint receiver,
                   const std::string packet) override {
    network_simulator.send_packet(endpoint, receiver, packet);

    return true;
  }

  bool is_incoming_packet_available() override {
    return network_simulator.is_incoming_packet_available(endpoint);
  }
  tl::optional<udp_interface::IncomingMessage> receive_packet() override {
    return network_simulator.receive_packet(endpoint);
  }
};

struct MDNSService {
  udp_interface::Endpoint endpoint;
  std::string service_name;
  std::string protocol;
  uint16_t port;

  MDNSService(udp_interface::Endpoint endpoint, std::string service_name,
              std::string protocol, uint16_t port)
      : endpoint(endpoint),
        service_name(service_name),
        protocol(protocol),
        port(port) {}

  bool operator==(const MDNSService &other) const {
    return endpoint == other.endpoint && service_name == other.service_name &&
           protocol == other.protocol && port == other.port;
  }

  bool operator<(const MDNSService &other) const {
    return std::tie(endpoint, service_name, protocol, port) <
           std::tie(other.endpoint, other.service_name, other.protocol,
                    other.port);
  }
};

struct MDNSServiceQuery {
  std::string service_name;
  std::string protocol;
  std::vector<MDNSService> found_services;

  MDNSServiceQuery() = default;

  MDNSServiceQuery(std::string service_name, std::string protocol,
                   std::vector<MDNSService> found_services)
      : service_name(service_name),
        protocol(protocol),
        found_services(found_services) {}

  MDNSService at(size_t index) { return found_services.at(index); }

  size_t size() { return found_services.size(); }
};

struct MDNSSimulator {
 private:
  std::map<udp_interface::Endpoint, std::string> endpoint_to_hostname;
  std::map<udp_interface::Endpoint, std::vector<MDNSService>>
      endpoint_to_services;
  std::map<udp_interface::Endpoint, MDNSServiceQuery> endpoint_to_service_query;
  std::map<MDNSService, std::string> service_to_service_txt;

 public:
  bool begin(udp_interface::Endpoint caller, const char *hostname) {
    endpoint_to_hostname[caller] = std::string(hostname);

#if DEBUG_PRINT
    TEST_PRINTF("Beginning mDNS for %s (hostname: %s).\n",
                caller.to_string().c_str(), hostname);
#endif

    return true;
  }

  bool add_service(udp_interface::Endpoint caller, const char *service_name,
                   const char *protocol, uint16_t port) {
    MDNSService service(caller, service_name, protocol, port);
    endpoint_to_services[caller].push_back(service);

    service_to_service_txt[service] = "";

#if DEBUG_PRINT
    TEST_PRINTF(
        "Adding service for %s (service_name: %s, protocol: %s, port: %u).\n",
        caller.to_string().c_str(), service_name, protocol, port);
#endif

    return true;
  }

  bool close(udp_interface::Endpoint caller) {
    if (endpoint_to_hostname.count(caller) == 0) {
      throw std::runtime_error("Caller endpoint is not registered.");
    }

    endpoint_to_hostname.erase(caller);
    endpoint_to_services.erase(caller);
    endpoint_to_service_query.erase(caller);

    // remove service texts
    for (auto it = service_to_service_txt.begin();
         it != service_to_service_txt.end();) {
      if (it->first.endpoint == caller) {
        it = service_to_service_txt.erase(it);
        continue;
      }

      ++it;
    }

#if DEBUG_PRINT
    TEST_PRINTF("Closing mDNS for %s.\n", caller.to_string().c_str());
#endif

    return true;
  }

  std::string hostname(udp_interface::Endpoint caller, int index) {
    const auto endpoint =
        endpoint_to_service_query.at(caller).found_services.at(index).endpoint;
    const auto to_be_returned = endpoint_to_hostname.at(endpoint);

#if DEBUG_PRINT
    TEST_PRINTF("Accessing hostname for %s (index: %d, accessed hostname: %s)",
                caller.to_string().c_str(), index, to_be_returned.c_str());
#endif

    return to_be_returned;
  }

  udp_interface::Endpoint endpoint(udp_interface::Endpoint caller, int index) {
    const auto to_be_returned =
        endpoint_to_service_query.at(caller).found_services.at(index).endpoint;

#if DEBUG_PRINT
    TEST_PRINTF("Accessing endpoint for %s (index: %d, accessed endpoint: %s)",
                caller.to_string().c_str(), index,
                to_be_returned.to_string().c_str());
#endif

    return to_be_returned;
  }

  bool add_service_text(udp_interface::Endpoint caller,
                        const char *service_name, const char *protocol,
                        const char *key, const char *value) {
    const auto services = endpoint_to_services.at(caller);
    const auto mdns_service = std::find_if(
        services.begin(), services.end(), [&](const MDNSService &s) {
          return s.service_name == service_name && s.protocol == protocol;
        });

    if (mdns_service != services.end()) {
#if DEBUG_PRINT
      TEST_PRINTF(
          "Adding service text for %s (service_name: %s, protocol: %s, key: "
          "%s, value: %s).\n",
          caller.to_string().c_str(), service_name, protocol, key, value);
#endif

      std::stringstream stream;

      stream << service_to_service_txt.at(*mdns_service).c_str() << key << '='
             << value << ';';

      service_to_service_txt[*mdns_service] = stream.str();

      return true;
    }

#if DEBUG_PRINT
    TEST_PRINTF(
        "Not adding service text for %s (service not found) (service_name: %s, "
        "protocol: %s, key: "
        "%s, value: %s).\n",
        caller.to_string().c_str(), service_name, protocol, key, value);
#endif

    return false;
  }

  void install_service_query(udp_interface::Endpoint caller,
                             const char *service, const char *protocol,
                             std::function<void()> on_data_received) {
    const auto queried_service_name = service;

    std::vector<MDNSService> found_services;

    for (const auto &services : endpoint_to_services) {
      if (services.first == caller) {
        continue;
      }

      for (const auto &service : services.second) {
        if (service.service_name == queried_service_name &&
            service.protocol == protocol) {
          found_services.push_back(service);
        }
      }
    }

    MDNSServiceQuery query(service, protocol, found_services);

    endpoint_to_service_query[caller] = query;

#if DEBUG_PRINT
    TEST_PRINTF(
        "Installing service query for %s (service: %s, protocol: %s).\n",
        caller.to_string().c_str(), service, protocol);
#endif

    for (const auto &service : found_services) {
      on_data_received();
    }
  }

  void remove_service_query(udp_interface::Endpoint caller) {
    endpoint_to_service_query.erase(caller);

#if DEBUG_PRINT
    TEST_PRINTF("Removing service query for %s.\n", caller.to_string().c_str());
#endif
  }

  const char *answer_text(udp_interface::Endpoint caller, int index) {
    // Warning: The answer text is not saved within the MDNSServiceQuery object.
    const auto service = endpoint_to_service_query.at(caller).at(index);

    const auto to_be_returned = service_to_service_txt.at(service).c_str();

#if DEBUG_PRINT
    TEST_PRINTF("Accessing answer text for %s (index: %d, answer text: %s)",
                caller.to_string().c_str(), index, to_be_returned);
#endif

    return to_be_returned;
  }

  int answer_count(udp_interface::Endpoint caller) {
    const auto to_be_returned = endpoint_to_service_query.at(caller).size();

#if DEBUG_PRINT
    TEST_PRINTF("Accessing answer count for %s (answer count: %d)",
                caller.to_string().c_str(), to_be_returned);
#endif

    return to_be_returned;
  }
};

struct MDNSInterfaceImpl : public mdns_interface::MDNSInterface {
  udp_interface::Endpoint &for_endpoint;
  MDNSSimulator &simulator;

  MDNSInterfaceImpl(udp_interface::Endpoint &for_endpoint,
                    MDNSSimulator &simulator)
      : for_endpoint(for_endpoint), simulator(simulator) {}

  bool begin(const char *hostname) override {
    return simulator.begin(for_endpoint, hostname);
  }

  bool add_service(const char *service_name, const char *protocol,
                   uint16_t port) override {
    return simulator.add_service(for_endpoint, service_name, protocol, port);
  }

  bool close() override { return simulator.close(for_endpoint); }

  std::string hostname(int index) override {
    return simulator.hostname(for_endpoint, index);
  }

  udp_interface::Endpoint endpoint(int index) override {
    return simulator.endpoint(for_endpoint, index);
  }

  bool add_service_text(const char *service_name, const char *protocol,
                        const char *key, const char *value) override {
    return simulator.add_service_text(for_endpoint, service_name, protocol, key,
                                      value);
  }

  void install_service_query(const char *service, const char *protocol,
                             std::function<void()> on_data_received) override {
    return simulator.install_service_query(for_endpoint, service, protocol,
                                           on_data_received);
  }

  void remove_service_query() override {
    return simulator.remove_service_query(for_endpoint);
  }

  const char *answer_text(int index) override {
    return simulator.answer_text(for_endpoint, index);
  }

  int answer_count() override { return simulator.answer_count(for_endpoint); }
};

struct EmptyMDNSInterfaceImpl : public mdns_interface::MDNSInterface {
  bool begin(const char *) override { return false; }

  bool add_service(const char *, const char *, uint16_t) override {
    return false;
  }

  bool close() override { return false; }

  std::string hostname(int) override { return ""; }

  udp_interface::Endpoint endpoint(int) override { return {}; }

  bool add_service_text(const char *, const char *, const char *,
                        const char *) override {
    return false;
  }

  void install_service_query(const char *, const char *,
                             std::function<void()>) override {}

  void remove_service_query() override {}

  const char *answer_text(int) override { return ""; }

  int answer_count() override { return 0; }
};
}  // namespace utils