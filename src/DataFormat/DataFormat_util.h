#pragma once

#include <memory>

#include "Codec/Codec.h"
#include "Codec/codecs/JsonCodec.h"
#include "Codec/codecs/MsgPackCodec.h"
#include "DataFormat.h"

tl::optional<DataFormat> get_data_format_from_format_byte(uint8_t value);

uint8_t get_format_byte_from_data_format(DataFormat format);

std::shared_ptr<Codec> create_codec_from_format(DataFormat format);