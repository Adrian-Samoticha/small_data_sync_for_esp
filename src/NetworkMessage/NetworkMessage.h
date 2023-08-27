#pragma once

#include <memory>

#include "DataObject/DataObject.h"

struct NetworkMessage {
  virtual std::shared_ptr<data_object::GenericValue> to_data_object() const = 0;

  virtual void on_send_succeeded() const {}

  virtual void on_send_failed() const {}

  virtual void on_cancelled() const {}

  virtual std::shared_ptr<data_object::GenericValue> get_info() const {
    return data_object::create_null_value();
  }

  virtual ~NetworkMessage() = default;
};