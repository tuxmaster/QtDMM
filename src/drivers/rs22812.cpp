#include "rs22812.h"

// https://sigrok.org/wiki/RadioShack_22-812
static const bool registered = []() {
  DmmDriver::addConfig({"Radioshack", "22-812", "Radioshack 22-812", 4800, 9, 8, 1, 1, 0, 4000, 0, 0, 1, 1, 1});
  return true;
}();

size_t DrvRS22812::getPacketLength(ReadEvent::DataFormat df)
{
  return  (df == ReadEvent::RS22812Continuous ? 9 : 0);
}

bool DrvRS22812::checkFormat(const char* data, size_t len, ReadEvent::DataFormat df)
{
  if (df == ReadEvent::RS22812Continuous)
  {
    unsigned int checksum = 0x00;
    unsigned char byte;
    char mode, unit, multiplier, dp, minmax, rs232;
    int offset = 0;

    if (len > 9)
      offset = len - 9;
    // mode
    byte = static_cast<unsigned char>(data[offset]);
    mode = byte;
    if (mode > 25)
    {
      (void)fprintf(stderr, "bad mode %#x\n", mode);
      return false;
    }
    // units and multipliers
    unit = 0;
    multiplier = 0;
    minmax = 0;
    if (len >= 1)
    {
      byte = static_cast<unsigned char>(data[offset + 1]);
      if (byte & 0x01) // m
        multiplier++;
      if (byte & 0x02)   // V
      {
        unit++;
        if ((mode != 0) && (mode != 1) && (mode != 19) && (mode != 22))
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      if (byte & 0x04)
      {
        // A
        unit++;
        if ((mode < 2) && (mode > 7))
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      if (byte & 0x08)
      {
        // F
        unit++;
        if (mode != 9)
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      if (byte & 0x10) // M
        multiplier++;
      if (byte & 0x20) // k
        multiplier++;
      if (byte & 0x40)
      {
        // ohms
        unit++;
        if (mode != 8)
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      if (byte & 0x80)
      {
        // Hz
        unit++;
        if ((mode < 10) && (mode > 12))
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      if (unit > 1)
      {
        (void)fprintf(stderr, "too many units in 2nd byte %#x\n", byte);
        return false;
      }
      if (multiplier > 1)
      {
        (void)fprintf(stderr, "too many multipliers in 2nd byte %#x\n", byte);
        return false;
      }
      if (mode < 2)
      {
        if (!(byte & 0x02))
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      else if (mode < 8)
      {
        if (!(byte & 0x04))
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      else if (mode < 9)
      {
        if (!(byte & 0x40))
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      else if (mode < 10)
      {
        if (!(byte & 0x08))
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      else if (mode < 13)
      {
        if (!(byte & 0x80))
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
    }
    if (len >= 2)
    {
      byte = static_cast<unsigned char>(data[offset + 2]);
      if (byte & 0x01) // MIN
        minmax++;
      if (byte & 0x02) // REL
        minmax++;
      if (byte & 0x04)
      {
        // hFE
        unit++;
        if (mode != 21)
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      if (byte & 0x10)
      {
        // S
        unit++;
        if ((mode < 16) && (mode > 18))
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      if (byte & 0x20)
      {
        // dBm
        unit++;
        if (mode != 23)
        {
          (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
          return false;
        }
      }
      if (byte & 0x40) // n
        multiplier++;
      if (byte & 0x80) // micro
        multiplier++;
      if (unit > 1)
      {
        (void)fprintf(stderr, "too many units in 3rd byte %#x\n", byte);
        return false;
      }
      if (multiplier > 1)
      {
        (void)fprintf(stderr, "too many multipliers in 3rd byte %#x\n", byte);
        return false;
      }
      if (minmax > 1)
      {
        (void)fprintf(stderr, "too many min/max/rel indications in 3rd byte %#x\n", byte);
        return false;
      }
    }
    dp = 0;
    if (len >= 3)
    {
      byte = static_cast<unsigned char>(data[offset + 3]);
      if (byte & 0x08)
        dp++;
      if (len >= 4)
      {
        byte = static_cast<unsigned char>(data[offset + 4]);
        if (byte & 0x08)
          dp++;
        if (len >= 5)
        {
          byte = static_cast<unsigned char>(data[offset + 5]);
          if (byte & 0x08)
            dp++;
        }
      }
    }
    if (dp > 1)
      return false;
    if (len >= 6)
    {
      byte = static_cast<unsigned char>(data[offset + 6]);
      if (byte & 0x08) // MAX
        minmax++;
      if (minmax > 1)
      {
        (void)fprintf(stderr, "too many min/max/rel indications in 7th byte %#x\n", byte);
        return false;
      }
    }
    if (len >= 7)
    {
      rs232 = 0;
      byte = static_cast<unsigned char>(data[offset + 7]);
      if (byte & 0x02) // RS232
        rs232++;
      if (rs232 != 1)
      {
        (void)fprintf(stderr, "rs232 flag not set in 8th byte %#x\n", byte);
        return false;
      }
    }
    if (len >= offset + 8)
    {
      // XXX compute and validate checksum
      for (int i = 0; i < 8; ++i)
      {
        byte = static_cast<unsigned char>(data[offset + i] & 0x0ff);
        checksum += byte;
      }
      byte = static_cast<unsigned char>(data[offset + 8] & 0x0ff);
      if (((checksum + 57) & 0x0ff) == byte)
        return true;
      else
      {
        (void)fprintf(stderr, "bad checksum byte %#x should be %#x\n", byte, (checksum + 57) & 0x0ff);
        return false;
      }
    }
  }
  return false;
}


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
