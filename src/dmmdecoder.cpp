#include "dmmdecoder.h"

std::vector<DmmDecoder::DMMInfo> *DmmDecoder::m_configurations;

void DmmDecoder::addConfig(DMMInfo info)
{
    if(!DmmDecoder::m_configurations)
        DmmDecoder::m_configurations = new std::vector<DMMInfo>();
  info.name = info.vendor + " " + info.model;
  DmmDecoder::m_configurations->push_back(info);
}

std::vector<DmmDecoder::DMMInfo> DmmDecoder::getDeviceConfigurations()
{
  if(!DmmDecoder::m_configurations)
    DmmDecoder::m_configurations = new std::vector<DMMInfo>();
  return *m_configurations;
}


QString DmmDecoder::insertComma(const QString &val, int pos)
{
  return val.left(2 + pos) + "." + val.right(4 - pos);
}


QString DmmDecoder::insertCommaIT(const QString &val, int pos)
{
  if (pos == 0)
    return val;

  if (val[0] == '-' || val[0] == ' ')
    return (val.left(pos + 1) + "." + val.mid(pos + 1));

  return (val.left(pos) + "." + val.mid(pos));
}

bool DmmDecoder::bit(const QByteArray &data, int byte, int bit) const {
  return data[byte] & (1 << bit);
}


void DmmDecoder::formatResultValue(int commaPos, const QString& prefix, const QString& baseUnit)
{
    static const QHash<QString, double> scaleMap = {
    { "",     1.0 },
    { "k",    1e3 },
    { "M",    1e6 },
    { "G",    1e9 },
    { "m",    1e-3 },
    { "µ",    1e-6 },
    { "u",    1e-6 },  // optional: ASCII fallback
    { "n",    1e-9 }
  };

  m_result.val = insertCommaIT(m_result.val, commaPos);
  m_result.dval = m_result.val.toDouble() * scaleMap.value(prefix, 1.0);
  m_result.unit = prefix + baseUnit;
}


QString DmmDecoder::toString() const {
  return QString("id=%1: %2 %3 (%4) [%5] range=%6 hold=%7 bar=%8")
                   .arg(m_result.id)
                   .arg(m_result.val)
                   .arg(m_result.unit)
                   .arg(m_result.dval)
                   .arg(m_result.special)
                   .arg(m_result.range)
                   .arg(m_result.hold)
                   .arg(m_result.showBar);
}

/* old stuff from readerthresd as reference for the new stuff, will be deleted soon*/

