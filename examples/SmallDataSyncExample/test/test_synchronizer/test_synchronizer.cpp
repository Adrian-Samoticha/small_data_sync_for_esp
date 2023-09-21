#include <unity.h>

#include <memory>

#include "../utils.h"
#include "foo.h"

struct SynchronizableMock : public Synchronizable {
 private:
  int integer = 0;

 public:
  int get_integer() const { return integer; }

  void set_integer(const int value) { integer = value; }

  std::string get_name() const { return "SynchronizableMock"; };

  std::shared_ptr<data_object::GenericValue> to_data_object() const {
    return data_object::create_number_value(integer);
  }

  bool apply_from_data_object(
      const std::shared_ptr<data_object::GenericValue> data_object) {
    if (data_object->is_number()) {
      integer = data_object->number_value().value();
      return true;
    }

    return false;
  }
};

struct DelegateImpl : public synchronizer::SynchronizerDelegate {
 private:
  std::vector<std::shared_ptr<Synchronizable>> synchronizables = {
      std::make_shared<SynchronizableMock>(),
  };

 public:
  std::vector<std::shared_ptr<Synchronizable>>
  create_initial_synchronizables_container() override {
    return {
        std::make_shared<SynchronizableMock>(),
    };
  }
};

void basic_synchronizer_test() {
  const auto synchronizer = synchronizer::Synchronizer::create();

  const auto group_name = "my group";
  synchronizer->set_group_name(group_name);
  const auto group_name_hash = synchronizer->get_group_name_hash();

  const auto delegate = std::make_shared<DelegateImpl>();
  synchronizer->set_delegate(delegate);

  const auto sender =
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(0), 0);

  synchronizer->handle_synchronization_message(
      group_name_hash, sender, "SynchronizableMock",
      data_object::create_number_value(42));

  const auto synchronizable =
      synchronizer->get_synchronizable_for_endpoint<SynchronizableMock>(
          sender, "SynchronizableMock");

  TEST_ASSERT_TRUE_MESSAGE(synchronizable.has_value(),
                           "synchronizable should have value.");

  const auto synchronizable_value = synchronizable.value();

  TEST_ASSERT_EQUAL_MESSAGE(42, synchronizable_value->get_integer(),
                            "synchronizable_value’s integer should be 42.");
}

void basic_synchronizer_test_with_network_simulator() {
  auto network_simulator = utils::NetworkSimulator();
  network_simulator.set_packet_loss_rate(0.5);

  auto sender =
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(0), 0);
  auto sender_synchronizer = synchronizer::Synchronizer::create();
  const auto sender_synchronizer_delegate = std::make_shared<DelegateImpl>();
  sender_synchronizer->set_delegate(sender_synchronizer_delegate);
  auto sender_network_handler = sender_synchronizer->get_network_handler();
  sender_synchronizer->set_default_data_format(DataFormat::JSON);

  auto const sender_udp_interface = std::make_shared<utils::UdpInterfaceImpl>(
      sender, sender_network_handler, network_simulator);
  sender_synchronizer->set_udp_interface(sender_udp_interface);

  auto receiver =
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(1), 1);
  auto receiver_synchronizer = synchronizer::Synchronizer::create();
  const auto receiver_synchronizer_delegate = std::make_shared<DelegateImpl>();
  receiver_synchronizer->set_delegate(receiver_synchronizer_delegate);
  auto receiver_network_handler = receiver_synchronizer->get_network_handler();
  receiver_synchronizer->set_default_data_format(DataFormat::JSON);

  auto const receiver_udp_interface = std::make_shared<utils::UdpInterfaceImpl>(
      receiver, receiver_network_handler, network_simulator);
  receiver_synchronizer->set_udp_interface(receiver_udp_interface);

  network_simulator.register_endpoint(sender);
  network_simulator.register_endpoint(receiver);

  auto sender_synchronizable = std::make_shared<SynchronizableMock>();
  sender_synchronizable->set_integer(42);

  receiver_synchronizer->add_endpoint(sender);
  sender_synchronizer->add_endpoint(receiver);

  sender_synchronizer->synchronize(sender_synchronizable);

  for (int i = 0; i < 100; i += 1) {
    sender_synchronizer->on_100_ms_passed();
    sender_synchronizer->heartbeat();
    receiver_synchronizer->on_100_ms_passed();
    receiver_synchronizer->heartbeat();
  }

  const auto receiver_synchronizable =
      receiver_synchronizer
          ->get_synchronizable_for_endpoint<SynchronizableMock>(
              sender, "SynchronizableMock");

  TEST_ASSERT_TRUE_MESSAGE(receiver_synchronizable.has_value(),
                           "receiver_synchronizable should have value.");

  const auto receiver_synchronizable_value = receiver_synchronizable.value();

  TEST_ASSERT_EQUAL_MESSAGE(
      42, receiver_synchronizable_value->get_integer(),
      "receiver_synchronizable_value’s integer should be 42.");
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(basic_synchronizer_test);
  RUN_TEST(basic_synchronizer_test_with_network_simulator);

  return UNITY_END();
}