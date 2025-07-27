#pragma once

#include "dmmdecoder.h"

class DecoderCyrusTekES51922 : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderCyrusTekES51922(ReadEvent::DataFormat df) : DmmDecoder(df) {};

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t idx);
  size_t getPacketLength();
};