/*
size_t DmmDriver::getPacketLength(ReadEvent::DataFormat df)
{
  switch (df)
  {
    case ReadEvent::Metex14:               return 14;
    case ReadEvent::Voltcraft14Continuous: return 14;
    case ReadEvent::Voltcraft15Continuous: return 15;
case ReadEvent::M9803RContinuous:      return 11;
    case ReadEvent::PeakTech10:            return 11;
    case ReadEvent::VC820Continuous:       return 14;
case ReadEvent::VC870Continuous:       return 23;
    case ReadEvent::IsoTech:               return 22;
case ReadEvent::VC940Continuous:       return 11;
    case ReadEvent::QM1537Continuous:      return 14;
    case ReadEvent::RS22812Continuous:     return 9;
    case ReadEvent::DO3122Continuous:      return 22;
    case ReadEvent::CyrustekES51922:       return 14;
    case ReadEvent::CyrustekES51962:       return 11;
    default:                               return 0;
  }
}
*/

  /*
  else if (m_format == ReadEvent::Metex14)
  {
    if (m_fifo[m_length] == 0x0d)
      return true;
  }
  else if (m_format == ReadEvent::Voltcraft14Continuous)
  {
    if (m_fifo[m_length] == 0x0d)
      return true;
  }
  else if (m_format == ReadEvent::Voltcraft15Continuous)
  {
    if (m_fifo[(m_length - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && m_fifo[m_length] == 0x0a)
      return true;
  }
  else if (m_format == ReadEvent::M9803RContinuous && m_length >= 10)
  {
    if (m_fifo[(m_length - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && m_fifo[m_length] == 0x0a)
      return true;
  }
  else if (m_format == ReadEvent::VC940Continuous && m_length >= 12)
  {
    if (m_fifo[(m_length - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && m_fifo[m_length] == 0x0a)
      return true;
  }
  else if (m_format == ReadEvent::PeakTech10 && m_length >= 11)
  {
    if (m_fifo[(m_length - 11 + FIFO_LENGTH) % FIFO_LENGTH] == '#')
      return true;
  }
  else if (m_format == ReadEvent::VC820Continuous)// && m_length >= 13)
  {qt_wrap_ui
    if ((m_fifo[m_length] & 0xf0) == 0xe0)
      return true;
  }
  else if (m_format == ReadEvent::VC870Continuous)
  {
    if ((m_length) && (m_fifo[m_length - 1] == 0x0d) && (m_fifo[m_length] == 0x0a))
      return true;
  }
  else if (m_format == ReadEvent::IsoTech && m_length >= 22)
  {
    for (int i = 0; i < 11; ++i)
    {
      if (m_fifo[m_length - 22 + i] != m_fifo[m_length - 22 + 11 + i])
        return false;
    }
    if (m_fifo[m_length - 22 + 9] != 0x0d ||  m_fifo[m_length - 22 + 10] != 0x0a)
      return false;
    return true;
  }
  else if (m_format == ReadEvent::QM1537Continuous)
  {
    if (m_fifo[m_length] == 0x0d)
      return true;
  }
  else if (m_format == ReadEvent::RS22812Continuous)
  {
    unsigned int checksum = 0x00;
    unsigned char byte;
    char mode, unit, multiplier, dp, minmax, rs232;
    int offset = 0;

    if (m_length > 9)
      offset = m_length - 9;
    // mode
    byte = static_cast<unsigned char>(m_fifo[offset]);
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
    if (m_length >= 1)
    {
      byte = static_cast<unsigned char>(m_fifo[offset + 1]);
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
    if (m_length >= 2)
    {
      byte = static_cast<unsigned char>(m_fifo[offset + 2]);
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
    if (m_length >= 3)
    {
      byte = static_cast<unsigned char>(m_fifo[offset + 3]);
      if (byte & 0x08)
        dp++;
      if (m_length >= 4)
      {
        byte = static_cast<unsigned char>(m_fifo[offset + 4]);
        if (byte & 0x08)
          dp++;
        if (m_length >= 5)
        {
          byte = static_cast<unsigned char>(m_fifo[offset + 5]);
          if (byte & 0x08)
            dp++;
        }
      }
    }
    if (dp > 1)
      return false;
    if (m_length >= 6)
    {
      byte = static_cast<unsigned char>(m_fifo[offset + 6]);
      if (byte & 0x08) // MAX
        minmax++;
      if (minmax > 1)
      {
        (void)fprintf(stderr, "too many min/max/rel indications in 7th byte %#x\n", byte);
        return false;
      }
    }
    if (m_length >= 7)
    {
      rs232 = 0;
      byte = static_cast<unsigned char>(m_fifo[offset + 7]);
      if (byte & 0x02) // RS232
        rs232++;
      if (rs232 != 1)
      {
        (void)fprintf(stderr, "rs232 flag not set in 8th byte %#x\n", byte);
        return false;
      }
    }
    if (m_length >= offset + 8)
    {
      // XXX compute and validate checksum
      for (int i = 0; i < 8; ++i)
      {
        byte = static_cast<unsigned char>(m_fifo[offset + i] & 0x0ff);
        checksum += byte;
      }
      byte = static_cast<unsigned char>(m_fifo[offset + 8] & 0x0ff);
      if (((checksum + 57) & 0x0ff) == byte)
        return true;
      else
      {
        (void)fprintf(stderr, "bad checksum byte %#x should be %#x\n", byte, (checksum + 57) & 0x0ff);
        return false;
      }
    }
  }
  else if (m_format == ReadEvent::DO3122Continuous)
  {
    if ((static_cast<uint8_t>(m_fifo[(m_length - 21 + FIFO_LENGTH) % FIFO_LENGTH]) != 0xAAu)
        || (static_cast<uint8_t>(m_fifo[(m_length - 20 + FIFO_LENGTH) % FIFO_LENGTH]) != 0x55u)
        || (static_cast<uint8_t>(m_fifo[(m_length - 19 + FIFO_LENGTH) % FIFO_LENGTH]) != 0x52u)
        || (static_cast<uint8_t>(m_fifo[(m_length - 18 + FIFO_LENGTH) % FIFO_LENGTH]) != 0x24u)
       )
      return false;

    if (static_cast<uint8_t>(m_fifo[(m_length - 17 + FIFO_LENGTH) % FIFO_LENGTH]) != 0x01u)
    {
      (void)fprintf(stderr, "bad mode %#x\n", static_cast<uint8_t>(m_fifo[(m_length - 17 + FIFO_LENGTH) % FIFO_LENGTH]));
      return false;
    }

    return true;
  }
  */
