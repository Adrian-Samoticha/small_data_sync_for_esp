#include "MessageType_util.h"

#include <string>

tl::optional<MessageType> get_message_type_from_string(const char* type_str) {
  if (strcmp(type_str, "msg") == 0) {
    return MessageType::MSG;
  }

  if (strcmp(type_str, "sync") == 0) {
    return MessageType::SYNC;
  }

  return {};
}

std::string get_string_from_message_type(MessageType type) {
  switch (type) {
    case MessageType::MSG:
      return std::string("msg");

    case MessageType::SYNC:
      return std::string("sync");

    default:
      return std::string("msg");
  }
}
