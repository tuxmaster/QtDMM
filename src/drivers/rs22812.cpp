#include "rs22812.h"

// https://sigrok.org/wiki/RadioShack_22-812

std::optional<DmmDriver::DmmResponse> DrvRS22812::decode(const QByteArray &data, int id, ReadEvent::DataFormat /*df*/)
{
  m_result = {};
  m_result.id = id;
  m_result.range = "";
  m_result.hold = false;
  m_result.showBar = true;

  const char *in = data.data();
  QString val;

  // check for overflow
  if (((in[3] & 0x0f7) == 0x000) &&
      ((in[4] & 0x0f7) == 0x027) &&
      ((in[5] & 0x0f7) == 0x0d7))
  {
    val = "  0L  ";
  }
  else
  {
    val = (in[7] & 0x08) ? " -" : "  ";

    for (int i = 0; i < 4; ++i)
    {
      const char *digit = RS22812Digit(in[6 - i]);
      val += digit ? digit : "?";
    }
  }

  // decimal marker
  if (in[3] & 0x08)      val = insertComma(val, 3);
  else if (in[4] & 0x08) val = insertComma(val, 2);
  else if (in[5] & 0x08) val = insertComma(val, 1);

  m_result.val = val;
  double d_val = val.toDouble();
  QString unit;
  QString special;

  // special mode
  if (in[7] & 0x40)
    special = "DI";
  special = (in[7] & 0x04) ? "AC" : "DC";

  // base unit
  if (in[1] & 0x08)
  {
    unit = "F"; special = "CA";
  }
  else if (in[1] & 0x40)
  {
    unit = "Ohm"; special = "OH";
  }
  else if (in[1] & 0x04)
    unit = "A";
  else if (in[1] & 0x80)
  {
    unit = "Hz"; special = "HZ";
  }
  else if (in[1] & 0x02)
    unit = "V";
  else if (in[2] & 0x80)
  {
    unit = "%"; special = "PC";
  }
  else
  {
    qDebug() << "Unknown unit!";
    return std::nullopt;
  }

  // prefix
  if (in[2] & 0x40)      formatResultValue(0, "n", unit);
  else if (in[2] & 0x80) formatResultValue(0, "u", unit);
  else if (in[1] & 0x01) formatResultValue(0, "m", unit);
  else if (in[1] & 0x20) formatResultValue(0, "k", unit);
  else if (in[1] & 0x10) formatResultValue(0, "M", unit);

  m_result.special = special;

  return m_result;
}

const char *DrvRS22812::RS22812Digit( int byte )
{
  int           digit[10] = { 0xd7, 0x50, 0xb5, 0xf1, 0x72, 0xe3, 0xe7, 0x51, 0xf7, 0xf3 };
  const char *c_digit[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

  byte &= 0x0f7;

  for (int n=0; n<10; n++)
  {
	if (byte == digit[n])
		return c_digit[n];
  }
  return 0;
}
