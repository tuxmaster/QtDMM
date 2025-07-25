#pragma once

#include "dmmdecoder.h"

class DecoderDO3122 : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderDO3122(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t idx);
  size_t getPacketLength();

private:
  const char* digit(int byte, bool *convOk);
};
