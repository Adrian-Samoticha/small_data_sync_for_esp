#pragma once

#include <string>

namespace endpoint_info {
struct EndpointInfo {
 private:
  std::string hostname;
  std::string answer_text;

 public:
  EndpointInfo() = default;

  EndpointInfo(std::string hostname, std::string answer_text)
      : hostname(hostname), answer_text(answer_text) {}

  std::string get_hostname() const { return hostname; }

  std::string get_answer_text() const { return answer_text; }

  std::string to_string() const {
    return "Hostname: " + hostname + ", Answer: " + answer_text;
  }
};
}  // namespace endpoint_info