#pragma once

#include "dmmdecoder.h"

class DecoderCyrusTekES51962 : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderCyrusTekES51962(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id, ReadEvent::DataFormat df);
  bool checkFormat(const char* data, size_t len, ReadEvent::DataFormat df);
  size_t getPacketLength(ReadEvent::DataFormat df);
};
