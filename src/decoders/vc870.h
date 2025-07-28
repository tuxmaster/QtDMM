#pragma once

#include "dmmdecoder.h"

class DecoderVC870 : public DmmDecoder
{
    Q_OBJECT
public:
  DecoderVC870(ReadEvent::DataFormat df) : DmmDecoder(df)  {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t idx);
  size_t getPacketLength();
};
