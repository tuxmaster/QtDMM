#include "ascii.h"

std::optional<DmmDriver::DmmResponse> DrvAscii::decode(const QByteArray &data, int id, ReadEvent::DataFormat df)
{
  m_result = {};
  m_result.id = id;
  m_result.range = "";
  m_result.hold = false;
  m_result.showBar = true;

  QString str(data);
  QString unit;

  if (df == ReadEvent::Metex14 ||
    df == ReadEvent::Voltcraft14Continuous ||
    df == ReadEvent::Voltcraft15Continuous)
  {
    m_result.val = str.mid(2, 7).trimmed();
    unit    = str.mid(9, 4).trimmed();
    m_result.special = str.left(3).trimmed();
  }
  else if (df == ReadEvent::PeakTech10)
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
