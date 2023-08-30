#pragma once

#include <unity.h>

#include <cstdlib>
#include <memory>
#include <queue>

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

  bool operator==(const udp_interface::IPAddress& other) const override {
    return address == ((IPAddressImpl&)other).address;
  }
  bool operator!=(const udp_interface::IPAddress& other) const override {
    return !(this->operator==(other));
  }
  bool operator<(const udp_interface::IPAddress& other) const override {
    return address < ((IPAddressImpl&)other).address;
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

    const auto incoming_message =
        udp_interface::IncomingMessage(sender, packet);
    endpoint_to_buffer.at(receiver).push(incoming_message);

#if DEBUG_PRINT
    TEST_PRINTF("succeeded.\n", -1);
#endif
  }

  bool is_incoming_packet_available(
      const udp_interface::Endpoint receiver) const {
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
  udp_interface::Endpoint& endpoint;
  NetworkHandler& network_handler;
  NetworkSimulator& network_simulator;

  UdpInterfaceImpl(udp_interface::Endpoint& endpoint,
                   NetworkHandler& network_handler,
                   NetworkSimulator& network_simulator)
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
}  // namespace utils