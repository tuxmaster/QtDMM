#include "cyrustek_es51981.h"

// https://www.mikrocontroller.net/topic/98208

// possibly same proto as iso-tech.
static const bool registered = []()
{
  DmmDecoder::addConfig({"PeakTech", "3315", "",  2400, 15, 7, 1, 1, 0, 6000, 0, 0, 1});
  DmmDecoder::addConfig({"Uni-Trend", "UT70B", "",  2400, 15, 7, 1, 1, 0, 4000, 0, 0, 1});
  return true;
}();

bool DecoderCyrusTekES51981::checkFormat(const char *data, size_t len)
{
  return (m_type == ReadEvent::CyrustekES51981 && len >= 12 && data[(len - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && data[len] == 0x0a);
}

size_t DecoderCyrusTekES51981::getPacketLength()
{
  return (m_type == ReadEvent::CyrustekES51981 ? 11 : 0);
}

std::optional<DmmDecoder::DmmResponse> DecoderCyrusTekES51981::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id     = id;
  m_result.hold   = false;
  m_result.range  = bit(data,8,1) ? "AUTO" : "MANU";
  m_result.val    = makeValue(data,1,4,bit(data,6,2));
  m_result.showBar = true;

  if (data[8] & 4)
    m_result.special = "AC";
  else if (data[8] & 8)
    m_result.special = "DC";

  if (!memcmp(data + 1, "4000", 4) || (data[6] & 1) != 0)
    m_result.val = "  0L";

  // Function
  switch (data[5])
  {
    case 0x31:
      m_result.unit = "D";
      break;
    case 0x32:

      switch (data[0]) // Range
      {
        case '0': formatResultValue(1,"k",bit(data,6,3)?"RPM":"Hz"); break;
        case '1': formatResultValue(2,"k",bit(data,6,3)?"RPM":"Hz"); break;
        case '2': formatResultValue(3,"k",bit(data,6,3)?"RPM":"Hz"); break;
        case '3': formatResultValue(1,"M",bit(data,6,3)?"RPM":"Hz"); break;
        case '4': formatResultValue(2,"M",bit(data,6,3)?"RPM":"Hz"); break;
        case '5': formatResultValue(3,"M",bit(data,6,3)?"RPM":"Hz"); break;
      }
      break;
    case 0x33:
      m_result.unit = "kOhm";
      switch (data[0]) // Range
      {
        case '0': formatResultValue(3,"","Ohm"); break;
        case '1': formatResultValue(1,"k","Ohm"); break;
        case '2': formatResultValue(2,"k","Ohm"); break;
        case '3': formatResultValue(3,"k","Ohm"); break;
        case '4': formatResultValue(1,"M","Ohm"); break;
        case '5': formatResultValue(2,"M","Ohm"); break;
      }
      break;
    case 0x34:
      m_result.special = "TE";
      formatResultValue(0,"",(data[6] & 8) ? "C" : "dF"); break;
      break;
    case 0x35:
      m_result.special = "BUZ";
      formatResultValue(3,"","Ohm");
      break;
    case 0x36:
      switch (data[0]) // Range
      {
        case '0': formatResultValue(1,"n","F"); break;
        case '1': formatResultValue(2,"n","F"); break;
        case '3': formatResultValue(3,"n","F"); break;
        case '4': formatResultValue(1,"u","F"); break;
        case '5': formatResultValue(2,"u","F"); break;
        case '6': formatResultValue(3,"u","F"); break;
        case '7': formatResultValue(1,"m","F"); break;
      }
      break;
    case 0x39:
      switch (data[0]) // Range
      {
        case '0': formatResultValue(3,"m","A"); break;
        case '1': formatResultValue(1,"m","A"); break;
      }
      break;
    case 0x3b:
      m_result.unit = "V";
      switch (data[0]) // Range
      {
        case '0': formatResultValue(3,"m","V"); break;
        case '1': formatResultValue(1,"","V"); break;
        case '2': formatResultValue(2,"","V"); break;
        case '3': formatResultValue(3,"","V"); break;
      }
      break;
    case 0x3d:
      switch (data[0]) // Range
      {
        case '0':  formatResultValue(3,"u","A"); break;
      }
      break;
    case 0x3f:
      formatResultValue(2,"A","Hz");
      break;
  }

  return m_result;
}
