#pragma once

#include "dmmdecoder.h"

/// Mastech M9803R and clones (ELV M9803R, McVoice M-980T): 11 binary bytes
/// (sign, four digit bytes, mode, range, ...) terminated by CR LF.
class DecoderM9803R : public DmmDecoder
{
    Q_OBJECT
public:
  DecoderM9803R(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t idx);
  size_t getPacketLength();
};
