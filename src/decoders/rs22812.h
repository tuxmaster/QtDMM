#pragma once
#include <QString>
#include <optional>

#include "dmmdecoder.h"

/// RadioShack 22-812: 9 bytes, seven-segment encoded digits, last byte is a
/// checksum (sum of the first eight plus 57).
class DecoderRS22812 : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderRS22812(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t idx);
  size_t getPacketLength();

private:
std::optional<QString> digit(uint8_t byte);
};
