#include "cyrustek_es51962.h"

// possibly same proto as iso-tech.
static const bool registered = []() {
  DmmDecoder::addConfig(  {"Uni-Trend","UT803","", 19200, 13, 7, 1, 1, 2, 6000, 0, 0,1});
  DmmDecoder::addConfig({"PeakTech 3315", 3, 10, 7, 1, 1, 0, 1, 0}
  return true;
}();

bool DecoderCyrusTekES51962::checkFormat(const char* data, size_t len, ReadEvent::DataFormat df)
{
  return (df == ReadEvent::CyrustekES51962 && len >= 12 && data[(len-1+FIFO_LENGTH)%FIFO_LENGTH] == 0x0d && data[len] == 0x0a);
}

size_t DecoderCyrusTekES51962::getPacketLength(ReadEvent::DataFormat df)
{
  return  (df == ReadEvent::CyrustekES51962 ? 11 : 0);
}

std::optional<DmmDecoder::DmmResponse> DecoderCyrusTekES51962::decode(const QByteArray &data, int id, ReadEvent::DataFormat /*df*/)
{
  return m_result;
}
