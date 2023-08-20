#include <unity.h>

#include "foo.h"

using namespace DataObject;

void basic_nested_data_object_test() {
  auto data_object = create_object({
      {"nullValue", create_null_value()},
      {"fanSpeed", create_number_value(50)},
      {"isOn", create_bool_value(true)},
      {
          "deviceInfo",
          create_object({
              {"deviceName", create_string_value("MyDevice92342")},
              {"freeRAM", create_number_value(23239)},
          }),
      },
      {
          "humidityValues",
          create_array({
              create_number_value(34),
              create_number_value(37),
              create_number_value(40),
              create_number_value(48),
              create_number_value(50),
          }),
      },
  });

  std::string expected_debug_string =
      "{\"deviceInfo\" : {\"deviceName\" : MyDevice92342, \"freeRAM\" : "
      "23239.000000}, \"fanSpeed\" : 50.000000, \"humidityValues\" : "
      "[34.000000, 37.000000, 40.000000, 48.000000, 50.000000], \"isOn\" : "
      "true, \"nullValue\" : null}";

  TEST_ASSERT_EQUAL_STRING(data_object->to_debug_string().c_str(),
                           expected_debug_string.c_str());
}

void basic_equivalence_test() {
  auto data_object = create_object({
      {"nullValue", create_null_value()},
      {"fanSpeed", create_number_value(50)},
      {"isOn", create_bool_value(true)},
      {
          "deviceInfo",
          create_object({
              {"deviceName", create_string_value("MyDevice92342")},
              {"freeRAM", create_number_value(23239)},
          }),
      },
      {
          "humidityValues",
          create_array({
              create_number_value(34),
              create_number_value(37),
              create_number_value(40),
              create_number_value(48),
              create_number_value(50),
          }),
      },
  });

  TEST_ASSERT_TRUE(data_object->equals(data_object));

  auto data_object1 = create_object({
      {"foo", create_string_value("bar")},
      {
          "bar",
          create_array({
              create_number_value(1),
              create_number_value(2),
              create_number_value(3),
              create_number_value(4),
              create_number_value(5),
          }),
      },
  });

  auto data_object2 = create_object({
      {"foo", create_string_value("bar")},
      {
          "bar",
          create_array({
              create_number_value(1),
              create_number_value(2),
              create_number_value(3),
              create_number_value(4),
              create_number_value(6),
          }),
      },
  });

  TEST_ASSERT_FALSE(data_object1->equals(data_object2));

  auto data_object_with_int = create_object({
      {"number", create_number_value(5)},
  });

  auto data_object_with_double = create_object({
      {"number", create_number_value(5.0)},
  });

  TEST_ASSERT_TRUE(data_object_with_int->equals(data_object_with_double));
}

int main(int argc, char **argv) {
  UNITY_BEGIN();

  RUN_TEST(basic_nested_data_object_test);

  return UNITY_END();
}