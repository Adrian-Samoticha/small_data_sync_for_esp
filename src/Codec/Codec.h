#pragma once

#include <memory>
#include <string>

#include "DataFormat/DataFormat.h"
#include "DataObject/DataObject.h"
#include "optional/include/tl/optional.hpp"

struct Codec {
  virtual tl::optional<std::shared_ptr<DataObject::GenericValue>> decode(
      std::string encoded_data, std::string &error_string) const = 0;

  virtual std::string encode(
      std::shared_ptr<DataObject::GenericValue> data) const = 0;

  virtual DataFormat get_format() const = 0;

  virtual ~Codec() = default;
};