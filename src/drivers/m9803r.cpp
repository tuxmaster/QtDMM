#include "drivers/m9803r.h"


static const bool registered = []() {
  DmmDriver::addConfig({"ELV", "M9803R", "ELV M9803R", 9600, 4, 7, 1, 1, 1, 4000, 0, 0, 1, 1, 1});
  DmmDriver::addConfig({"MASTECH", "M9803R", "MASTECH M9803R", 9600, 4, 7, 1, 1, 1, 4000, 0, 0, 1, 1, 1});
  DmmDriver::addConfig({"McVoice", "M-980T", "McVoice M-980T", 9600, 4, 7, 1, 1, 0, 4000, 0, 0, 1, 1, 1});
  return true;
}();

size_t DrvM9803R::getPacketLength(ReadEvent::DataFormat df)
{
  return  (df == ReadEvent::M9803RContinuous ? 11 : 0);
}


bool DrvM9803R::checkFormat(const char* data, size_t len, ReadEvent::DataFormat df)
{
  return (df==ReadEvent::M9803RContinuous && len >= 10 && data[(len - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && data[len] == 0x0a);
}


std::optional<DmmDriver::DmmResponse> DrvM9803R::decode(const QByteArray &data, int id, ReadEvent::DataFormat /*df*/)
{
  m_result = {};
  m_result.id     = id;
  m_result.hold   = false;
  m_result.range  = "";

  QString val;
  QString special;
  QString unit;

  if (data[0] & 0x01)
    val = "  0L";
  else
  {
    val = data[0] == 0x08 ? " -" : "  ";
    val += QChar('0' + data[4]);
    val += QChar('0' + data[3]);
    val += QChar('0' + data[2]);
    val += QChar('0' + data[1]);
  }

  double d_val = val.toDouble();

  bool showBar = true;

  switch (data[5])
  {
    case 0x00:
      switch (data[6])
      {
        case 0x00:
          unit = "mV";
          val = insertComma(val, 3);
          d_val /= 10000.;
          break;
        case 0x01:
          unit = "V";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x02:
          unit = "V";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x03:
          unit = "V";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
        case 0x04:
          unit = "V";
          break;
      }
      special = "DC";
      break;
    case 0x01:
      switch (data[6])
      {
        case 0x00:
          unit = "mV";
          val = insertComma(val, 3);
          d_val /= 10000.;
          break;
        case 0x01:
          unit = "V";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x02:
          unit = "V";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x03:
          unit = "V";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
        case 0x04:
          unit = "V";
          break;
      }
      special = "AC";
      break;
    case 0x02:
      switch (data[6])
      {
        case 0x00:
          unit = "mA";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x01:
          unit = "mA";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x02:
          unit = "mA";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
      }
      special = "DC";
      break;
    case 0x03:
      switch (data[6])
      {
        case 0x00:
          unit = "mA";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x01:
          unit = "mA";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x02:
          unit = "mA";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
      }
      special = "AC";
      break;
    case 0x04:
      switch (data[6])
      {
        case 0x00:
          unit = "Ohm";
          val = insertComma(val, 1);
          d_val /= 10.;
          break;
        case 0x01:
          unit = "kOhm";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x02:
          unit = "kOhm";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x03:
          unit = "kOhm";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
        case 0x04:
          unit = "kOhm";
          break;
        case 0x05:
          unit = "MOhm";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
      }
      special = "OH";
      break;
    case 0x05:
      switch (data[6])
      {
        case 0x00:
          unit = "Ohm";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
      }
      special = "OH";
      break;
    case 0x06:
      unit = "V";
      val = insertComma(val, 1);
      d_val /= 1000.;
      special = "DI";
      break;
    case 0x08:
      unit = "A";
      val = insertComma(val, 2);
      d_val /= 100.;
      special = "DC";
      break;
    case 0x09:
      unit = "A";
      val = insertComma(val, 2);
      d_val /= 100.;
      special = "AC";
      break;
    case 0x0A:
      showBar = false;
      switch (data[6])
      {
        case 0x00:
          unit = "kHz";
          val = insertComma(val, 1);
          //d_val /= 1000.;
          break;
        case 0x01:
          unit = "kHz";
          val = insertComma(val, 2);
          d_val *= 10.;
          break;
        case 0x05:
          unit = "Hz";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x06:
          unit = "Hz";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
      }
      special = "HZ";
      break;
    case 0x0C:
      switch (data[6])
      {
        case 0x00:
          unit = "nF";
          val = insertComma(val, 1);
          d_val /= 1e12;
          break;
        case 0x01:
          unit = "nF";
          val = insertComma(val, 2);
          d_val /= 1e11;
          break;
        case 0x02:
          unit = "nF";
          val = insertComma(val, 3);
          d_val /= 1e10;
          break;
        case 0x03:
          unit = "uF";
          val = insertComma(val, 1);
          d_val /= 1e9;
          break;
        case 0x04:
          unit = "uF";
          val = insertComma(val, 2);
          d_val /= 1e8;
          break;
      }
      special = "CA";
      break;
  }

  m_result.dval   = d_val;
  m_result.special= special;
  m_result.val    = val;
  m_result.unit  = unit;
  m_result.showBar = showBar;

  return m_result;
}
