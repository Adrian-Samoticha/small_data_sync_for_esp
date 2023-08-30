#pragma once

#include "Codec/Codec.h"
#include "Codec/codecs/JsonCodec.h"
#include "Codec/codecs/MsgPackCodec.h"
#include "DataObject/DataObject.h"
#include "NetworkHandler/NetworkHandler.h"
#include "Synchronizable/Synchronizable.h"
#include "Synchronizer/Synchronizer.h"

struct SmallDataSync {
 public:
  int returnInt(int);
};
