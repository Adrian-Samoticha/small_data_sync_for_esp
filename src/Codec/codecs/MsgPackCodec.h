#pragma once

#include "Codec/Codec.h"
#include "msgpack11/msgpack11.hpp"

struct MsgPackCodec : public Codec {
  std::shared_ptr<data_object::GenericValue> msgpack_to_data_object(
      const msgpack11::MsgPack& msgpack) const {
    if (msgpack.is_null()) {
      return data_object::create_null_value();
    }

    if (msgpack.is_number()) {
      return data_object::create_number_value(msgpack.number_value());
    }

    if (msgpack.is_bool()) {
      return data_object::create_bool_value(msgpack.bool_value());
    }

    if (msgpack.is_string()) {
      return data_object::create_string_value(msgpack.string_value());
    }

    if (msgpack.is_array()) {
      auto msgpack_array_items = msgpack.array_items();
      auto data_object_array = data_object::GenericValue::array();
      data_object_array.reserve(msgpack_array_items.size());
      for (auto& element : msgpack_array_items) {
        auto data_object_element = msgpack_to_data_object(element);
        data_object_array.push_back(data_object_element);
      }

      return data_object::create_array(data_object_array);
    }

    if (msgpack.is_object()) {
      auto msgpack_object_items = msgpack.object_items();
      auto data_object_object = data_object::GenericValue::object();
      for (auto& item : msgpack_object_items) {
        auto key = item.first;
        auto msgpack_value = item.second;
        auto value_data_object = msgpack_to_data_object(msgpack_value);
        data_object_object[key.string_value()] = value_data_object;
      }

      return data_object::create_object(data_object_object);
    }

    return data_object::create_null_value();
  }

  msgpack11::MsgPack data_object_to_msgpack(
      const std::shared_ptr<data_object::GenericValue> data_object) const {
    if (data_object->is_null()) {
      return msgpack11::MsgPack();
    }

    if (data_object->is_number()) {
      return msgpack11::MsgPack(data_object->number_value().value());
    }

    if (data_object->is_bool()) {
      return msgpack11::MsgPack(data_object->bool_value().value());
    }

    if (data_object->is_string()) {
      return msgpack11::MsgPack(data_object->string_value().value());
    }

    if (data_object->is_array()) {
      auto data_object_array = data_object->array_items().value();
      auto msgpack_array = msgpack11::MsgPack::array();
      msgpack_array.reserve(data_object_array->size());
      for (auto& element : *data_object_array) {
        auto msgpack_element = data_object_to_msgpack(element);
        msgpack_array.push_back(msgpack_element);
      }

      return msgpack11::MsgPack(msgpack_array);
    }

    if (data_object->is_object()) {
      auto data_object_items = data_object->object_items().value();
      auto msgpack_object = msgpack11::MsgPack::object();
      for (auto& item : *data_object_items) {
        auto key = item.first;
        auto value = item.second;
        auto msgpack_value = data_object_to_msgpack(value);
        msgpack_object[key] = msgpack_value;
      }

      return msgpack11::MsgPack(msgpack_object);
    }

    return msgpack11::MsgPack();
  }

 public:
  tl::optional<std::shared_ptr<data_object::GenericValue>> decode(
      std::string encoded_data, std::string& error_string) const override {
    auto parsed_msgpack = msgpack11::MsgPack::parse(encoded_data, error_string);

    if (!error_string.empty()) {
      return {};
    }

    return msgpack_to_data_object(parsed_msgpack);
  }

  std::string encode(
      std::shared_ptr<data_object::GenericValue> data) const override {
    auto msgpack = data_object_to_msgpack(data);

    return msgpack.dump();
  }

  DataFormat get_format() const override { return DataFormat::MSGPACK; };
};