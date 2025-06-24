#include "drivers/vc940.h"

std::optional<DmmDriver::DmmResponse> DrvVC940::decode(const QByteArray &data, int id, ReadEvent::DataFormat /*df*/)
{
  m_result = {};
  m_result.id     = id;
	m_result.showBar = true;
  m_result.hold   = false;
  m_result.range  = "";

  QString val = " ";
  QString special;
  QString unit;

  QString str(data);

  double scale = 1.0;

  int function = data[6] & 0x0f;
  int range    = data[5] & 0x0f;
  int mode     = data[7];
  int mode2    = data[8];

  if (function != 12 && mode2 & 0x04) val = "-";

  for (int i = 0; i < 4; ++i)
    val += str[i];
  if (data[4] != 'A')
    val += str[4];
  switch (function)
  {
    case 0:
      val = insertCommaIT(val, 3);
      special = "AC";
      unit = "mV";
      scale = 1e-3;
      break;
    case 1:
      val = insertCommaIT(val, range);
      special = "DC";
      unit = "V";
      break;
    case 2:
      val = insertCommaIT(val, range);
      special = "AC";
      unit = "V";
      break;
    case 3:
      val = insertCommaIT(val, 3);
      special = "DC";
      unit = "mV";
      scale = 1e-3;
      break;
    case 4:
      unit = "Ohm";
      switch (range)
      {
        case 1:
          val = insertCommaIT(val, 3);
          break;
        case 2:
          val = insertCommaIT(val, 1);
          unit = "kOhm";
          scale = 1e3;
          break;
        case 3:
          val = insertCommaIT(val, 2);
          unit = "kOhm";
          scale = 1e3;
          break;
        case 4:
          val = insertCommaIT(val, 3);
          unit = "kOhm";
          scale = 1e3;
          break;
        case 5:
          val = insertCommaIT(val, 1);
          unit = "MOhm";
          scale = 1e6;
          break;
        case 6:
          val = insertCommaIT(val, 2);
          unit = "MOhm";
          scale = 1e6;
          break;
      }
      special = "OH";
      break;
    case 5:
      unit = "F";
      switch (range)
      {
        case 1:
          val = insertCommaIT(val, 2);
          unit = "nF";
          scale = 1e-9;
          break;
        case 2:
          val = insertCommaIT(val, 3);
          unit = "nF";
          scale = 1e-9;
          break;
        case 3:
          val = insertCommaIT(val, 1);
          unit = "uF";
          scale = 1e-6;
          break;
        case 4:
          val = insertCommaIT(val, 2);
          unit = "uF";
          scale = 1e-6;
          break;
        case 5:
          val = insertCommaIT(val, 3);
          unit = "uF";
          scale = 1e-6;
          break;
        case 6:
          val = insertCommaIT(val, 4);
          unit = "uF";
          scale = 1e-6;
          break;
        case 7:
          val = insertCommaIT(val, 2);
          unit = "mF";
          scale = 1e-3;
          break;
      }
      special = "CA";
      break;
    case 6:
      special = "TE";
      unit = "C";
      val = insertCommaIT(val, 4);
      break;
    case 7:
      if (mode & 0x01)
      {
        // can't handle AC+DC
        special = "AC";
      }
      else
        special = "DC";
      switch (range)
      {
        case 0:
          val = insertCommaIT(val, 3);
          break;
        case 1:
          val = insertCommaIT(val, 4);
          break;
      }
      unit = "uA";
      scale = 1e-6;
      break;
    case 8:
      if (mode & 0x01)
      {
        // can't handle AC+DC
        special = "AC";
      }
      else
        special = "DC";
      switch (range)
      {
        case 0:
          val = insertCommaIT(val, 2);
          break;
        case 1:
          val = insertCommaIT(val, 3);
          break;
      }
      unit = "mA";
      scale = 1e-3;
      break;
    case 9:
      if (mode & 0x01)
      {
        // can't handle AC+DC
        special = "AC";
      }
      else
        special = "DC";
      val = insertCommaIT(val, 2);
      unit = "A";
      break;
    case 10:   // buzzer
      special = "BUZ";
      unit = "Ohm";
      val = insertCommaIT(val, 3);
      break;
    case 11:
      special = "DI";
      unit = "V";
      val = insertCommaIT(val, 1);
      break;
    case 12:
      if (mode2 & 0x04)
      {
        special = "PC";
        unit = "%";
        val = insertCommaIT(val, 3);
      }
      else
      {
        special = "FR";
        unit = "Hz";
        switch (range)
        {
          case 0:
            val = insertCommaIT(val, 2);
            break;
          case 1:
            val = insertCommaIT(val, 3);
            break;
          case 2:
            val = insertCommaIT(val, 1);
            unit = "kHz";
            scale = 1e3;
            break;
          case 3:
            val = insertCommaIT(val, 2);
            unit = "kHz";
            scale = 1e3;
            break;
          case 4:
            val = insertCommaIT(val, 3);
            unit = "kHz";
            scale = 1e3;
            break;
          case 5:
            val = insertCommaIT(val, 1);
            unit = "MHz";
            scale = 1e6;
            break;
          case 6:
            val = insertCommaIT(val, 2);
            unit = "MHz";
            scale = 1e6;
            break;
          case 7:
            val = insertCommaIT(val, 3);
            unit = "MHz";
            scale = 1e6;
            break;
        }
      }
      break;
    case 13:
      special = "TE";
      unit = "F";
      val = insertCommaIT(val, 4);
      break;

  }

  double d_val = val.toDouble() * scale;

  m_result.dval   = d_val;
  m_result.special= special;
  m_result.val    = val;
  m_result.unit  = unit;

  return m_result;
}
