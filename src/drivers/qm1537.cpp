#include "qm1537.h"

static const bool registered = []() {
  DmmDriver::addConfig({"Digitek", "DT4000ZC", "Digitek DT4000ZC", 2400, 8, 8, 1, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::addConfig({"Digitech", "QM1537", "Digitech QM1537", 2400, 8, 8, 1, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::addConfig({"PeakTech", "3430", "PeakTech 3430", 19200, 8, 7, 2, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::addConfig({"TekPower", "TP4000ZC", "TekPower TP4000ZC", 2400, 8, 8, 1, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::addConfig({"Uni-Trend", "UT61B", "Uni-Trend UT61B", 2400, 8, 8, 1, 1, 0, 4000, 0, 0, 1, 1, 1});
  DmmDriver::addConfig({"Uni-Trend", "UT61C", "Uni-Trend UT61C", 2400, 8, 8, 1, 1, 0, 6000, 0, 0, 0, 0, 1});
  DmmDriver::addConfig({"Uni-Trend", "UT61D", "Uni-Trend UT61D", 2400, 8, 8, 1, 1, 0, 6000, 0, 0, 0, 0, 1});
  DmmDriver::addConfig({"Vichy", "VC99", "Vichy VC99", 2400, 8, 8, 1, 1, 0, 6000, 0, 0, 0, 0, 1});
  return true;
}();


std::optional<DmmDriver::DmmResponse> DrvQM1537::decode(const QByteArray &data, int id, ReadEvent::DataFormat /*df*/)
{
  m_result = {};
  m_result.id     = id;
  m_result.hold   = false;
  m_result.range  = "";

  QString val;
  QString special;
  QString unit;
  const char *pStr = data.data();

  if (pStr[0] != 0x0A)
    return m_result;

  val = (pStr[1] == '-') ? " -" : val = "  ";

  if ((pStr[2] == ';') &&
      (pStr[3] == '0') &&
      (pStr[4] == ':') &&
      (pStr[5] == ';'))
    val += "  0L";
  else
  {
    val += pStr[2];
    val += pStr[3];
    val += pStr[4];
    val += pStr[5];
  }

  bool doACDC = false;
  bool doUnits = true;

  switch (pStr[7])
  {
    case 0x31:
      val = insertComma(val, 1);
      break;
    case 0x32:
      val = insertComma(val, 2);
      break;
    case 0x34:
      val = insertComma(val, 3);
      break;
      // default case is no comma/decimal point at all.
  }

  double d_val = val.toDouble();

  /* OK, now let's figure out what we're looking at. */
  if (pStr[11] & 0x80)
  {
    /* Voltage, including diode test */
    unit = "V";
    if (pStr[10] & 0x04)
    {
      /* Diode test */
      special = "DI";
      unit = "V";
    }
    else
      doACDC = true;
  }
  else if (pStr[11] & 0x40)
  {
    /* Current */
    unit = "A";
    doACDC = true;
  }
  else if (pStr[11] & 0x20)
  {
    /* Resistance, including continuity test */
    unit = "Ohm";
    special = "OH";
  }
  else if (pStr[11] & 0x08)
  {
    /* Frequency */
    unit = "Hz";
    special = "HZ";
  }
  else if (pStr[11] & 0x04)
  {
    /* Capacitance */
    unit = "F";
    special = "CA";
  }
  else if (pStr[10] & 0x02)
  {
    /* Duty cycle */
    unit = "%";
    special = "PC";
    doUnits = false;
  }

  if (doACDC)
  {
    special = (pStr[8] & 0x08) ? "AC" : "DC";
  }

  if (doUnits)
  {
    if (pStr[9] & 0x02)
    {
      d_val /= 1e9;
      unit.prepend('n');
    }
    else if (pStr[10] & 0x80)
    {
      d_val /= 1e6;
      unit.prepend('u');
    }
    else if (pStr[10] & 0x40)
    {
      d_val /= 1e3;
      unit.prepend('m');
    }
    else if (pStr[10] & 0x20)
    {
      d_val *= 1e3;
      unit.prepend('k');
    }
    else if (pStr[10] & 0x10)
    {
      d_val *= 1e6;
      unit.prepend('M');
    }
  }

  m_result.dval   = d_val;
  m_result.special= special;
  m_result.val    = val;
  m_result.unit   = unit;
  m_result.showBar= true;

 return m_result;
}
