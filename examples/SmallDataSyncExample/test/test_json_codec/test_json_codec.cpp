#include <unity.h>

#include <memory>

#include "../utils.h"
#include "foo.h"

using namespace DataObject;

void basic_json_codec_test() {
  auto codec = std::make_shared<JsonCodec>();

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

  auto encoding = codec->encode(data_object);

  std::string error_string;
  auto decoded_data_object = codec->decode(encoding, error_string);

  TEST_ASSERT_TRUE(decoded_data_object.has_value());

  TEST_ASSERT_TRUE(decoded_data_object.value()->equals(data_object));

  TEST_ASSERT_EQUAL_STRING(
      decoded_data_object.value()->to_debug_string().c_str(),
      data_object->to_debug_string().c_str());
}

void invalid_encoding_test() {
  auto codec = std::make_shared<JsonCodec>();

  std::string invalid_json = "[1,1,1,1,1,1/INVALID]";

  std::string error_string;
  auto decoded_data_object = codec->decode(invalid_json, error_string);

  TEST_ASSERT_EQUAL_STRING(error_string.c_str(),
                           "expected ',' in list, got '/' (47)");

  TEST_ASSERT_FALSE(decoded_data_object.has_value());
}

void fuzzy_json_codec_test() {
  auto codec = std::make_shared<JsonCodec>();

  std::srand(234823u);

  for (int i = 0; i < 2000; i += 1) {
    auto data_object = utils::generate_random_data_object(3);

    auto encoding = codec->encode(data_object);

    std::string error_string;
    auto decoded_data_object = codec->decode(encoding, error_string);

    TEST_ASSERT_TRUE(decoded_data_object.has_value());

    TEST_ASSERT_TRUE(decoded_data_object.value()->equals(data_object));

    TEST_ASSERT_EQUAL_STRING(
        decoded_data_object.value()->to_debug_string().c_str(),
        data_object->to_debug_string().c_str());
  }
}

int main(int argc, char **argv) {
  UNITY_BEGIN();

  RUN_TEST(basic_json_codec_test);
  RUN_TEST(invalid_encoding_test);
  RUN_TEST(fuzzy_json_codec_test);

  return UNITY_END();
}