#pragma once

#include <unity.h>

#include <cstdlib>
#include <memory>
#include <queue>

#include "foo.h"

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

  double random_double() { return std::rand() / (RAND_MAX + 1.0); }

 public:
  void register_endpoint(udp_interface::Endpoint endpoint) {
    endpoint_to_buffer[endpoint] = std::queue<udp_interface::IncomingMessage>();
  }

  void send_packet(udp_interface::Endpoint sender,
                   udp_interface::Endpoint receiver, std::string packet) {
    if (random_double() < packet_loss_rate) {
      // Packet lost.
      return;
    }

    auto incoming_message = udp_interface::IncomingMessage(sender, packet);
    endpoint_to_buffer.at(receiver).push(incoming_message);
  }

  bool is_incoming_packet_available(udp_interface::Endpoint receiver) {
    return !endpoint_to_buffer.at(receiver).empty();
  }

  tl::optional<udp_interface::IncomingMessage> receive_packet(
      udp_interface::Endpoint receiver) {
    if (!is_incoming_packet_available(receiver)) {
      return {};
    }

    auto result = endpoint_to_buffer.at(receiver).front();
    endpoint_to_buffer.at(receiver).pop();

    return result;
  }

  void set_packet_loss_rate(double new_rate) { packet_loss_rate = new_rate; }
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

  bool send_packet(udp_interface::Endpoint receiver, std::string packet) {
    network_simulator.send_packet(endpoint, receiver, packet);
    return true;
  }

  bool is_incoming_packet_available() {
    return network_simulator.is_incoming_packet_available(endpoint);
  }
  tl::optional<udp_interface::IncomingMessage> receive_packet() {
    return network_simulator.receive_packet(endpoint);
  }
};
}  // namespace utils