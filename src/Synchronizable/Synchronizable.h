#pragma once

#include <memory>
#include <string>

#include "Codec/Codec.h"
#include "DataObject/DataObject.h"

/**
 * A struct representing a synchronizable object that can be serialized and
 * deserialized.
 */
struct Synchronizable {
  /**
   * Returns the name of the synchronizable object.
   */
  virtual std::string get_name() const = 0;

  /**
   * Converts this synchronizable object to data object.
   */
  virtual std::shared_ptr<data_object::GenericValue> to_data_object() const = 0;

  /**
   * Applies the data from the given data object to this synchronizable object.
   */
  virtual bool apply_from_data_object(
      const std::shared_ptr<data_object::GenericValue> data_object) = 0;

  virtual ~Synchronizable() = default;
};