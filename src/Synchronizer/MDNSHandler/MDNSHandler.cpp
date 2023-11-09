#include "MDNSHandler.h"

#include <algorithm>

#include "../Synchronizer.h"
#include "ErriezCRC32/ErriezCRC32.h"
#include "Synchronizer/EndpointInfo/EndpointInfo.h"

namespace mdns_handler {
std::string MDNSHandler::sanitize_string(std::string string) const {
  string.erase(std::remove(string.begin(), string.end(), '='), string.end());
  const auto substring = string.substr(0, 200);

  return substring;
}  // TODO: remove this maybe?

bool MDNSHandler::add_services() {
  return mdns_interface->add_service(get_group_service_name(group_name).c_str(),
                                     "udp",
                                     9000);  // TODO: make port configurable
}

bool MDNSHandler::setup_service_texts() { return false; }  // TODO

bool MDNSHandler::start_mdns() {
  if (!mdns_interface->begin(hostname)) {
    return false;
  }

  if (!add_services()) {
    return false;
  }

  if (!setup_service_texts()) {
    return false;
  }

  is_mdns_running = true;

  return true;
}

bool MDNSHandler::stop_mdns() {
  const auto close_succeeded = mdns_interface->close();

  if (!close_succeeded) {
    return false;
  }

  is_mdns_running = false;

  return true;
}  // TODO: perhaps deregister all known peers?

bool MDNSHandler::restart_mdns() {
  if (!stop_mdns()) {
    return false;
  }

  if (!start_mdns()) {
    return false;
  }

  return true;
}

std::string MDNSHandler::get_group_service_name(
    const std::string group_name) const {
  const auto group_name_hash = crc32String(group_name.c_str());

  char group_name_hash_string[9];
  snprintf(group_name_hash_string, 9, "%08X", group_name_hash);

  return "SmallDataSync_Group_" + std::string(group_name_hash_string);
}

bool MDNSHandler::start_scan() {
  mdns_interface->install_service_query(
      get_group_service_name(group_name).c_str(), "udp", []() {});

  is_performing_scan = true;

  return true;
}

bool MDNSHandler::stop_scan() {
  mdns_interface->remove_service_query();

  is_performing_scan = false;

  return true;
}

void MDNSHandler::commit_scan_results() {
  const auto answer_count = mdns_interface->answer_count();

  for (int i = 0; i < answer_count; i++) {
    const auto endpoint = mdns_interface->endpoint(i);
    const auto hostname = mdns_interface->hostname(i);
    const auto answer_text = mdns_interface->answer_text(i);

    synchronizer->add_endpoint(endpoint);

    const auto endpoint_info =
        endpoint_info::EndpointInfo(hostname, answer_text);
    synchronizer->set_endpoint_info(endpoint, endpoint_info);

    synchronizer->perform_initial_synchronization(endpoint);
    synchronizer->request_initial_synchronization_from_endpoint(endpoint);
  }
}

void MDNSHandler::handle_scanning_schedule() {
  if (is_performing_scan) {
    time_until_commit_in_deciseconds -= 1;
    if (time_until_commit_in_deciseconds == 0) {
      commit_scan_results();
      stop_scan();

      time_until_next_scan_in_deciseconds = time_between_scans_in_deciseconds;
    }
  } else {
    time_until_next_scan_in_deciseconds -= 1;
    if (time_until_next_scan_in_deciseconds == 0) {
      start_scan();

      time_until_commit_in_deciseconds = scan_duration_in_deciseconds;
    }
  }
}

bool MDNSHandler::init() {
  if (mdns_interface == nullptr) {
    throw std::runtime_error(
        "MDNS handler was not provided an mDNS interface.");
  }

  if (is_mdns_running) {
    return true;
  }

  return start_mdns();
}

void MDNSHandler::set_mdns_interface(
    const std::shared_ptr<mdns_interface::MDNSInterface>& interface) {
  mdns_interface = interface;
}

std::string MDNSHandler::get_hostname() const { return hostname; }

bool MDNSHandler::set_hostname(const char* new_hostname) {
  hostname = new_hostname;

  return restart_mdns();
}

std::string MDNSHandler::get_group_name() const { return group_name; }

bool MDNSHandler::set_group_name(const std::string new_group_name) {
  group_name = new_group_name;

  if (is_mdns_running) {
    return restart_mdns();
  }

  return true;
}

unsigned int MDNSHandler::get_time_between_scans() const {
  return time_between_scans_in_deciseconds;
}

void MDNSHandler::set_time_between_scans(unsigned int new_time_in_deciseconds) {
  time_between_scans_in_deciseconds = new_time_in_deciseconds;

  time_until_next_scan_in_deciseconds = std::min(
      time_until_next_scan_in_deciseconds, time_between_scans_in_deciseconds);
}

unsigned int MDNSHandler::get_time_until_next_scan() const {
  return time_until_next_scan_in_deciseconds;
}

unsigned int MDNSHandler::get_time_until_next_commit() const {
  return time_until_commit_in_deciseconds;
}

unsigned int MDNSHandler::get_scan_duration() const {
  return scan_duration_in_deciseconds;
}

void MDNSHandler::set_scan_duration(
    unsigned int new_scan_duration_in_deciseconds) {
  scan_duration_in_deciseconds = new_scan_duration_in_deciseconds;

  time_until_commit_in_deciseconds =
      std::min(time_until_commit_in_deciseconds, scan_duration_in_deciseconds);
}

bool MDNSHandler::is_scanning() const { return is_performing_scan; }

void MDNSHandler::on_100_ms_passed() { handle_scanning_schedule(); }

void MDNSHandler::heartbeat() {}
}  // namespace mdns_handler