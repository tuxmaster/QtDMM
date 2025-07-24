#include "m9803r.h"

/*
 Reverse engeneered by Matthias Toussaint

    Port settings seem to be 9600 7E1 or 9600 7O1 (looks like a bug)
    Protocoll is 11 bytes binary fixed length
    END OF PACKET is 0x0d 0x0a
    Multimeter continuously sends data

Byte 0: Sign ored together
0x00 -> positive
0x08 -> negative
0x01 -> overflow

Byte 1: Digit 4 binary
Byte 2: Digit 3 binary
Byte 3: Digit 2 binary
Byte 4: Digit 1 binary

Byte 5: Mode
0x00 DCV
0x01 ACV
0x02 DCA
0x03 ACA
0x04 Ohms
0x05 Ohms + Beep
0x06 Diode V
0x07 ADP
0x08 DCA (10A)
0x09 ACA (10A)
0x0A Freq
0x0C Capacity

Byte 6:Decimal point position
Display	Unit	Value
Frequency
0.000	kHz	0x00
00.00	kHz	0x01
00.00	Hz	0x05
000.0	Hz	0x06
Voltage
000.0	mV	0x00
0.000	V	0x01
00.00	V	0x02
000.0	V	0x03
0000.	V	0x04
Current
0.000	mA	0x00
00.00	mA	0x01
000.0	mA	0x02
Capacity
0.000	nF	0x00
00.00	nF	0x01
000.0	nF	0x02
0.000	uF	0x03
00.00	uF	0x04
Resistance
000.0	Ohm	0x00
0.000	kOhm	0x01
00.00	kOhm	0x02
000.0	kOhm	0x03
0000.	kOhm	0x04
00.00	MOhm	0x05

Byte 7: Hold/min/Max/Rel ored together
0x01 Hold
0x02 Rel
0x04 Min
0x08 Max

Byte 8: Auto/Manu
0x01 APOF (AutoPowerOff)
0x02 Manu
0x04 Auto
0x08 MEM
*/

static const bool registered = []() {
  DmmDecoder::addConfig({"ELV", "M9803R", "", 9600, 4, 7, 1, 1, 1, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"MASTECH", "M9803R", "", 9600, 4, 7, 1, 1, 1, 4000, 0, 0, 1});
  DmmDecoder::addConfig({"McVoice", "M-980T", "", 9600, 4, 7, 1, 1, 0, 4000, 0, 0, 1});
  return true;
}();

size_t DecoderM9803R::getPacketLength()
{
  return  (m_type == ReadEvent::M9803RContinuous ? 11 : 0);
}


bool DecoderM9803R::checkFormat(const char* data, size_t len)
{
  return (m_type==ReadEvent::M9803RContinuous && len >= 10 && data[(len - 1 + FIFO_LENGTH) % FIFO_LENGTH] == 0x0d && data[len] == 0x0a);
}


std::optional<DmmDecoder::DmmResponse> DecoderM9803R::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id     = id;
  m_result.hold   = false;
  m_result.range  = bit(data,8,2) ? "AUTO" : "MANU";
  m_result.showBar = true;

  QString val;
  QString special;
  QString unit;

  if (data[0] & 0x01)
    val = "  0L";
  else
  {
    val = data[0] == 0x08 ? " -" : "  ";
    val += QChar('0' + data[4]);
    val += QChar('0' + data[3]);
    val += QChar('0' + data[2]);
    val += QChar('0' + data[1]);
  }

  double d_val = val.toDouble();


  switch (data[5])
  {
    case 0x00:
      switch (data[6])
      {
        case 0x00:
          unit = "mV";
          val = insertComma(val, 3);
          d_val /= 10000.;
          break;
        case 0x01:
          unit = "V";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x02:
          unit = "V";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x03:
          unit = "V";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
        case 0x04:
          unit = "V";
          break;
      }
      special = "DC";
      break;
    case 0x01:
      switch (data[6])
      {
        case 0x00:
          unit = "mV";
          val = insertComma(val, 3);
          d_val /= 10000.;
          break;
        case 0x01:
          unit = "V";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x02:
          unit = "V";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x03:
          unit = "V";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
        case 0x04:
          unit = "V";
          break;
      }
      special = "AC";
      break;
    case 0x02:
      switch (data[6])
      {
        case 0x00:
          unit = "mA";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x01:
          unit = "mA";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x02:
          unit = "mA";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
      }
      special = "DC";
      break;
    case 0x03:
      switch (data[6])
      {
        case 0x00:
          unit = "mA";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x01:
          unit = "mA";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x02:
          unit = "mA";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
      }
      special = "AC";
      break;
    case 0x04:
      switch (data[6])
      {
        case 0x00:
          unit = "Ohm";
          val = insertComma(val, 1);
          d_val /= 10.;
          break;
        case 0x01:
          unit = "kOhm";
          val = insertComma(val, 1);
          d_val /= 1000.;
          break;
        case 0x02:
          unit = "kOhm";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x03:
          unit = "kOhm";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
        case 0x04:
          unit = "kOhm";
          break;
        case 0x05:
          unit = "MOhm";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
      }
      special = "OH";
      break;
    case 0x05:
      switch (data[6])
      {
        case 0x00:
          unit = "Ohm";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
      }
      special = "OH";
      break;
    case 0x06:
      unit = "V";
      val = insertComma(val, 1);
      d_val /= 1000.;
      special = "DI";
      break;
    case 0x08:
      unit = "A";
      val = insertComma(val, 2);
      d_val /= 100.;
      special = "DC";
      break;
    case 0x09:
      unit = "A";
      val = insertComma(val, 2);
      d_val /= 100.;
      special = "AC";
      break;
    case 0x0A:
      showBar = false;
      switch (data[6])
      {
        case 0x00:
          unit = "kHz";
          val = insertComma(val, 1);
          //d_val /= 1000.;
          break;
        case 0x01:
          unit = "kHz";
          val = insertComma(val, 2);
          d_val *= 10.;
          break;
        case 0x05:
          unit = "Hz";
          val = insertComma(val, 2);
          d_val /= 100.;
          break;
        case 0x06:
          unit = "Hz";
          val = insertComma(val, 3);
          d_val /= 10.;
          break;
      }
      special = "HZ";
      break;
    case 0x0C:
      switch (data[6])
      {
        case 0x00:
          unit = "nF";
          val = insertComma(val, 1);
          d_val /= 1e12;
          break;
        case 0x01:
          unit = "nF";
          val = insertComma(val, 2);
          d_val /= 1e11;
          break;
        case 0x02:
          unit = "nF";
          val = insertComma(val, 3);
          d_val /= 1e10;
          break;
        case 0x03:
          unit = "uF";
          val = insertComma(val, 1);
          d_val /= 1e9;
          break;
        case 0x04:
          unit = "uF";
          val = insertComma(val, 2);
          d_val /= 1e8;
          break;
      }
      special = "CA";
      break;
  }

  m_result.dval   = d_val;
  m_result.special= special;
  m_result.val    = val.trimmed();
  m_result.unit  = unit;

  return m_result;
}
