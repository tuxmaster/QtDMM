#include "cyrustek_es51922.h"

static const bool registered = []() {
  DmmDecoder::addConfig(  {"Uni-Trend","UT61E","", 19200, 12, 7, 1, 1, 2, 22000, 0, 0, 1});
  DmmDecoder::addConfig(  {" Wintex","TD2200","", 19200, 12, 7, 1, 1, 0, 22000, 0, 0, 1});
  return true;
}();

bool DecoderCyrusTekES51922::checkFormat(const char* data, size_t idx)
{
  return (m_type == ReadEvent::CyrustekES51922 && (idx) && (data[idx - 1] == 0x0d) && (data[idx] == 0x0a));
}

size_t DecoderCyrusTekES51922::getPacketLength()
{
  return  (m_type == ReadEvent::CyrustekES51922 ? 14 : 0);
}

std::optional<DmmDecoder::DmmResponse> DecoderCyrusTekES51922::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id     = id;
  m_result.showBar = true;
  m_result.hold   = bit(data,11,1);
  m_result.range  = bit(data,10,1) ? "AUTO":"MANU";
  m_result.special= bit(data,10,2) ? "AC"  :"DC";
  m_result.val    = makeValue(data,1,5, bit(data,7,2));
  bool ul         = bit(data,9,3);
  bool ovl        = bit(data,7,0);
  bool hz         = bit(data,10,0);
  bool duty       = bit(data,7,3);
  int function    = data[6] & 0x0f;
  int range       = data[0] & 0x0f;

  switch (function)
  {
    case 0xB:
      switch (range)
      {
        case 0: formatResultValue(1,"","V"); break;
        case 1: formatResultValue(2,"","V"); break;
        case 2: formatResultValue(3,"","V"); break;
        case 3: formatResultValue(4,"","V"); break;
        case 4: formatResultValue(3,"m","V");break;
      }
      if(hz) m_result.unit = "Hz";
      if(duty) m_result.unit = "%";
      break;
    case 0x1:
      m_result.special = "DI";
      formatResultValue(3,"m","V"); break;
      break;
    case 0x2:
       m_result.special = "FR";
      switch (range)
      {
        case 0: formatResultValue(0,"","Hz"); break;
        case 1: formatResultValue(2,"k","Hz"); break;
        case 2: formatResultValue(3,"k","Hz"); break;
        case 3: formatResultValue(1,"M","Hz"); break;
        case 4: formatResultValue(2,"M","Hz"); break;
      }
     m_result.unit = duty ? "%" :  "Hz";
     break;
    case 0x3:
      m_result.special = "OH";
      switch (range)
      {
        case 0: formatResultValue(3,"","Ohm");  break;
        case 1: formatResultValue(1,"k","Ohm"); break;
        case 2: formatResultValue(2,"k","Ohm"); break;
        case 3: formatResultValue(3,"k","Ohm"); break;
        case 4: formatResultValue(1,"M","Ohm"); break;
        case 5: formatResultValue(2,"M","Ohm"); break;
        case 6: formatResultValue(3,"M","Ohm"); break;
      }      break;
    case 0x5:
      m_result.special = "BUZ";
      formatResultValue(3,"","Ohm");
      break;
    case 0x6:
      m_result.special = "CA";
      switch (range)
      {
        case 0: formatResultValue(2,"n","F"); break;
        case 1: formatResultValue(3,"n","F"); break;
        case 2: formatResultValue(1,"u","F"); break;
        case 3: formatResultValue(2,"u","F"); break;
        case 4: formatResultValue(3,"u","F"); break;
        case 5: formatResultValue(1,"m","F"); break;
        case 6: formatResultValue(2,"m","F"); break;
        case 7: formatResultValue(3,"m","F"); break;
      }
      break;
    case 0xD:
      switch (range)
      {
        case 0: formatResultValue(3,"u","A"); break;
        case 1: formatResultValue(4,"u","A"); break;
      }
      if(hz) m_result.unit = "Hz";
      if(duty) m_result.unit = "%";
      break;
    case 0xF:
      switch (range)
      {
        case 0: formatResultValue(2,"m","A"); break;
        case 1: formatResultValue(3,"m","A"); break;
      }
      if(hz) m_result.unit = "Hz";
      if(duty) m_result.unit = "%";
      break;
    case 0x9:
    case 0x0:
      formatResultValue(2,"","A");
      if(hz) m_result.unit = "Hz";
      if(duty) m_result.unit = "%";
      break;
  }

  if (ul) {
    m_result.val  = "UL";
    m_result.dval = 0;
  }
  if (ovl) {
    m_result.val  = "OL";
    m_result.dval = 0;
  }
  return m_result;
}
