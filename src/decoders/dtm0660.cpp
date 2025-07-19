#include "dtm0660.h"

/*
 * https://www.eevblog.com/forum/testgear/hacking-the-victor-vc-921/
 * https://github.com/pingumacpenguin/QtDMM-DTM0660-Version
 */

static const bool registered = []() {
  DmmDecoder::addConfig({"Generic", "DTM0660 4000 count", "", 2400, 13, 8, 1, 1, 0, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"Generic", "DTM0660 6000 count", "", 2400, 13, 8, 1, 1, 0, 6000, 0, 0, 1});
  DmmDecoder::addConfig({"Generic", "DTM0660 8000 count", "", 2400, 13, 8, 1, 1, 0, 8000, 0, 0, 1});
  return true;
}();

size_t DecoderDTM0660::getPacketLength(ReadEvent::DataFormat df)
{
  return  (df == ReadEvent::DTM0660 ? 15 : 0);
}

bool DecoderDTM0660::checkFormat(const char* data, size_t len, ReadEvent::DataFormat df)
{
  return (df == ReadEvent::DTM0660 && ((data[len] & 0xf0) == 0xf0));
}

std::optional<DmmDecoder::DmmResponse> DecoderDTM0660::decode(const QByteArray &data, int id, ReadEvent::DataFormat /*df*/)
{
  m_result = {};
  m_result.id     = id;
  m_result.hold   = false;
  m_result.range  = "";
  m_result.showBar = true;

  const char *in = data.data();

  // check for overload else find sign and fill in digits
  if (((in[3] & 0x0e) == 0x0e) &&
      ((in[4] & 0x0f) == 0x0b) &&
      ((in[5] & 0x0e) == 0x06) &&
      ((in[6] & 0x0f) == 0x01))
  {
    m_result.val = " 0L ";
  }
  else
  {
    m_result.val = (in[1] & 0x01) ? " -" : "  ";

    // create string;
    for (int i=0; i<4; ++i)
    {
      m_result.val += digit( ((in[1+2*i] & 0x0f) <<4) | (in[2+2*i] & 0x0f));
    }
  }

  // find decimal point position
  if (in[3] & 0x01)
  {
    m_result.val = insertComma( m_result.val, 1 );
  }
  else if (in[5] & 0x01)
  {
    m_result.val = insertComma( m_result.val, 2 );
  }
  else if(in[7] & 0x01)
  {
    m_result.val = insertComma( m_result.val, 3 );
  }

  m_result.dval = m_result.val.toDouble();

  // try to find some special modes
  if (in[9] & 0x08)
  {
    m_result.special = "DI";
  }
  if (in[0] & 0x01)
  {
    m_result.special = "AC";
  }
  if (in[0] & 0x02)
  {
    m_result.special = "DC";
  }

  // try to find mode
  if (in[11] & 0x01)
  {
    m_result.unit    = "F";
    m_result.special = "CA";
  }
  else if (in[11] & 0x02)
  {
    m_result.unit    = "Ohm";
    m_result.special = "OH";
  }
  else if (in[12] & 0x08)
  {
    m_result.unit = "A";
  }
  else if (in[12] & 0x04)
  {
    m_result.unit    = "Hz";
    m_result.special = "HZ";
  }
  else if (in[12] & 0x02)
  {
    m_result.unit = "V";
  }
  else if (in[10] & 0x02)
  {
    m_result.unit    = "%";
    m_result.special = "PC";
  }

  else if (in[13] & 0x02)
  {
    m_result.unit    = "C";
    m_result.special = "TE";
  }
  else if (in[13] & 0x01)
  {
    m_result.unit    = "F";
    m_result.special = "TE";
  }
  else if (in[13] & 0x01)
  {
    m_result.unit    = "C";
    m_result.special = "TE";
  }
  else
  {
    qWarning() << "Unknown DTM0660 unit!";
  }

  // try to find prefix
  //
  if (in[9] & 0x02)
  {
    m_result.dval /= 1e9;
    m_result.unit.prepend( "n" );
  }
  else if (in[9] & 0x01)
  {
    m_result.dval /= 1e6;
    m_result.unit.prepend( "u" );
  }
  else if (in[10] & 0x01)
  {
    m_result.dval /= 1e3;
    m_result.unit.prepend( "m" );
  }
  else if (in[9] & 0x04)
  {
    m_result.dval *= 1e3;
    m_result.unit.prepend( "k" );
  }
  else if (in[10] & 0x04)
  {
    m_result.dval *= 1e6;
    m_result.unit.prepend( "M" );
  }

  return m_result;
}

const char *DecoderDTM0660::digit( int byte )
{
  int           digit[10] = { 0xeb, 0x0a, 0xad, 0x8f, 0x4e, 0xc7, 0xe7, 0x8a, 0xef, 0xcf };
  const char *c_digit[10] = { "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9" };

  byte &= 0xef;

  for (int n=0; n<10; n++)
  {
    if (byte == digit[n]) return c_digit[n];
  }
  qWarning() << "Unknown DTM0660 byte " << byte;
  return 0;
}
