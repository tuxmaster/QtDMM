#include "ascii.h"

static const bool registered = []() {
  DmmDecoder::addConfig({"Digitech", "QM1350", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"MASTECH", "MAS-343", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"MASTECH", "MAS-345", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"McVoice", "M-345pro", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3660D", "", 1200, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3830D", "", 1200, 0, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3840D", "", 1200, 0, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3850D", "", 1200, 0, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3850M", "", 9600, 0, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3870D", "", 1200, 0, 7, 1, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-4650C", "", 1200, 0, 7, 2, 4, 0, 20000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "ME-11", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "ME-22", "", 2400, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "ME-32", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "ME-42", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "universal system 9160", "", 1200, 0, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "4010", "", 9600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "4015A", "", 9600, 0, 7, 2, 4, 0, 100000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "4360", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "4390", "", 9600, 0, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Radioshack", "22-805 DMM", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Radioshack", "RS22-168A", "", 1200, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Sinometer", "MAS-343", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-3610D", "", 1200, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-3650D", "", 1200, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-3860", "", 9600, 0, 7, 2, 4, 0, 20000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-4660", "", 1200, 0, 7, 2, 4, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "ME-11", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "ME-22T", "", 2400, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "ME-32", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "ME-42", "", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-4660A", "", 9600, 0, 7, 2, 4, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-4660M", "", 9600, 0, 7, 2, 4, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "MXD-4660A", "", 9600, 0, 7, 2, 4, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "451", "", 600, 1, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-4650CR", "", 1200, 2, 7, 2, 1, 0, 20000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 670", "", 4800, 2, 7, 1, 1, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 630", "", 4800, 2, 7, 1, 1, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 650", "", 4800, 2, 7, 1, 1, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 635", "", 2400, 3, 7, 1, 1, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 655", "", 2400, 3, 7, 1, 1, 0, 50000, 0, 0, 1});
  return true;
}();

bool DecoderAscii::checkFormat(const char* data, size_t idx)
{
  switch(m_type)
  {
    case ReadEvent::PeakTech10: return (data[(idx-11+FIFO_LENGTH)%FIFO_LENGTH] == '#');
    case ReadEvent::Metex14:
    case ReadEvent::Voltcraft14Continuous: return (data[idx] == 0x0d);
    case ReadEvent::Voltcraft15Continuous: return (data[(idx - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && data[idx] == 0x0a);
    default: return false;
  }
}

size_t DecoderAscii::getPacketLength()
{
  switch (m_type)
  {
    case ReadEvent::PeakTech10:            return 11;
    case ReadEvent::Metex14:               return 14;
    case ReadEvent::Voltcraft14Continuous: return 14;
    case ReadEvent::Voltcraft15Continuous: return 15;
    default: return 0;
  }
}

std::optional<DmmDecoder::DmmResponse> DecoderAscii::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id = id;
  m_result.range = "";
  m_result.hold = false;
  m_result.showBar = true;

  QString str(data);
  QString unit;

  if (m_type == ReadEvent::Metex14 ||
    m_type == ReadEvent::Voltcraft14Continuous ||
    m_type == ReadEvent::Voltcraft15Continuous)
  {
    m_result.val = str.mid(2, 7).trimmed();
    unit    = str.mid(9, 4).trimmed();
    m_result.special = str.left(3).trimmed();
  }
  else if (m_type == ReadEvent::PeakTech10)
  {
    m_result.val  = str.mid(1, 6).trimmed();
    unit = str.mid(7, 4).trimmed();
  }
  else
    return std::nullopt;

  switch (unit.length())
  {
    case 0: return std::nullopt;
    case 1: formatResultValue(0, "", unit); break;
    default:formatResultValue(0, unit.left(1), unit.mid(1)); break;
  }

  return m_result;
}
