#pragma once

#include <memory>

#include "foo.h"

namespace utils {
std::string generate_random_string() {
  auto length = std::rand() % 32;

  std::string output;
  for (int i = 0; i < length; i += 1) {
    output += 'a' + rand() % 26;
  }

  return output;
}

std::shared_ptr<data_object::GenericValue> generate_random_data_object(
    unsigned int depth) {
  if (depth == 0) {
    auto random_int = std::rand() % 4;

    switch (random_int) {
      case 0:
        return data_object::create_null_value();

      case 1: {
        auto random_float = (std::rand() % 1000000) / 100.0;
        return data_object::create_number_value(random_float);
      }

      case 2:
        return data_object::create_bool_value(std::rand() % 2 == 1);

      case 3:
        return data_object::create_string_value(generate_random_string());
    }
  } else {
    auto random_int = std::rand() % 2;

    switch (random_int) {
      case 0: {
        auto size = std::rand() % 8;
        data_object::GenericValue::array array;
        array.reserve(size);
        for (int i = 0; i < size; i += 1) {
          array.push_back(generate_random_data_object(depth - 1));
        }
        return data_object::create_array(array);
      }

      case 1: {
        auto size = std::rand() % 8;
        data_object::GenericValue::object object;
        for (int i = 0; i < size; i += 1) {
          auto key = generate_random_string();
          object[key] = generate_random_data_object(depth - 1);
        }
        return data_object::create_object(object);
      }
    }
  }

  return data_object::create_null_value();
}
}  // namespace utils