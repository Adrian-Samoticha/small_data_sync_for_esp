#include <unity.h>

#include <functional>
#include <queue>

#include "foo.h"

struct IPAddressImpl : public IPAddress {
  unsigned int address;

  IPAddressImpl(unsigned int address) : address(address) {}

  bool operator==(const IPAddress& other) const override {
    return address == ((IPAddressImpl&)other).address;
  }
  bool operator!=(const IPAddress& other) const override {
    return !(this->operator==(other));
  }
  bool operator<(const IPAddress& other) const override {
    return address < ((IPAddressImpl&)other).address;
  }

  std::string to_string() const override { return std::to_string(address); }
};

struct NetworkSimulator {
 private:
  std::map<Endpoint, std::queue<IncomingMessage>> endpoint_to_buffer;

 public:
  void register_endpoint(Endpoint endpoint) {
    endpoint_to_buffer[endpoint] = std::queue<IncomingMessage>();
  }

  void send_packet(Endpoint sender, Endpoint receiver, std::string packet) {
    auto incoming_message = IncomingMessage(sender, packet);
    endpoint_to_buffer.at(receiver).push(incoming_message);
  }

  bool is_incoming_packet_available(Endpoint receiver) {
    return !endpoint_to_buffer.at(receiver).empty();
  }

  tl::optional<IncomingMessage> receive_packet(Endpoint receiver) {
    if (!is_incoming_packet_available(receiver)) {
      return {};
    }

    auto result = endpoint_to_buffer.at(receiver).front();
    endpoint_to_buffer.at(receiver).pop();

    return result;
  }
};

struct UdpInterfaceImpl : public UDPInterface {
  Endpoint& endpoint;
  NetworkHandler& network_handler;
  NetworkSimulator& network_simulator;

  UdpInterfaceImpl(Endpoint& endpoint, NetworkHandler& network_handler,
                   NetworkSimulator& network_simulator)
      : endpoint(endpoint),
        network_handler(network_handler),
        network_simulator(network_simulator) {}

  bool send_packet(Endpoint receiver, std::string packet) {
    network_simulator.send_packet(endpoint, receiver, packet);
    return true;
  }

  bool is_incoming_packet_available() {
    return network_simulator.is_incoming_packet_available(endpoint);
  }
  tl::optional<IncomingMessage> receive_packet() {
    auto result = network_simulator.receive_packet(endpoint);
    return network_simulator.receive_packet(endpoint);
  }
};

struct NetworkMessageImpl : public NetworkMessage {
  virtual std::shared_ptr<DataObject::GenericValue> to_data_object() const {
    return DataObject::create_null_value();
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
  auto network_simulator = NetworkSimulator();

  auto sender = Endpoint(std::make_shared<IPAddressImpl>(0), 0);
  auto sender_network_handler = NetworkHandler();
  auto sender_udp_interface = std::make_shared<UdpInterfaceImpl>(
      sender, sender_network_handler, network_simulator);
  sender_network_handler.set_udp_interface(sender_udp_interface);

  auto receiver = Endpoint(std::make_shared<IPAddressImpl>(1), 0);
  auto receiver_network_handler = NetworkHandler();
  auto receiver_udp_interface = std::make_shared<UdpInterfaceImpl>(
      receiver, receiver_network_handler, network_simulator);
  receiver_network_handler.set_udp_interface(receiver_udp_interface);

  network_simulator.register_endpoint(sender);
  network_simulator.register_endpoint(receiver);

  auto has_received_message = std::make_shared<bool>(false);

  auto receiver_delegate = std::make_shared<NetworkHandlerDelegateImpl>(
      [has_received_message](IncomingDecodedMessage message) {
        *has_received_message = true;
      });
  receiver_network_handler.set_delegate(receiver_delegate);

  auto message = std::make_shared<NetworkMessageImpl>();
  sender_network_handler.send_message(message, receiver, 100);

  sender_network_handler.on_100_ms_passed();
  receiver_network_handler.on_100_ms_passed();

  TEST_ASSERT_EQUAL(true, *has_received_message);
}

int main(int argc, char** argv) {
  UNITY_BEGIN();

  RUN_TEST(basic_network_handler_test);

  return UNITY_END();
}