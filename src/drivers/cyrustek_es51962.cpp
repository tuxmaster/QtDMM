#include "cyrustek_es51962.h"

// possibly same proto as iso-tech.

static const bool registered = []() {
  DmmDriver::addConfig(  {"Uni-Trend","U803","Uni-Trend UT803", 19200, 13, 7, 1, 1, 2, 6000, 0, 0,0,0,1});
  return true;
}();

bool DrvCyrusTekES51962::checkFormat(const char* data, size_t len, ReadEvent::DataFormat df)
{
  return (df == ReadEvent::CyrustekES51962 && len >= 12 && data[(len-1+FIFO_LENGTH)%FIFO_LENGTH] == 0x0d && data[len] == 0x0a);
}

size_t DrvCyrusTekES51962::getPacketLength(ReadEvent::DataFormat df)
{
  return  (df == ReadEvent::CyrustekES51962 ? 11 : 0);
}

std::optional<DmmDriver::DmmResponse> DrvCyrusTekES51962::decode(const QByteArray &data, int id, ReadEvent::DataFormat /*df*/)
{
  m_result = {};
  m_result.id     = id;
  m_result.showBar = true;
  m_result.hold   = bit(data,7,3);
  m_result.range  = bit(data,8,1) ? "AUTO":"MANU";
  m_result.special= bit(data,8,2) ? "AC"  :"DC";
  m_result.val    = bit(data,6,2) ? "-"   :"";
  bool ovl        = bit(data,6,0);
  QString unit_temp= bit(data,6,3) ? "C" : "F";
  int function = data[5] & 0x0f;
  int range    = data[0] & 0x0f;

  QString str(data);
  for (int i = 1; i < 5; ++i)
    m_result.val += str[i];

  switch (function)
  {
    case 0xB:
      switch (range)
      {
        case 0: formatResultValue(1,"","V"); break;
        case 1: formatResultValue(2,"","V"); break;
        case 2: formatResultValue(3,"","V"); break;
        case 3: formatResultValue(0,"","V"); break;
        case 4: formatResultValue(3,"m","V");break;
      }
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
      }
      break;
    case 0x4:
      m_result.special = "TE";
      m_result.showBar= false;
      formatResultValue(0,"",unit_temp); break;
      break;
    case 0x6:
      m_result.special = "CA";
      switch (range)
      {
        case 0: formatResultValue(1,"n","F"); break;
        case 1: formatResultValue(2,"n","F"); break;
        case 2: formatResultValue(3,"n","F"); break;
        case 3: formatResultValue(1,"u","F"); break;
        case 4: formatResultValue(2,"u","F"); break;
        case 5: formatResultValue(3,"u","F"); break;
        case 6: formatResultValue(1,"m","F"); break;
      }
      break;
    case 0xD:
      switch (range)
      {
        case 0: formatResultValue(3,"u","A"); break;
        case 1: formatResultValue(4,"u","A"); break;
      }
      break;
    case 0xF:
      switch (range)
      {
        case 0: formatResultValue(2,"m","A"); break;
        case 1: formatResultValue(3,"m","A"); break;
      }
      break;
    case 0x9:
      formatResultValue(2,"","A");
      break;
    case 0x5:   // buzzer
      m_result.special = "BUZ";
      formatResultValue(3,"","Ohm");
      break;
    case 0x1:
      m_result.special = "DI";
      formatResultValue(1,"","V");
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
      break;
    case 0xE:
      m_result.special = "HFE";
      formatResultValue(4,"m","A");
      break;
  }

  if (ovl) {
    m_result.val  = "OL";
    m_result.dval = 0;
  }
  return m_result;
}
