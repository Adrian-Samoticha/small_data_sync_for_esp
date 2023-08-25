#pragma once

#include <stdint.h>

#include <cstddef>
#include <memory>
#include <string>

#include "optional/include/tl/optional.hpp"

namespace udp_interface {
struct IPAddress {
  virtual bool operator==(const IPAddress& other) const = 0;
  virtual bool operator!=(const IPAddress& other) const = 0;
  virtual bool operator<(const IPAddress& other) const = 0;

  virtual std::string to_string() const { return "unimplemented"; };

  virtual ~IPAddress() = default;
};

struct Endpoint {
  std::shared_ptr<IPAddress> ip;
  uint16_t port;

  Endpoint(std::shared_ptr<IPAddress> ip, uint16_t port) : ip(ip), port(port) {}

  bool operator==(const Endpoint& other) const {
    return this->ip == other.ip && this->port == other.port;
  };
  bool operator!=(const Endpoint& other) const {
    return this->ip != other.ip || this->port != other.port;
  };
  bool operator<(const Endpoint& other) const {
    if (other.ip < this->ip) {
      return false;
    }

    return this->ip < other.ip || this->port < other.port;
  };
};

struct IncomingMessage {
  Endpoint endpoint;
  std::string data;

  IncomingMessage(Endpoint endpoint, std::string data)
      : endpoint(endpoint), data(data) {}
};

struct UDPInterface {
  virtual bool send_packet(Endpoint endpoint, std::string packet) = 0;

  virtual bool is_incoming_packet_available() = 0;
  virtual tl::optional<IncomingMessage> receive_packet() = 0;

  virtual ~UDPInterface() = default;
};
}