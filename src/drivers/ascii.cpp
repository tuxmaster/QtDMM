#include "ascii.h"

static const bool registered = []() {
  DmmDriver::m_configurations.push_back({"Digitech", "QM1350", "Digitech QM1350", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"MASTECH", "MAS-343", "MASTECH MAS-343", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"MASTECH", "MAS-345", "MASTECH MAS-345", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"McVoice", "M-345pro", "McVoice M-345pro", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "M-3660D", "Metex M-3660D", 1200, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "M-3830D", "Metex M-3830D", 1200, 0, 7, 2, 4, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "M-3840D", "Metex M-3840D", 1200, 0, 7, 2, 4, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "M-3850D", "Metex M-3850D", 1200, 0, 7, 2, 4, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "M-3850M", "Metex M-3850M", 9600, 0, 7, 2, 4, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "M-3870D", "Metex M-3870D", 1200, 0, 7, 1, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "M-4650C", "Metex M-4650C", 1200, 0, 7, 2, 4, 0, 20000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "ME-11", "Metex ME-11", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "ME-22", "Metex ME-22", 2400, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "ME-32", "Metex ME-32", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "ME-42", "Metex ME-42", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Metex", "universal system 9160", "Metex universal system 9160", 1200, 0, 7, 2, 4, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"PeakTech", "4010", "PeakTech 4010", 9600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"PeakTech", "4015A", "PeakTech 4015A", 9600, 0, 7, 2, 4, 0, 100000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"PeakTech", "4360", "PeakTech 4360", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"PeakTech", "4390", "PeakTech 4390", 9600, 0, 7, 2, 4, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Radioshack", "22-805 DMM", "Radioshack 22-805 DMM", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Radioshack", "RS22-168A", "Radioshack RS22-168A", 1200, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Sinometer", "MAS-343", "Sinometer MAS-343", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "M-3610D", "Voltcraft M-3610D", 1200, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "M-3650D", "Voltcraft M-3650D", 1200, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "M-3860", "Voltcraft M-3860", 9600, 0, 7, 2, 4, 0, 20000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "M-4660", "Voltcraft M-4660", 1200, 0, 7, 2, 4, 0, 50000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "ME-11", "Voltcraft ME-11", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "ME-22T", "Voltcraft ME-22T", 2400, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "ME-32", "Voltcraft ME-32", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "ME-42", "Voltcraft ME-42", 600, 0, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "M-4660A", "Voltcraft M-4660A", 9600, 0, 7, 2, 4, 0, 50000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "M-4660M", "Voltcraft M-4660M", 9600, 0, 7, 2, 4, 0, 50000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "MXD-4660A", "Voltcraft MXD-4660A", 9600, 0, 7, 2, 4, 0, 50000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"PeakTech", "451", "PeakTech 451", 600, 1, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "M-4650CR", "Voltcraft M-4650CR", 1200, 2, 7, 2, 1, 0, 20000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "VC 670", "Voltcraft VC 670", 4800, 2, 7, 1, 1, 0, 50000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "VC 630", "Voltcraft VC 630", 4800, 2, 7, 1, 1, 0, 50000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "VC 650", "Voltcraft VC 650", 4800, 2, 7, 1, 1, 0, 50000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "VC 635", "Voltcraft VC 635", 2400, 3, 7, 1, 1, 0, 50000, 0, 0, 1, 1, 1});
  DmmDriver::m_configurations.push_back({"Voltcraft", "VC 655", "Voltcraft VC 655", 2400, 3, 7, 1, 1, 0, 50000, 0, 0, 1, 1, 1});
  return true;
}();

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
