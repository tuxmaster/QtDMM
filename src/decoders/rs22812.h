#pragma once
#include <QString>
#include <optional>

#include "dmmdecoder.h"

class DecoderRS22812 : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderRS22812(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t len);
  size_t getPacketLength();

private:
std::optional<QString> digit(uint8_t byte);
};
