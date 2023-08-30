#pragma once

#include "MessageType.h"
#include "optional/include/tl/optional.hpp"

tl::optional<MessageType> get_message_type_from_string(const char* type_str);

std::string get_string_from_message_type(MessageType type);