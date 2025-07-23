#include "rs22812.h"
#include <QHash>
/**
Bit
Byte    7       6       5       4       3       2       1       0
0       ---------------------- Mode -----------------------------
1       Hz      Ohms    K       M       F       A       V       m
2       u       n       dBm     s       %       hFE     REL     MIN
3       4D      4C      4G      4B      DP3     4E      4F      4A
4       3D      3C      3G      3B      DP2     3E      3F      3A
5       2D      2C      2G      2B      DP1     2E      2F      2A
6       1D      1C      1G      1B      MAX     1E      1F      1A
7       Beep    Diode   Bat     Hold    -       ~       RS232   Auto
8       -------------------- Checksum ---------------------------

The segment lettering is

    |--A--|
    |     |
    F     B
    |     |
    |--G--|
    |     |
    E     C
    |     |
    |--D--|

The mode is given by the following table:

    0    DC V
    1    AC V
    2    DC uA
    3    DC mA
    4    DC A
    5    AC uA
    6    AC mA
    7    AC A
    8    OHM
    9    CAP
    10   HZ
    11   NET HZ
    12   AMP HZ
    13   DUTY
    14   NET DUTY
    15   AMP DUTY
    16   WIDTH
    17   NET WIDTH
    18   AMP WIDTH
    19   DIODE
    20   CONT
    21   HFE
    22   LOGIC
    23   DBM
    24   EF
    25   TEMP

https://github.com/syn-net/rs22812/blob/master/rs22812_linux.py
 https://sigrok.org/wiki/RadioShack_22-812
*/

static const bool registered = []() {
  DmmDecoder::addConfig({"Radioshack", "22-812", "", 4800, 9, 8, 1, 1, 0, 4000, 0, 0, 1});
  return true;
}();


size_t DecoderRS22812::getPacketLength()
{
  return  (m_type == ReadEvent::RS22812Continuous ? 9 : 0);
}


bool DecoderRS22812::checkFormat(const char* data, size_t len)
{
  if (m_type == ReadEvent::RS22812Continuous)
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


std::optional<DmmDecoder::DmmResponse> DecoderRS22812::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id = id;
  m_result.range = bit(data,7,0) ? "AUTO":"MANU";
  m_result.hold = bit(data, 7, 4);
  m_result.showBar = true;
  m_result.special = bit(data, 7, 2) ? "AC" : "DC";

  // check for overflow
  if (((data[3] & 0x0f7) == 0x000) &&
      ((data[4] & 0x0f7) == 0x027) &&
      ((data[5] & 0x0f7) == 0x0d7))
  {
    m_result.val = "  0L  ";
  }
  else
  {
    m_result.val = bit(data, 7, 3) ? " -" : "  ";

    for (int i = 0; i < 4; ++i)
    {
      m_result.val +=  digit (data[6 - i]).value_or("?");
    }
  }

  // decimal marker
  if (data[3] & 0x08)      m_result.val = insertComma(m_result.val, 3);
  else if (data[4] & 0x08) m_result.val = insertComma(m_result.val, 2);
  else if (data[5] & 0x08) m_result.val = insertComma(m_result.val, 1);

  m_result.dval = m_result.val.toDouble();
  QString unit;

  // special mode
  if (bit(data, 7, 6)) m_result.special = "DI";
  else if (bit(data, 7, 7)) m_result.special = "BUZ";

  // base unit
  if (bit(data, 1, 3))      {unit = "F";   m_result.special = "CA"; }
  else if (bit(data, 1, 2)) {unit = "A"; }
  else if (bit(data, 1, 6)) {unit = "Ohm"; m_result.special = "OH"; }
  else if (bit(data, 1, 7)) {unit = "Hz";  m_result.special = "HZ"; }
  else if (bit(data, 1, 1)) { unit = "V"; }
  else if (bit(data, 2, 3)) {unit = "%";   m_result.special = "PC"; }
  else if (bit(data, 2, 5)) {unit = "dBm"; }
  else if (bit(data, 2, 4)) {unit = "s";  m_result.special = "HZ";}
  else if (bit(data, 2, 2)) {unit = "A";  m_result.special = "HFE"; }
  else { qDebug() << "Unknown unit!"; return std::nullopt;  }

  // prefix
  if (bit(data, 2, 6))      formatResultValue(0, "n", unit);
  else if (bit(data, 2, 7)) formatResultValue(0, "u", unit);
  else if (bit(data, 1, 0)) formatResultValue(0, "m", unit);
  else if (bit(data, 1, 5)) formatResultValue(0, "k", unit);
  else if (bit(data, 1, 4)) formatResultValue(0, "M", unit);

  return m_result;
}


std::optional<QString> DecoderRS22812::digit(uint8_t byte)
{
  static const QHash<uint8_t, QString> digitMap = {
    {0xd7, "0"}, {0x50, "1"}, {0xb5, "2"}, {0xf1, "3"},{0x72, "4"},
    {0xe3, "5"}, {0xe7, "6"}, {0x51, "7"}, {0xf7, "8"}, {0xf3, "9"},
    {0x27, "F"}, {0x37, "P"}, {0xA7, "E"}, {0x87, "C"}, {0x86, "L"},
    {0x76, "H"}, {0x06, "I"}, {0x66, "h"}, {0x24, "r"}, {0xA6, "t"},
    {0x64, "n"}, {0x20, "-"}, {0x00, " "}
  };

  byte &= 0xF7;
  if (auto it = digitMap.constFind(byte); it != digitMap.cend())
    return *it;
  return std::nullopt;
}
