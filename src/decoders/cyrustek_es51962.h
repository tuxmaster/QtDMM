#pragma once

#include "dmmdecoder.h"

class DecoderCyrusTekES51962 : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderCyrusTekES51962(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t idx);
  size_t getPacketLength();
};
