#pragma once

#include <stdint.h>

#include <string>

#include "../UDPInterface/UDPInterface.h"

namespace mdns_interface {
struct MDNSInterface {
  virtual bool begin(const char *hostname) = 0;
  virtual bool add_service(const char *service_name, const char *protocol,
                           uint16_t port) = 0;
  virtual bool close() = 0;
  virtual std::string hostname(int index) = 0;
  virtual udp_interface::Endpoint endpoint(int index) = 0;
  virtual bool add_service_text(const char *service_name, const char *protocol,
                                const char *key, const char *value) = 0;
  virtual void install_service_query(
      const char *service, const char *protocol,
      std::function<void()> on_data_received) = 0;
  virtual void remove_service_query() = 0;
  virtual const char *answer_text(int index) = 0;
  virtual int answer_count() = 0;
};
}  // namespace mdns_interface