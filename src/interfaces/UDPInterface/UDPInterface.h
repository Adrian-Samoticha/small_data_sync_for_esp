#pragma once

#include <stdint.h>

#include <cstddef>
#include <memory>
#include <string>

#include "optional/include/tl/optional.hpp"

namespace udp_interface {
struct AbstractIPAddress {
  virtual bool operator==(const AbstractIPAddress& other) const = 0;
  virtual bool operator!=(const AbstractIPAddress& other) const = 0;
  virtual bool operator<(const AbstractIPAddress& other) const = 0;

  virtual std::string to_string() const { return "unimplemented"; };

  virtual ~AbstractIPAddress() = default;
};

struct Endpoint {
  std::shared_ptr<AbstractIPAddress> ip;
  uint16_t port;

  Endpoint() = default;

  Endpoint(const std::shared_ptr<AbstractIPAddress> ip, const uint16_t port)
      : ip(ip), port(port) {}

  bool operator==(const Endpoint& other) const {
    return *this->ip == *other.ip && this->port == other.port;
  };
  bool operator!=(const Endpoint& other) const {
    return *this->ip != *other.ip || this->port != other.port;
  };
  bool operator<(const Endpoint& other) const {
    return std::tie(*this->ip, this->port) < std::tie(*other.ip, other.port);
  };

  std::string to_string() const {
    return ip->to_string() + ":" + std::to_string(port);
  }
};

struct IncomingMessage {
  const Endpoint endpoint;
  const std::string data;

  IncomingMessage(const Endpoint endpoint, const std::string data)
      : endpoint(endpoint), data(data) {}
};

struct UDPInterface {
  virtual bool send_packet(const Endpoint endpoint,
                           const std::string packet) = 0;

  virtual bool is_incoming_packet_available() = 0;
  virtual tl::optional<IncomingMessage> receive_packet() = 0;

  virtual ~UDPInterface() = default;
};
}  // namespace udp_interface