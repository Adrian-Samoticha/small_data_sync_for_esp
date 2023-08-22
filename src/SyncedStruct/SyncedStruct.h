#pragma once

#include <memory>
#include <string>

#include "Codec/Codec.h"
#include "DataObject/DataObject.h"

struct SyncedStruct {
  virtual std::string get_name() const = 0;

  virtual std::shared_ptr<DataObject::GenericValue> to_data_object() const = 0;

  virtual bool apply_from_data_object(
      std::shared_ptr<DataObject::GenericValue> data_object) = 0;

  virtual ~SyncedStruct() = default;
};