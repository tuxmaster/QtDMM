#include "drivers/isotech.h"

std::optional<DmmDriver::DmmResponse> DrvIsoTech::decode(const QByteArray &data, int id, ReadEvent::DataFormat /*df*/)
{

  m_result = {};
  m_result.id     = id;
	m_result.showBar = true;
  m_result.hold   = false;
  m_result.range  = "";


  QString tmp(data);
  QString val = tmp.mid(11, 4);
  QString unit, special;
  const char *tmpstring = data.data();

  double d_val = val.toDouble();

  // Type of measurement
  int typecode = tmpstring[15] & 0xf;
  int rangecode = tmpstring[10] & 0xf;
  int acdccode = tmpstring[18] & 0xc;
  //  printf("ISO: value %s, type: %d, range: %d, ac/dc: %c\n",
  //      re->string(), typecode, rangecode, acdccode == 0x8 ? 'D': 'A' );

  double multiplier = 1.0;

  switch (typecode)
  {
    case 0xb:
      // voltage
      if (rangecode < 4)
        rangecode += 5;
      rangecode -= 4;
      multiplier = pow(10, rangecode - 4);
      if (acdccode == 0x8)
        special = "DC";
      else
        special = "AC";
      switch (rangecode)
      {
        case 0:
          val = insertCommaIT(val, 3);
          unit = "mV";
          break;
        case 1:
          val = insertCommaIT(val, 1);
          unit = "V";
          break;
        case 2:
          val = insertCommaIT(val, 2);
          unit = "V";
          break;
        case 3:
          val = insertCommaIT(val, 3);
          unit = "V";
          break;
        case 4:
          unit = "V";
          break;
        default:
          // error
          ;
      }
      break;
    case 0x3:
      // resistance
      multiplier = pow(10, rangecode - 1);
      special = "OH";
      switch (rangecode)
      {
        case 0:
          val = insertCommaIT(val, 3);
          unit = "Ohm";
          break;
        case 1:
          val = insertCommaIT(val, 1);
          unit = "kOhm";
          break;
        case 2:
          val = insertCommaIT(val, 2);
          unit = "kOhm";
          break;
        case 3:
          val = insertCommaIT(val, 3);
          unit = "kOhm";
          break;
        case 4:
          val = insertCommaIT(val, 1);
          unit = "MOhm";
          break;
        case 5:
          val = insertCommaIT(val, 2);
          unit = "MOhm";
          break;
        default:
          // error
          ;
      }
      break;
    case 0x1:
      // diode
      multiplier = 0.001;
      val = insertCommaIT(val, 1);
      unit = "V";
      break;
    case 0xd:
      // current micro
      multiplier = pow(10, rangecode - 7);
      special = "DC";
      if (rangecode == 0)
      {
        val = insertCommaIT(val, 3);
        unit = "uA";
      }
      else
      {
        val = insertCommaIT(val, 1);
        unit = "mA";
      }
      break;
    case 0x0:
      // current
      multiplier = pow(10, rangecode - 3);
      unit = "A";
      val = insertCommaIT(val, rangecode + 1);
      if (acdccode == 0x8)
        special = "DC";
      else
        special = "AC";
      break;
    case 0x2:
      // frequency
      special = "HZ";
      multiplier = pow(10, rangecode);
      switch (rangecode)
      {
        case 0x0:
          val = insertCommaIT(val, 1);
          unit = "kHz";
          break;
        case 0x1:
          val = insertCommaIT(val, 2);
          unit = "kHz";
          break;
        case 0x2:
          val = insertCommaIT(val, 3);
          unit = "kHz";
          break;
        case 0x3:
          val = insertCommaIT(val, 1);
          unit = "MHz";
          break;
        case 0x4:
          val = insertCommaIT(val, 2);
          unit = "MHz";
          break;
        default:
          // error
          ;

      }
      break;
    case 0x6:
      // capacity
      special = "CA";
      multiplier = pow(10, rangecode - 12);
      switch (rangecode)
      {
        case 0x0:
          val = insertCommaIT(val, 1);
          unit = "nF";
          break;
        case 0x1:
          val = insertCommaIT(val, 2);
          unit = "nF";
          break;
        case 0x2:
          val = insertCommaIT(val, 3);
          unit = "nF";
          break;
        case 0x3:
          val = insertCommaIT(val, 1);
          unit = "uF";
          break;
        case 0x4:
          val = insertCommaIT(val, 2);
          unit = "uF";
          break;
        case 0x5:
          val = insertCommaIT(val, 3);
          unit = "uF";
          break;
        case 0x6:
          val = insertCommaIT(val, 1);
          unit = "mF";
          break;
        default:
          // error
          ;
      }
      break;
    default:
      // error - unknown type of measurement
      ;
  }
  d_val *= multiplier;

  if (tmpstring[16] & 0x01)
    val = "  0L";
  if (tmpstring[16] & 0x4)
  {
    d_val = - d_val;
    val = " -" + val;
  }
  else
    val = "  " + val;

// printf ("DVAL: %f\n", d_val);

  m_result.dval   = d_val;
  m_result.special= special;
  m_result.val    = val;
  m_result.unit  = unit;

  return m_result;
}
