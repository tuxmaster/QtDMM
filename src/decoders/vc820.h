#pragma once

#include "dmmdecoder.h"

/// Voltcraft VC820/840, Uni-Trend UT60A/E and clones (FS9721 chip): 14
/// bytes, each carrying a 4-bit sequence number in the high nibble and four
/// segment bits in the low nibble. Spec in docs/protocols/spec/vc820.yaml.
class DecoderVC820 : public DmmDecoder
{
    Q_OBJECT
public:
  DecoderVC820(ReadEvent::DataFormat df) : DmmDecoder(df) {m_name="VC820";}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id) override;
  bool checkFormat(const char* data, size_t idx) override;
  size_t getPacketLength() override;

private:
  const char* vc820Digit(int byte);
};
