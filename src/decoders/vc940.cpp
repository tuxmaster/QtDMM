#include "vc940.h"
// ES51966

static const bool registered = []() {
  DmmDecoder::addConfig({"Tenma", "72-7732", "", 2400, 7, 7, 1, 1, 2, 40000, 0, 0, 1});
  DmmDecoder::addConfig({"Uni-Trend", "UT71B", "", 2400, 7, 7, 1, 1, 2, 200000, 0, 0, 1});
  DmmDecoder::addConfig({"Uni-Trend", "UT71CDE", "", 2400, 7, 7, 1, 1, 2, 40000, 0, 0, 1});
  DmmDecoder::addConfig({"Uni-Trend", "UT804", "", 2400, 7, 7, 1, 1, 2, 40000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 920", "", 2400, 7, 7, 1, 1, 2, 40000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 940", "", 2400, 7, 7, 1, 1, 2, 40000, 0, 0, 1});
  DmmDecoder::addConfig({"Voltcraft", "VC 960", "", 2400, 7, 7, 1, 1, 2, 40000, 0, 0, 1});
  return true;
}();

size_t DecoderVC940::getPacketLength()
{
  return  (m_type == ReadEvent::VC940Continuous ? 11 : 0);
}

bool DecoderVC940::checkFormat(const char* data, size_t idx)
{
  return (m_type==ReadEvent::VC940Continuous && idx >= 12 && data[(idx - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && data[idx] == 0x0a);
}

std::optional<DmmDecoder::DmmResponse> DecoderVC940::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id     = id;
  m_result.showBar = true;
  m_result.hold   = false;
  m_result.range  = bit(data,8,0) ? "AUTO":"MANU";
  m_result.special= bit(data,7,0) ? "AC"  :"DC";
  m_result.special+= bit(data,7,1) ? "DC" : "";

  int function = data[6] & 0x0f;
  int range    = data[5] & 0x0f;
  int mode2    = data[8];
  bool neg     = bit(data,8,3);

  m_result.val = makeValue(data,0,3, (function != 0xC && neg));
  if (data[4] != 'A')
    m_result.val += data[4];

  switch (function)
  {
    case 0x0:formatResultValue(3,"m","V"); break;
    case 0x1:formatResultValue(range,"","V");  m_result.special = "DC"; break;
    case 0x2:formatResultValue(range,"","V");  m_result.special = "AC"; break;
    case 0x3:formatResultValue(range,"m","V"); m_result.special = "DC"; break;
    case 0x4:
      m_result.special = "OH";
      switch (range)
      {
        case 1:formatResultValue(3,"","Ohm");  break;
        case 2:formatResultValue(1,"k","Ohm"); break;
        case 3:formatResultValue(2,"k","Ohm"); break;
        case 4:formatResultValue(3,"k","Ohm"); break;
        case 5:formatResultValue(1,"M","Ohm"); break;
        case 6:formatResultValue(2,"M","Ohm"); break;
      }
      break;
    case 0x5:
      m_result.special = "CA";
      switch (range)
      {
        case 1:formatResultValue(2,"n","F"); break;
        case 2:formatResultValue(3,"n","F"); break;
        case 3:formatResultValue(1,"u","F"); break;
        case 4:formatResultValue(2,"u","F"); break;
        case 5:formatResultValue(3,"u","F"); break;
        case 6:formatResultValue(4,"u","F"); break;
        case 7:formatResultValue(1,"m","F"); break;
      }
      break;
    case 0x6:formatResultValue(4,"","C");  m_result.special = "TE"; break;
    case 0x7:
      switch (range)
      {
        case 0:formatResultValue(3,"u","A"); break;
        case 1:formatResultValue(4,"u","A"); break;
      }
      break;
    case 0x8:
      switch (range)
      {
        case 0:formatResultValue(2,"m","A"); break;
        case 1:formatResultValue(3,"m","A"); break;
      }
      break;
    case 0x9: formatResultValue(2,"","A"); break;
    case 0xA:
      m_result.special = "BUZ";
      formatResultValue(3,"","Ohm");
      break;
    case 0xB:
      m_result.special = "DI";
      formatResultValue(1,"","V");
      break;
    case 0xC:
      if (neg)
      {
        formatResultValue(3,"","%");
        m_result.special = "PC";
      }
      else
      {
        m_result.special = "FR";
        switch (range)
        {
          case 0:formatResultValue(2,"","Hz"); break;
          case 1:formatResultValue(3,"","Hz"); break;
          case 2:formatResultValue(1,"k","Hz"); break;
          case 3:formatResultValue(2,"k","Hz"); break;
          case 4:formatResultValue(3,"k","Hz"); break;
          case 5:formatResultValue(1,"M","Hz"); break;
          case 6:formatResultValue(2,"k","Hz"); break;
          case 7:formatResultValue(3,"k","Hz"); break;
        }
      }
      break;
    case 0xD:
      m_result.special = "TE";
      formatResultValue(4,"","dF"); break;
      break;
  }


  return m_result;
}
