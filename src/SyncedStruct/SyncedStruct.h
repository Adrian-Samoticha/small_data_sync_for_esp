#pragma once

#include <memory>

#include "SyncedStructEncoding/SyncedStructEncoding.h"

struct SyncedStruct {
  virtual bool applyEncoding(const SyncedStructEncoding& encoding) = 0;

  virtual std::unique_ptr<SyncedStructEncoding> getEncoding(
      const Codec& codec) const = 0;

  virtual ~SyncedStruct() = default;
};