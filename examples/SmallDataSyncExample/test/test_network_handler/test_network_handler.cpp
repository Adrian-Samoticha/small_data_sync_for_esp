#include <unity.h>

#include <functional>
#include <queue>

#include "../utils.h"
#include "foo.h"

struct NetworkMessageImpl : public NetworkMessage {
  virtual std::shared_ptr<data_object::GenericValue> to_data_object() const {
    return data_object::create_null_value();
  };

  virtual void on_send_succeeded() const {}

  virtual void on_send_failed() const {}
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
      udp_interface::Endpoint(std::make_shared<utils::IPAddressImpl>(1), 0);
  auto receiver_network_handler = NetworkHandler();
  auto receiver_udp_interface = std::make_shared<utils::UdpInterfaceImpl>(
      receiver, receiver_network_handler, network_simulator);
  receiver_network_handler.set_udp_interface(receiver_udp_interface);

  network_simulator.register_endpoint(sender);
  network_simulator.register_endpoint(receiver);

  auto has_received_message = std::make_shared<bool>(false);

  auto receiver_delegate = std::make_shared<NetworkHandlerDelegateImpl>(
      [has_received_message](IncomingDecodedMessage message) {
        if (message.data_object->is_null()) {
          *has_received_message = true;
        }
      });
  receiver_network_handler.set_delegate(receiver_delegate);

  auto message = std::make_shared<NetworkMessageImpl>();
  sender_network_handler.send_message(message, receiver, 100);

  sender_network_handler.on_100_ms_passed();
  receiver_network_handler.heartbeat();

  TEST_ASSERT_EQUAL(true, *has_received_message);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(basic_network_handler_test);

  return UNITY_END();
}