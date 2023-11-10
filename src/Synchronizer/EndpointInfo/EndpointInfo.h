#pragma once

#include <string>

namespace endpoint_info {
/**
 * A struct containing an endpoint’s hostname and answer text.
 */
struct EndpointInfo {
 private:
  std::string hostname;
  std::string answer_text;

 public:
  EndpointInfo() = default;

  EndpointInfo(std::string hostname, std::string answer_text)
      : hostname(hostname), answer_text(answer_text) {}

  /**
   * Returns the hostname.
   */
  std::string get_hostname() const { return hostname; }

  /**
   * Returns the answer text.
   */
  std::string get_answer_text() const { return answer_text; }

  /**
   * Converts this `EndpointInfo` to a string for debug purposes.
   */
  std::string to_string() const {
    return "Hostname: " + hostname + ", Answer: " + answer_text;
  }
};
}  // namespace endpoint_info