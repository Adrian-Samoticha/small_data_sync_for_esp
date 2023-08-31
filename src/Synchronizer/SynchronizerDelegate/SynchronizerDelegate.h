#pragma once

#include "Synchronizable/Synchronizable.h"

namespace synchronizer {
struct SynchronizerDelegate {
  virtual std::vector<std::shared_ptr<Synchronizable>>
  create_initial_synchronizables_container() = 0;

  virtual void on_message_received(IncomingDecodedMessage message) const {};

  virtual void on_message_emitted(
      std::shared_ptr<NetworkMessage> message) const {};

  virtual void on_ack_received(unsigned int message_id) const {};

  virtual void on_message_discarded(unsigned int message_id) const {};

  virtual void on_decode_failed(std::string error,
                                std::shared_ptr<Codec> codec) const {};

  virtual ~SynchronizerDelegate() = default;
};
}  // namespace synchronizer