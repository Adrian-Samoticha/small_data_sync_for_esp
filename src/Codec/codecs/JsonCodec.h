#pragma once

#include "../Codec.h"
#include "json11/json11.hpp"

struct JsonCodec : public Codec {
 private:
  std::shared_ptr<data_object::GenericValue> json_to_data_object(
      const json11::Json& json) const {
    if (json.is_null()) {
      return data_object::create_null_value();
    }

    if (json.is_number()) {
      return data_object::create_number_value(json.number_value());
    }

    if (json.is_bool()) {
      return data_object::create_bool_value(json.bool_value());
    }

    if (json.is_string()) {
      return data_object::create_string_value(json.string_value());
    }

    if (json.is_array()) {
      auto json_array_items = json.array_items();
      auto data_object_array = data_object::GenericValue::array();
      data_object_array.reserve(json_array_items.size());
      for (auto& element : json_array_items) {
        auto data_object_element = json_to_data_object(element);
        data_object_array.push_back(data_object_element);
      }

      return data_object::create_array(data_object_array);
    }

    if (json.is_object()) {
      auto json_object_items = json.object_items();
      auto data_object_object = data_object::GenericValue::object();
      for (auto& item : json_object_items) {
        auto key = item.first;
        auto json_value = item.second;
        auto value_data_object = json_to_data_object(json_value);
        data_object_object[key] = value_data_object;
      }

      return data_object::create_object(data_object_object);
    }

    return data_object::create_null_value();
  }

  json11::Json data_object_to_json(
      const std::shared_ptr<data_object::GenericValue> data_object) const {
    if (data_object->is_null()) {
      return json11::Json();
    }

    if (data_object->is_number()) {
      return json11::Json(data_object->number_value().value());
    }

    if (data_object->is_bool()) {
      return json11::Json(data_object->bool_value().value());
    }

    if (data_object->is_string()) {
      return json11::Json(data_object->string_value().value());
    }

    if (data_object->is_array()) {
      auto data_object_array = data_object->array_items().value();
      auto json_array = json11::Json::array();
      json_array.reserve(data_object_array->size());
      for (auto& element : *data_object_array) {
        auto json_element = data_object_to_json(element);
        json_array.push_back(json_element);
      }

      return json11::Json(json_array);
    }

    if (data_object->is_object()) {
      auto data_object_items = data_object->object_items().value();
      auto json_object = json11::Json::object();
      for (auto& item : *data_object_items) {
        auto key = item.first;
        auto value = item.second;
        auto json_value = data_object_to_json(value);
        json_object[key] = json_value;
      }

      return json11::Json(json_object);
    }

    return json11::Json();
  }

 public:
  tl::optional<std::shared_ptr<data_object::GenericValue>> decode(
      std::string encoded_data, std::string& error_string) const override {
    auto parsed_json = json11::Json::parse(encoded_data, error_string);

    if (!error_string.empty()) {
      return {};
    }

    return json_to_data_object(parsed_json);
  }

  std::string encode(
      std::shared_ptr<data_object::GenericValue> data) const override {
    auto json = data_object_to_json(data);

    return json.dump();
  }

  DataFormat get_format() const override { return DataFormat::JSON; };
};