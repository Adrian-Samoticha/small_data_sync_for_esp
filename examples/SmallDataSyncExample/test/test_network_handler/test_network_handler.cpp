#include <unity.h>

#include <cstdlib>
#include <functional>
#include <queue>

#include "../utils.h"
#include "foo.h"

struct NetworkMessageImpl : public NetworkMessage {
  std::function<void()> on_send_succeeded_cb = []() {};

  NetworkMessageImpl() {}

  NetworkMessageImpl(std::function<void()> on_send_succeeded_cb)
      : on_send_succeeded_cb(on_send_succeeded_cb) {}

  std::shared_ptr<data_object::GenericValue> to_data_object() const override {
    return data_object::create_null_value();
  };

  void on_send_succeeded() const override { on_send_succeeded_cb(); }

  void on_send_failed() const override {}
};

struct NetworkHandlerDelegateImpl : public NetworkHandlerDelegate {
 private:
  std::function<void(IncomingDecodedMessage message)> on_message_received_cb;

 public:
  NetworkHandlerDelegateImpl(std::function<void(IncomingDecodedMessage message)>
                                 on_message_received_cb)
      : on_message_received_cb(on_message_received_cb) {}

  void on_message_received(IncomingDecodedMessage message) const override {
    on_message_received_cb(message);
  };

  void on_message_emitted(
      std::shared_ptr<NetworkMessage> message) const override{};

  void on_ack_received(unsigned int message_id) const override{};

  void on_message_discarded(unsigned int message_id) const override{};

  void on_decode_failed(std::string error,
                        std::shared_ptr<Codec> codec) const override{};
};

void basic_network_handler_test() {
  auto network_simulator = utils::NetworkSimulator();

  auto sender =
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(0), 0);
  auto sender_network_handler = NetworkHandler();
  auto sender_udp_interface = std::make_shared<utils::UdpInterfaceImpl>(
      sender, sender_network_handler, network_simulator);
  sender_network_handler.set_udp_interface(sender_udp_interface);

  auto receiver =
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(1), 1);
  auto receiver_network_handler = NetworkHandler();
  auto receiver_udp_interface = std::make_shared<utils::UdpInterfaceImpl>(
      receiver, receiver_network_handler, network_simulator);
  receiver_network_handler.set_udp_interface(receiver_udp_interface);

  network_simulator.register_endpoint(sender);
  network_simulator.register_endpoint(receiver);

  auto has_received_message = std::make_shared<bool>(false);
  auto has_received_ack = std::make_shared<bool>(false);

  auto receiver_delegate = std::make_shared<NetworkHandlerDelegateImpl>(
      [has_received_message](IncomingDecodedMessage message) {
        if (message.data_object->is_null()) {
          *has_received_message = true;
        }
      });
  receiver_network_handler.set_delegate(receiver_delegate);

  auto message = std::make_shared<NetworkMessageImpl>(
      [has_received_ack]() { *has_received_ack = true; });
  sender_network_handler.send_message(message, receiver, 100);

  receiver_network_handler.heartbeat();
  sender_network_handler.heartbeat();

  TEST_ASSERT_EQUAL_MESSAGE(
      true, *has_received_message,
      "basic_network_handler_test (has_received_message should be true.)");
  TEST_ASSERT_EQUAL_MESSAGE(
      true, *has_received_ack,
      "basic_network_handler_test (has_received_ack should be true.)");
}

void basic_network_handler_test_with_packet_loss() {
  std::srand(234823u);

  auto network_simulator = utils::NetworkSimulator();
  network_simulator.set_packet_loss_rate(0.9);

  auto sender =
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(0), 0);
  auto sender_network_handler = NetworkHandler();
  auto sender_udp_interface = std::make_shared<utils::UdpInterfaceImpl>(
      sender, sender_network_handler, network_simulator);
  sender_network_handler.set_udp_interface(sender_udp_interface);

  auto receiver =
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(1), 1);
  auto receiver_network_handler = NetworkHandler();
  auto receiver_udp_interface = std::make_shared<utils::UdpInterfaceImpl>(
      receiver, receiver_network_handler, network_simulator);
  receiver_network_handler.set_udp_interface(receiver_udp_interface);

  network_simulator.register_endpoint(sender);
  network_simulator.register_endpoint(receiver);

  auto has_received_message = std::make_shared<bool>(false);

  auto receiver_delegate = std::make_shared<NetworkHandlerDelegateImpl>(
      [has_received_message](IncomingDecodedMessage message) {
        TEST_PRINTF("Received.\n", 0);
        if (message.data_object->is_null()) {
          *has_received_message = true;
        }
      });
  receiver_network_handler.set_delegate(receiver_delegate);

  auto message = std::make_shared<NetworkMessageImpl>();
  sender_network_handler.send_message(message, receiver, 100,
                                      std::make_shared<JsonCodec>());

  for (int i = 0; i < 200; ++i) {
    if (*has_received_message) {
      break;
    }

    for (int j = 0; j < 10; ++j) {
      sender_network_handler.heartbeat();
      receiver_network_handler.heartbeat();
    }
    sender_network_handler.on_100_ms_passed();
    receiver_network_handler.on_100_ms_passed();
  }

  TEST_ASSERT_EQUAL_MESSAGE(true, *has_received_message,
                            "basic_network_handler_test_with_packet_loss");
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(basic_network_handler_test);
  RUN_TEST(basic_network_handler_test_with_packet_loss);

  return UNITY_END();
}