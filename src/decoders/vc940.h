#pragma once

#include "dmmdecoder.h"

/// Voltcraft VC920/940/960, Uni-Trend UT71/UT804, Tenma 72-7732: 11 bytes.
/// Spec in docs/protocols/spec/vc940.yaml.
class DecoderVC940 : public DmmDecoder
{
    Q_OBJECT
public:
  DecoderVC940(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id) override;
  bool checkFormat(const char* data, size_t idx) override;
  size_t getPacketLength() override;
};
