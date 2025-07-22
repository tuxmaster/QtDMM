#pragma once

#include "dmmdecoder.h"

class DecoderM9803R : public DmmDecoder
{
    Q_OBJECT
public:
  DecoderM9803R(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);
  bool checkFormat(const char* data, size_t len, ReadEvent::DataFormat df);
  size_t getPacketLength(ReadEvent::DataFormat df);
};
