#pragma once

#include <string>

namespace endpoint_info {
/// @brief A struct containing an endpoint’s hostname and answer text.
struct EndpointInfo {
 private:
  std::string hostname;
  std::string answer_text;

 public:
  EndpointInfo() = default;

  EndpointInfo(std::string hostname, std::string answer_text)
      : hostname(hostname), answer_text(answer_text) {}

  /// @brief Returns the hostname.
  /// @return The hostname string.
  std::string get_hostname() const { return hostname; }

  /// @brief Returns the answer text.
  /// @return The answer text string.
  std::string get_answer_text() const { return answer_text; }

  /// @brief Converts this EndpointInfo to a string for debug purposes.
  /// @return A string containing the hostname and answer text.
  std::string to_string() const {
    return "Hostname: " + hostname + ", Answer: " + answer_text;
  }
};
}  // namespace endpoint_info