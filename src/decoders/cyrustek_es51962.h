#pragma once

#include "dmmdecoder.h"

/// Cyrustek ES51962 chip (PeakTech 3315, Uni-Trend UT70B): 11 bytes, sent
/// twice per measurement, only the second copy is decoded. Spec in
/// docs/protocols/spec/cyrustek_es51962.yaml.
class DecoderCyrusTekES51962 : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderCyrusTekES51962(ReadEvent::DataFormat df) : DmmDecoder(df) {};

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t idx);
  size_t getPacketLength();
};
