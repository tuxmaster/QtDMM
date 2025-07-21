#include "cyrustek_es51981.h"

// https://www.mikrocontroller.net/topic/98208

// possibly same proto as iso-tech.
static const bool registered = []()
{
  DmmDecoder::addConfig({"PeakTech", "3315", "",  2400, 15, 7, 1, 1, 0, 6000, 0, 0, 1});
  DmmDecoder::addConfig({"Uni-Trend", "UT70B", "",  2400, 15, 7, 1, 1, 0, 4000, 0, 0, 1});
  return true;
}();

bool DecoderCyrusTekES51981::checkFormat(const char *data, size_t len, ReadEvent::DataFormat df)
{
  return (df == ReadEvent::CyrustekES51981 && len >= 12 && data[(len - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && data[len] == 0x0a);
}

size_t DecoderCyrusTekES51981::getPacketLength(ReadEvent::DataFormat df)
{
  return (df == ReadEvent::CyrustekES51981 ? 11 : 0);
}

std::optional<DmmDecoder::DmmResponse> DecoderCyrusTekES51981::decode(const QByteArray &data, int id, ReadEvent::DataFormat /*df*/)
{
  m_result = {};
  m_result.id     = id;
  m_result.hold   = false;
  m_result.range  = "";
  m_result.showBar = true;

  QString val;
  QString special;
  QString unit;
  double scale = 1.0;

  // Digit values
  if (data[6] & 4) // Sign
    val = " -";
  else
    val = "  ";

  val += data[1];
  val += data[2];
  val += data[3];
  val += data[4];
  if (!memcmp(data + 1, "4000", 4) ||
      (data[6] & 1) != 0)
    val = "  0L";

  // Function
  switch (data[5])
  {
    case 0x31:
      unit = "D";
      break;

    case 0x3b:
      unit = "V";
      switch (data[0]) // Range
      {
        case '0':
          unit = "mV";
          scale = 1e-3;
          val = insertComma(val, 3);
          break;
        case '1':
          val = insertComma(val, 1);
          break;
        case '2':
          val = insertComma(val, 2);
          break;
        case '3':
          val = insertComma(val, 3);
          break;
      }
      break;
    case 0x3d:
      unit = "uA";
      scale = 1e-6;
      switch (data[0]) // Range
      {
        case '0':
          val = insertComma(val, 3);
          break;
      }
      break;
    case 0x39:
      unit = "mA";
      scale = 1e-3;
      switch (data[0]) // Range
      {
        case '0':
          val = insertComma(val, 3);
          break;
        case '1':
          val = insertComma(val, 1);
          break;
      }
      break;
    case 0x33:
      unit = "kOhm";
      switch (data[0]) // Range
      {
        case '0':
          unit = "Ohm";
          val = insertComma(val, 3);
          break;
        case '1':
          scale = 1e3;
          val = insertComma(val, 1);
          break;
        case '2':
          scale = 1e3;
          val = insertComma(val, 2);
          break;
        case '3':
          scale = 1e3;
          val = insertComma(val, 3);
          break;
        case '4':
          unit = "MOhm";
          scale = 1e6;
          val = insertComma(val, 1);
          break;
        case '5':
          scale = 1e6;
          unit = "MOhm";
          val = insertComma(val, 2);
          break;
      }
      break;
    case 0x34:
      unit = (data[6] & 8) ? "C" : "F";
      break;
    case 0x3f:
      unit = "A";
      val = insertComma(val, 2);
      break;
    case 0x32:
      switch (data[0]) // Range
      {
        case '0':
          unit = "kHz";
          scale = 1e3;
          val = insertComma(val, 1);
          break;
        case '1':
          unit = "kHz";
          scale = 1e3;
          val = insertComma(val, 2);
          break;
        case '2':
          unit = "kHz";
          scale = 1e3;
          val = insertComma(val, 3);
          break;
        case '3':
          unit = "MHz";
          scale = 1e6;
          val = insertComma(val, 1);
          break;
        case '4':
          unit = "MHz";
          scale = 1e6;
          val = insertComma(val, 2);
          break;
        case '5':
          unit = "MHz";
          scale = 1e6;
          val = insertComma(val, 3);
          break;
      }
      break;
    case 0x36:
      switch (data[0]) // Range
      {
        case '0':
          unit = "nF";
          scale = 1e-9;
          val = insertComma(val, 1);
          break;
        case '1':
          unit = "nF";
          scale = 1e-9;
          val = insertComma(val, 2);
          break;
        case '3':
          unit = "nF";
          scale = 1e-9;
          val = insertComma(val, 3);
          break;
        case '4':
          unit = "uF";
          scale = 1e-6;
          val = insertComma(val, 1);
          break;
        case '5':
          unit = "uF";
          scale = 1e-6;
          val = insertComma(val, 2);
          break;
        case '6':
          unit = "uF";
          scale = 1e-6;
          val = insertComma(val, 3);
          break;
        case '7':
          unit = "mF";
          scale = 1e-3;
          val = insertComma(val, 1);
          break;
      }
      break;


  }

  if (data[8] & 4)
    special += "AC";
  else if (data[8] & 8)
    special += "DC";

  m_result.dval   = val.toDouble() * scale;;
  m_result.special = special;
  m_result.val    = val;
  m_result.unit  = unit;

  return m_result;
}
