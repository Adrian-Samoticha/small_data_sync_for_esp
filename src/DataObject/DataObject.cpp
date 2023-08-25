#include "DataObject.h"

namespace DataObject {
std::shared_ptr<GenericValue> create_null_value() {
  return std::make_shared<NullValue>();
}

std::shared_ptr<GenericValue> create_number_value(double value) {
  return std::make_shared<NumberValue>(value);
}

std::shared_ptr<GenericValue> create_number_value(int value) {
  return std::make_shared<NumberValue>(value);
}

std::shared_ptr<GenericValue> create_bool_value(bool value) {
  return std::make_shared<BoolValue>(value);
}

std::shared_ptr<GenericValue> create_string_value(std::string value) {
  return std::make_shared<StringValue>(value);
}

std::shared_ptr<Array> create_array(GenericValue::array value) {
  auto shared_pointer = std::make_shared<GenericValue::array>(value);
  return std::make_shared<Array>(shared_pointer);
}

std::shared_ptr<Object> create_object(GenericValue::object value) {
  auto shared_pointer = std::make_shared<GenericValue::object>(value);
  return std::make_shared<Object>(shared_pointer);
}
}