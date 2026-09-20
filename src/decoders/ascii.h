#pragma once

#include "dmmdecoder.h"

/// The ASCII protocol family: Metex14 (14 bytes, polled with 'D'),
/// PeakTech10, Voltcraft14Continuous, Voltcraft15Continuous and the
/// sigrok-cli text lines (30 bytes, see SigrokDevice). Frames are readable
/// text "<mode> <value> <unit>\r".
class DecoderAscii : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderAscii(ReadEvent::DataFormat df) : DmmDecoder(df) {}

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id);
  bool checkFormat(const char* data, size_t len);
  size_t getPacketLength();

private:
  bool decodeSigrok(QString str);
};
