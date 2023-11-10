#pragma once

#include <memory>

#include "DataObject/DataObject.h"
#include "NetworkHandler/MessageType/MessageType.h"

/**
 * An abstract base class defining the interface for network message objects.
 * Subclasses will implement specific message types.
 */
struct NetworkMessage {
  /**
   * Returns a data object containing any information contained in this message.
   */
  virtual std::shared_ptr<data_object::GenericValue> to_data_object() const = 0;

  /**
   * Returns the message type of this message.
   */
  virtual MessageType get_message_type() const { return MessageType::MSG; }

  /**
   * Called when the message has been successfully sent over the network.
   */
  virtual void on_send_succeeded() const {}

  /**
   * Called when the message failed to send over the network.
   */
  virtual void on_send_failed() const {}

  /**
   * Called when the message transmission was cancelled before completion.
   */
  virtual void on_cancelled() const {}

  /**
   * Returns a data object containing metadata about this message. This can be
   * overridden by subclasses to return additional type-specific metadata.
   */
  virtual std::shared_ptr<data_object::GenericValue> get_info() const {
    return data_object::create_null_value();
  }

  virtual ~NetworkMessage() = default;
};