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

int main(int argc, char **argv) {
  UNITY_BEGIN();

  RUN_TEST(basic_nested_data_object_test);

  return UNITY_END();
}