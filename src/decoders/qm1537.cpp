#include "qm1537.h"

// FS9922-DMM4

static const bool registered = []() {
  DmmDecoder::addConfig({"Digitek", "DT4000ZC", "", 2400, 8, 8, 1, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Digitech", "QM1537", "", 2400, 8, 8, 1, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"PeakTech", "3430", "", 19200, 8, 7, 2, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"TekPower", "TP4000ZC", "", 2400, 8, 8, 1, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Uni-Trend", "UT61B", "", 2400, 8, 8, 1, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Uni-Trend", "UT61C", "", 2400, 8, 8, 1, 1, 0, 6000, 0, 0, 1});
  DmmDecoder::addConfig({"Uni-Trend", "UT61D", "", 2400, 8, 8, 1, 1, 0, 6000, 0, 0, 1});
  DmmDecoder::addConfig({"Vichy", "VC99", "", 2400, 8, 8, 1, 1, 0, 6000, 0, 0, 1});
  return true;
}();

size_t DecoderQM1537::getPacketLength()
{
  return  (m_type == ReadEvent::QM1537Continuous ? 14 : 0);
}

bool DecoderQM1537::checkFormat(const char* data, size_t idx)
{
  return (m_type == ReadEvent::QM1537Continuous && data[idx] == 0x0d);
}

std::optional<DmmDecoder::DmmResponse> DecoderQM1537::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id     = id;
  m_result.hold   = bit(data,7,1);
  m_result.range  = bit(data,7,5) ? "AUTO" : "MANU";
  m_result.showBar= bit(data,7,0);
  m_result.special = (data[7] & 0x08) ? "AC" : "DC";

  if (data[0] == '-')
  {
    m_result.val = " -";
  }
  else if(data[0] == '+')
  {
    m_result.val = "";
  }
  else goto error;

  for(int i=1;i<4;i++)
  {
    if(data[i]<'0')
      goto error;
    if(data[i]>'9')
      goto error;
  }

  if(0)
  {
    error:
      printf("Data error!\n");
      m_result.error = "Data error!";
      return m_result;
  }

  if ((data[1] == ';') &&
      (data[2] == '0') &&
      (data[3] == ':') &&
      (data[4] == ';'))
  {
     m_result.val += "  0L";
  }
  else
    m_result.val += makeValue(data,1,4);

  bool doUnits = true;

  switch (data[6])
  {
    case 0x31: m_result.val = insertCommaIT (m_result.val,1); break;
    case 0x32: m_result.val = insertCommaIT (m_result.val,2); break;
    case 0x34: m_result.val = insertCommaIT (m_result.val,3); break;
    // default case is no comma/decimal point at all.
  }

  m_result.dval = m_result.val.toDouble();

  /* OK, now let's figure out what we're looking at. */
  if (data[10] & 0x80)
  {
    /* Voltage, including diode test */
    m_result.unit = "V";
    if (data[9] & 0x04)
    {
      /* Diode test */
      m_result.special = "DI";
      m_result.unit = "V";
    }
  }
  else if (data[10] & 0x40)
  {
    /* Current */
    m_result.unit = "A";
  }
  else if (data[10] & 0x20)
  {
    /* Resistance, including continuity test */
    m_result.unit = "Ohm";
    m_result.special = "OH";
  }
  else if (data[10] & 0x08)
  {
    /* Frequency */
    m_result.unit = "Hz";
    m_result.special = "HZ";
  }
  else if (data[10 ] & 0x04)
  {
    /* Capacitance */
    m_result.unit = "F";
    m_result.special = "CA";
  }
  else if (data[9] & 0x02)
  {
    /* Duty cycle */
    m_result.unit = "%";
    m_result.special = "PC";
    doUnits = false;
  }

  if (doUnits)
  {
    if (data[8] & 0x02)      m_result.unit.prepend ('n');
    else if (data[9] & 0x80) m_result.unit.prepend ('u');
    else if (data[9] & 0x40) m_result.unit.prepend ('m');
    else if (data[9] & 0x20) m_result.unit.prepend ('k');
    else if (data[9] & 0x10) m_result.unit.prepend ('M');
  }

 return m_result;
}
