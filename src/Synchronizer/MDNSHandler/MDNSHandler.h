#pragma once

#include <memory>
#include <string>

#include "../Synchronizer.fwd.h"
#include "interfaces/MDNSInterface/MDNSInterface.h"

namespace mdns_handler {
struct MDNSHandler {
 private:
  bool is_mdns_running = false;
  std::shared_ptr<mdns_interface::MDNSInterface> mdns_interface;
  std::shared_ptr<synchronizer::Synchronizer> synchronizer;
  const char *hostname;
  std::string group_name;
  unsigned int time_between_scans_in_deciseconds = 600;
  unsigned int time_until_next_scan_in_deciseconds = 600;
  unsigned int scan_duration_in_deciseconds = 20;
  unsigned int time_until_commit_in_deciseconds = 0;
  bool is_performing_scan = false;

  std::string sanitize_string(std::string string) const;

  bool add_services();
  bool setup_service_texts();

  bool start_mdns();
  bool stop_mdns();

  bool restart_mdns();

  std::string get_group_service_name(const std::string group_name) const;

  bool start_scan();
  bool stop_scan();

  void commit_scan_results();

  void handle_scanning_schedule();

 public:
  MDNSHandler() = default;

  MDNSHandler(const std::shared_ptr<synchronizer::Synchronizer> synchronizer,
              const char *hostname)
      : synchronizer(synchronizer), hostname(hostname) {}

  bool init();

  void set_mdns_interface(
      const std::shared_ptr<mdns_interface::MDNSInterface> &interface);

  std::string get_hostname() const;
  bool set_hostname(const char *new_hostname);

  std::string get_group_name() const;
  bool set_group_name(const std::string new_group_name);

  unsigned int get_time_between_scans() const;
  void set_time_between_scans(unsigned int new_time_in_deciseconds);

  unsigned int get_time_until_next_scan() const;
  unsigned int get_time_until_next_commit() const;

  unsigned int get_scan_duration() const;
  void set_scan_duration(unsigned int new_scan_duration_in_deciseconds);

  bool is_scanning() const;

  void on_100_ms_passed();
  void heartbeat();
};
}  // namespace mdns_handler