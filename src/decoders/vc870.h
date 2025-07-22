#pragma once

#include "dmmdecoder.h"

class DecoderVC870 : public DmmDecoder
{
    Q_OBJECT
public:
  DecoderVC870(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);
  bool checkFormat(const char* data, size_t len, ReadEvent::DataFormat df);
  size_t getPacketLength(ReadEvent::DataFormat df);
};
