#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "optional/include/tl/optional.hpp"

namespace DataObject {
struct GenericValue {
  typedef std::vector<std::shared_ptr<GenericValue>> array;
  typedef std::map<std::string, std::shared_ptr<GenericValue>> object;

  virtual bool is_null() const { return false; }
  virtual bool is_number() const { return false; }
  virtual bool is_bool() const { return false; }
  virtual bool is_string() const { return false; }
  virtual bool is_array() const { return false; }
  virtual bool is_object() const { return false; }

  virtual tl::optional<double> number_value() const { return {}; }
  virtual tl::optional<int> int_value() const { return {}; }
  virtual tl::optional<bool> bool_value() const { return {}; }
  virtual const tl::optional<std::string> string_value() const { return {}; }
  virtual const tl::optional<std::shared_ptr<GenericValue::array>> array_items()
      const {
    return {};
  }
  virtual const tl::optional<std::shared_ptr<GenericValue>> operator[](
      size_t i) const {
    return {};
  }
  virtual const tl::optional<std::shared_ptr<GenericValue::object>>
  object_items() const {
    return {};
  }
  virtual const tl::optional<std::shared_ptr<GenericValue>> operator[](
      const std::string &key) const {
    return {};
  }

  virtual std::string to_debug_string() const { return ""; }

  virtual bool equals(const std::shared_ptr<GenericValue> other) const {
    return false;
  }

  virtual ~GenericValue() {}
};

struct NullValue : public GenericValue {
  bool is_null() const override { return true; }

  std::string to_debug_string() const override { return "null"; }

  bool equals(const std::shared_ptr<GenericValue> other) const override {
    return other->is_null();
  }
};

struct NumberValue : public GenericValue {
 private:
  double value;

 public:
  NumberValue(double value) : value(value) {}

  NumberValue(int value) { this->value = (double)value; }

  bool is_number() const override { return true; }
  tl::optional<double> number_value() const override { return value; }
  tl::optional<int> int_value() const override { return std::round(value); }

  std::string to_debug_string() const override { return std::to_string(value); }

  bool equals(const std::shared_ptr<GenericValue> other) const override {
    if (other->is_number()) {
      return value == other->number_value().value();
    }
    return false;
  }
};

struct BoolValue : public GenericValue {
 private:
  bool value;

 public:
  BoolValue(bool value) : value(value) {}

  bool is_bool() const override { return true; }
  tl::optional<bool> bool_value() const override { return value; }

  std::string to_debug_string() const override {
    return value ? "true" : "false";
  }

  bool equals(const std::shared_ptr<GenericValue> other) const override {
    if (other->is_bool()) {
      return value == other->bool_value().value();
    }
    return false;
  }
};

struct StringValue : public GenericValue {
 private:
  std::string value;

 public:
  StringValue(std::string value) : value(value) {}

  bool is_string() const override { return true; }
  const tl::optional<std::string> string_value() const override {
    return value;
  }

  std::string to_debug_string() const override { return value; }

  bool equals(const std::shared_ptr<GenericValue> other) const override {
    if (other->is_string()) {
      return value == other->string_value().value();
    }
    return false;
  }
};

struct Array : public GenericValue {
 private:
  std::shared_ptr<GenericValue::array> value;

 public:
  Array(std::shared_ptr<GenericValue::array> value) : value(value) {}

  bool is_array() const override { return true; }
  const tl::optional<std::shared_ptr<GenericValue::array>> array_items()
      const override {
    return value;
  }
  const tl::optional<std::shared_ptr<GenericValue>> operator[](
      size_t i) const override {
    if (i >= value->size()) {
      return {};
    }

    return value->at(i);
  }

  std::string to_debug_string() const override {
    std::string result = "[";

    for (int i = 0; i < value->size(); i += 1) {
      result += value->at(i)->to_debug_string();

      if (i != value->size() - 1) {
        result += ", ";
      }
    }

    return result + "]";
  }

  bool equals(const std::shared_ptr<GenericValue> other) const override {
    if (!other->is_array()) {
      return false;
    }

    const auto &other_array = other->array_items().value();

    if (value->size() != other_array->size()) {
      return false;
    }

    for (size_t i = 0; i < value->size(); i += 1) {
      if (!value->at(i)->equals(other_array->at(i))) {
        return false;
      }
    }

    return true;
  }
};

struct Object : public GenericValue {
 private:
  std::shared_ptr<GenericValue::object> value;

 public:
  Object(std::shared_ptr<GenericValue::object> value) : value(value) {}

  bool is_object() const override { return true; }
  const tl::optional<std::shared_ptr<GenericValue::object>> object_items()
      const override {
    return value;
  }
  const tl::optional<std::shared_ptr<GenericValue>> operator[](
      const std::string &key) const override {
    if (value->count(key) == 0) {
      return {};
    }

    return value->at(key);
  }

  std::string to_debug_string() const override {
    std::string result = "{";

    for (auto it = value->begin(); it != value->end(); ++it) {
      if (it != value->begin()) {
        result += ", ";
      }

      result += "\"" + it->first + "\"" + " : " + it->second->to_debug_string();
    }

    return result + "}";
  }

  bool equals(const std::shared_ptr<GenericValue> other) const override {
    if (!other->is_object()) {
      return false;
    }

    const auto &other_object_items = other->object_items().value();

    if (value->size() != other_object_items->size()) {
      return false;
    }

    for (auto it = value->begin(); it != value->end(); ++it) {
      auto key = it->first;
      auto data_object = it->second;

      if (other_object_items->count(key) == 0) {
        return false;
      }

      if (!data_object->equals(other_object_items->at(key))) {
        return false;
      }
    }

    return true;
  }
};

std::shared_ptr<GenericValue> create_null_value();

std::shared_ptr<GenericValue> create_number_value(double value);

std::shared_ptr<GenericValue> create_number_value(int value);

std::shared_ptr<GenericValue> create_bool_value(bool value);

std::shared_ptr<GenericValue> create_string_value(std::string value);

std::shared_ptr<Array> create_array(GenericValue::array value);

std::shared_ptr<Object> create_object(GenericValue::object value);
}  // namespace DataObject