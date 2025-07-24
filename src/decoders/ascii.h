#pragma once

#include "dmmdecoder.h"

class DecoderAscii : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderAscii(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t len);
  size_t getPacketLength();
};
