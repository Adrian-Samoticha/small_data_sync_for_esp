#include <string.h>
#include <unity.h>

#include <memory>

#include "../utils.h"
#include "foo.h"

void basic_mdns_simulator_test() {
  utils::MDNSSimulator simulator;

  const auto endpoint0 =
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(0), 0);

  const auto endpoint1 =
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(1), 1);

  simulator.begin(endpoint0, "endpoint0");
  simulator.begin(endpoint1, "endpoint1");

  simulator.add_service(endpoint0, "my_service", "udp", 9000);
  TEST_ASSERT_TRUE_MESSAGE(
      simulator.add_service_text(endpoint0, "my_service", "udp",
                                 "number_as_text", "zero"),
      "basic_mdns_simulator_test (add_service_text should return true)");

  simulator.add_service(endpoint1, "my_other_service", "udp", 9001);
  TEST_ASSERT_TRUE_MESSAGE(
      simulator.add_service_text(endpoint1, "my_other_service", "udp",
                                 "number_as_text", "one"),
      "basic_mdns_simulator_test (add_service_text should return true)");

  const auto service_query_0_succeeded = std::make_shared<bool>(false);
  simulator.install_service_query(endpoint0, "my_other_service", "udp", [&]() {
    if (simulator.answer_count(endpoint0) != 1) {
      return;
    }

    const auto answer_text = simulator.answer_text(endpoint0, 0);
    if (strncmp(answer_text, "number_as_text=one;", 20) == 0) {
      *service_query_0_succeeded = true;
    }
  });

  const auto service_query_1_succeeded = std::make_shared<bool>(false);
  simulator.install_service_query(endpoint1, "my_service", "udp", [&]() {
    if (simulator.answer_count(endpoint1) != 1) {
      return;
    }

    const auto answer_text = simulator.answer_text(endpoint1, 0);
    if (strncmp(answer_text, "number_as_text=zero;", 21) == 0) {
      *service_query_1_succeeded = true;
    }
  });

  TEST_ASSERT_TRUE_MESSAGE(
      *service_query_0_succeeded,
      "basic_mdns_simulator_test (service_query_0_succeeded should be true.)");
  TEST_ASSERT_TRUE_MESSAGE(
      *service_query_1_succeeded,
      "basic_mdns_simulator_test (service_query_1_succeeded should be true.)");

  TEST_ASSERT_EQUAL_STRING_MESSAGE(
      endpoint1.to_string().c_str(),
      simulator.endpoint(endpoint0, 0).to_string().c_str(),
      "basic_mdns_simulator_test (simulator.endpoint() should return the "
      "correct endpoint.)");
  TEST_ASSERT_EQUAL_STRING_MESSAGE(
      endpoint0.to_string().c_str(),
      simulator.endpoint(endpoint1, 0).to_string().c_str(),
      "basic_mdns_simulator_test (simulator.endpoint() should return the "
      "correct endpoint.)");
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(basic_mdns_simulator_test);

  return UNITY_END();
}