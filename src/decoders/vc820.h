#pragma once

#include "dmmdecoder.h"

class DecoderVC820 : public DmmDecoder
{
    Q_OBJECT
public:
  DecoderVC820(ReadEvent::DataFormat df) : DmmDecoder(df) {m_name=tr("VC820");}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t idx);
  size_t getPacketLength();

private:
  const char* vc820Digit(int byte);
};
