#include "MessageType_util.h"

#include <string.h>

#include <string>

tl::optional<MessageType> get_message_type_from_string(const char* type_str) {
  if (strcmp(type_str, "msg") == 0) {
    return MessageType::MSG;
  }

  if (strcmp(type_str, "sync") == 0) {
    return MessageType::SYNC;
  }

  if (strcmp(type_str, "dereg") == 0) {
    return MessageType::DEREG;
  }

  if (strcmp(type_str, "req_init_sync") == 0) {
    return MessageType::REQ_INIT_SYNC;
  }

  return {};
}

std::string get_string_from_message_type(MessageType type) {
  switch (type) {
    case MessageType::MSG:
      return std::string("msg");

    case MessageType::SYNC:
      return std::string("sync");

    case MessageType::DEREG:
      return std::string("dereg");

    case MessageType::REQ_INIT_SYNC:
      return std::string("req_init_sync");

    default:
      return std::string("msg");
  }
}
