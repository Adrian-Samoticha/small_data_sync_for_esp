#include "DataFormat_util.h"

tl::optional<DataFormat> get_data_format_from_format_byte(uint8_t value) {
  switch (value) {
    case 0x01:
      return DataFormat::JSON;

    case 0x02:
      return DataFormat::MSGPACK;

    default:
      return {};
  }
}

uint8_t get_format_byte_from_data_format(DataFormat format) {
  switch (format) {
    case DataFormat::JSON:
      return 0x01;

    case DataFormat::MSGPACK:
      return 0x02;

    default:
      return 0x01;
  }
}

std::shared_ptr<Codec> create_codec_from_format(DataFormat format) {
  switch (format) {
    case DataFormat::JSON:
      return std::make_shared<JsonCodec>();

    case DataFormat::MSGPACK:
      return std::make_shared<MsgPackCodec>();

    default:
      return std::make_shared<JsonCodec>();
  }
}