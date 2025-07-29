#include "ascii.h"

static const bool registered = []() {
  DmmDecoder::addConfig({"Digitech", "QM1350", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"MASTECH", "MAS-343", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"MASTECH", "MAS-345", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"McVoice", "M-345pro", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3660D", "", 1200, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3830D", "", 1200, ReadEvent::Metex14, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3840D", "", 1200, ReadEvent::Metex14, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3850D", "", 1200, ReadEvent::Metex14, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3850M", "", 9600, ReadEvent::Metex14, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-3870D", "", 1200, ReadEvent::Metex14, 7, 1, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "M-4650C", "", 1200, ReadEvent::Metex14, 7, 2, 4, 0, 20000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "ME-11", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "ME-22", "", 2400, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "ME-32", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "ME-42", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Metex", "universal system 9160", "", 1200, ReadEvent::Metex14, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "4010", "", 9600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "4015A", "", 9600, ReadEvent::Metex14, 7, 2, 4, 0, 100000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "4360", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "4390", "", 9600, ReadEvent::Metex14, 7, 2, 4, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Radioshack", "22-805 DMM", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Radioshack", "RS22-168A", "", 1200, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Sinometer", "MAS-343", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-3610D", "", 1200, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-3650D", "", 1200, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-3860", "", 9600, ReadEvent::Metex14, 7, 2, 4, 0, 20000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-4660", "", 1200, ReadEvent::Metex14, 7, 2, 4, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "ME-11", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "ME-22T", "", 2400, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "ME-32", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "ME-42", "", 600, ReadEvent::Metex14, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-4660A", "", 9600, ReadEvent::Metex14, 7, 2, 4, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-4660M", "", 9600, ReadEvent::Metex14, 7, 2, 4, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "MXD-4660A", "", 9600, ReadEvent::Metex14, 7, 2, 4, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "451", "", 600, ReadEvent::PeakTech10, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "M-4650CR", "", 1200, ReadEvent::Voltcraft14Continuous, 7, 2, 1, 0, 20000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 670", "", 4800, ReadEvent::Voltcraft14Continuous, 7, 1, 1, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 630", "", 4800, ReadEvent::Voltcraft14Continuous, 7, 1, 1, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 650", "", 4800, ReadEvent::Voltcraft14Continuous, 7, 1, 1, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 635", "", 2400, ReadEvent::Voltcraft15Continuous, 7, 1, 1, 0, 50000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 655", "", 2400,  ReadEvent::Voltcraft15Continuous, 7, 1, 1, 0, 50000, 0, 0, 1});
  return true;
}();

bool DecoderAscii::checkFormat(const char* data, size_t idx)
{
  switch(m_type)
  {
    case ReadEvent::PeakTech10: return (data[(idx-11+FIFO_LENGTH)%FIFO_LENGTH] == '#');
    case ReadEvent::Metex14:
    case ReadEvent::Voltcraft14Continuous: return (data[idx] == 0x0d);
    case ReadEvent::Sigrok:                return (data[idx] == 0x0a);
    case ReadEvent::Voltcraft15Continuous: return (data[(idx - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && data[idx] == 0x0a);
    default: return false;
  }
}

size_t DecoderAscii::getPacketLength()
{
  switch (m_type)
  {
    case ReadEvent::Sigrok:                return 30;
    case ReadEvent::PeakTech10:            return 11;
    case ReadEvent::Metex14:               return 14;
    case ReadEvent::Voltcraft14Continuous: return 14;
    case ReadEvent::Voltcraft15Continuous: return 15;
    default: return 0;
  }
}

bool DecoderAscii::decodeSigrok(QString str)
{
  QStringList list = str.trimmed().split(" ");
  if (list.size()<3)
    return false;
  m_result.val = list[1];
  if (m_result.val == "inf")
  {
    m_result.val = " OL ";
  }

  m_result.unit = list[2];

  for(auto const& item : list)
  {
    if (item == "HOLD") m_result.hold = true;
    if (item == "DC") m_result.special = "DC";
    else if (item == "AC") m_result.special = "AC";
    else if (item == "DIODE") m_result.special = "DI";
    m_result.range = (item == "AUTO") ? "AUTO" : "MANU";
  }
  return true;
}


std::optional<DmmDecoder::DmmResponse> DecoderAscii::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id = id;
  m_result.range = "";
  m_result.hold = false;
  m_result.showBar = true;
  QStringList unit_prefixes = {"k","M","G","m","µ","u","n","p"};
    QString str(data);

  switch (m_type)
  {
    case ReadEvent::Metex14:
    case ReadEvent::Voltcraft14Continuous:
    case ReadEvent::Voltcraft15Continuous:
      m_result.val     = str.mid(2, 7).trimmed();
      m_result.unit    = str.mid(9, 4).trimmed();
      m_result.special = str.left(3).trimmed();
      break;
    case ReadEvent::PeakTech10:
      m_result.val  = str.mid(1, 6).trimmed();
      m_result.unit = str.mid(7, 4).trimmed();
      break;
    case ReadEvent::Sigrok:
      if (!decodeSigrok(str))
        return std::nullopt;
      break;
    default:
      return std::nullopt;
  }

  m_result.dval = m_result.val.toDouble();

  switch (m_result.unit.length())
  {
    case 0:
      if (m_type!=ReadEvent::Sigrok) return std::nullopt;
        formatResultValue(0, "", m_result.unit);
        break;
    case 1:
      formatResultValue(0, "", m_result.unit);
      break;
    default:
      if (unit_prefixes.contains(m_result.unit.left(1)))
        formatResultValue(0, m_result.unit.left(1), m_result.unit.mid(1));
      else
        formatResultValue(0, "", m_result.unit);
      break;
  }
  return m_result;
}
