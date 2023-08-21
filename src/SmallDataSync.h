#pragma once

#include "Codec/Codec.h"
#include "Codec/codecs/JsonCodec.h"
#include "Codec/codecs/MsgPackCodec.h"
#include "DataObject/DataObject.h"
#include "SyncedStruct/SyncedStruct.h"

struct SmallDataSync {
 public:
  int returnInt(int);
};
