#pragma once

#include <memory>
#include <string>

#include "Codec/Codec.h"
#include "DataObject/DataObject.h"

struct Synchronizable {
  virtual std::string get_name() const = 0;

  virtual std::shared_ptr<data_object::GenericValue> to_data_object() const = 0;

  virtual bool apply_from_data_object(
      std::shared_ptr<data_object::GenericValue> data_object) = 0;

  virtual ~Synchronizable() = default;
};