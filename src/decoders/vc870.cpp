#include "vc870.h"

static const bool registered = []() {
  DmmDecoder::addConfig({"Voltcraft", "VC 870", "", 9600, 10, 8, 1, 2, 0, 40000, 0, 0, 1});
  return true;
}();

bool DecoderVC870::checkFormat(const char* data, size_t len)
{
  return (m_type == ReadEvent::VC870Continuous && (len) && (data[len - 1] == 0x0d) && (data[len] == 0x0a));
}

size_t DecoderVC870::getPacketLength()
{
  return  (m_type == ReadEvent::VC870Continuous ? 23 : 0);
}

std::optional<DmmDecoder::DmmResponse> DecoderVC870::decode(const QByteArray &data, int id)
{
  // Support for the Voltcraft VC870 digital multimeter was contributed by Florian Evers, florian-evers@gmx.de, under the "GPLv3 or later" licence
  m_result = {};
  m_result.id     = id;
  m_result.hold   = false;
  m_result.range  = "";

  QString val1;
  QString val2;
  QString special;
  QString unit1;
  QString unit2;
  const char *in = data.data();

  // Obtain units and range
  const uint8_t l_FunctionCode = in[0];
  const uint8_t l_FunctionSelectCode = in[1];
  const uint8_t l_FactorIndex = (in[2] & 0x0f); // Main Display Range

  double l_Factor  = 0.0;
  double l_Factor2 = 0.0;
  int    l_DotPos  = 0;
  int    l_DotPos2 = 0;
  bool   l_bParserError = false;
  bool   l_bShowBar = true;
  if (l_FunctionCode == 0x30)
  {
    // Measurement mode: DCV / ACV
    unit1 = "V";
    unit2 = "V";
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: DCV
      special = "DC";
      switch (l_FactorIndex)
      {
        case 0: // 4 V
          l_Factor = 1e-4;
          l_DotPos = 2;
          break;
        case 1:
          // 40 V
          l_Factor = 1e-3;
          l_DotPos = 3;
          break;
        case 2:
          // 400 V
          l_Factor = 1e-2;
          l_DotPos = 4;
          break;
        case 3:
          // 4000 V (but 1000 V is the maximum)
          l_Factor = 1e-1;
          l_DotPos = 5;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else if (l_FunctionSelectCode == 0x31)
    {
      // Measurement mode: ACV
      special = "AC";
      switch (l_FactorIndex)
      {
        case 0:
          // 4 V
          l_Factor = 1e-3;
          l_DotPos = 3;
          break;
        case 1:
          // 40 V
          l_Factor = 1e-2;
          l_DotPos = 4;
          break;
        case 2:
          // 400 V
          l_Factor = 1e-1;
          l_DotPos = 5;
          break;
        case 3:
          // 4000 V (but 1000 V is the maximum)
          l_Factor = 1;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else if (l_FunctionCode == 0x31)
  {
    // Measurement mode: DCmV / Celsius
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: DCmV
      special    = "DC";
      unit1      = "mV";
      unit2      = "mV";
      switch (l_FactorIndex)
      {
        case 0:
          // 400 mV
          l_Factor = 1e-5;
          l_DotPos = 4;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else if (l_FunctionSelectCode == 0x31)
    {
      // Measurement mode: degrees celsius and fahrenheit
      special    = "TE";
      unit1      = "C";
      unit2      = "F";
      l_bShowBar = false;
      switch (l_FactorIndex)
      {
        case 0:
          // 4000 degrees C
          l_Factor = 1e-1;
          l_DotPos = 5;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else if (l_FunctionCode == 0x32)
  {
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: Resistance OHM
      special = "OH";
      switch (l_FactorIndex)
      {
        case 0:
          // 400 Ohm
          l_Factor = 1e-2;
          l_DotPos = 4;
          unit1    = "Ohm";
          unit2    = "Ohm";
          break;
        case 1:
          // 4 kOhm
          l_Factor = 1e-1;
          l_DotPos = 2;
          unit1    = "kOhm";
          unit2    = "kOhm";
          break;
        case 2:
          // 40 kOhm
          l_Factor = 1e1;
          l_DotPos = 4;
          unit1    = "kOhm";
          unit2    = "kOhm";
          break;
        case 3:
          // 400 kOhm
          l_Factor = 1e2;
          l_DotPos = 5;
          unit1    = "kOhm";
          unit2    = "kOhm";
          break;
        case 4:
          // 4 MOhm
          l_Factor = 1e3;
          l_DotPos = 3;
          unit1    = "MOhm";
          unit2    = "MOhm";
          break;
        case 5:
          // 40 MOhm
          l_Factor = 1e4;
          l_DotPos = 4;
          unit1    = "MOhm";
          unit2    = "MOhm";
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else if (l_FunctionSelectCode == 0x31)
    {
      // Measurement mode: Short-circuit test CTN
      unit1   = "Ohm";
      unit2   = "Ohm";
      special = "OH";
      l_bShowBar = false;
      switch (l_FactorIndex)
      {
        case 0:
          // 400 Ohm
          l_Factor = 1e-2;
          l_DotPos = 4;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else if (l_FunctionCode == 0x33)
  {
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: Capacitance CAP
      special = "CA";
      l_bShowBar = false;
      switch (l_FactorIndex)
      {
        case 0:
          // 40 nF
          l_Factor = 1e-12;
          l_DotPos = 3;
          unit1    = "nF";
          unit2    = "nF";
          break;
        case 1:
          // 400 nF
          l_Factor = 1e-11;
          l_DotPos = 4;
          unit1    = "nF";
          unit2    = "nF";
          break;
        case 2:
          // 4 uF
          l_Factor = 1e-10;
          l_DotPos = 2;
          unit1    = "uF";
          unit2    = "uF";
          break;
        case 3:
          // 40 uF
          l_Factor = 1e-9;
          l_DotPos = 3;
          unit1    = "uF";
          unit2    = "uF";
          break;
        case 4:
          // 400 uF
          l_Factor = 1e-8;
          l_DotPos = 4;
          unit1    = "uF";
          unit2    = "uF";
          break;
        case 5:
          // 4000 uF
          l_Factor = 1e-7;
          l_DotPos = 5;
          unit1    = "uF";
          unit2    = "uF";
          break;
        case 6:
          // 40 mF
          l_Factor = 1e-6;
          l_DotPos = 3;
          unit1    = "mF";
          unit2    = "mF";
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else if (l_FunctionCode == 0x34)
  {
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: Diode DIO
      unit1   = "V";
      unit2   = "V";
      special = "DI";
      l_bShowBar = false;
      switch (l_FactorIndex)
      {
        case 0:
          // 4 V
          l_Factor = 1e-4;
          l_DotPos = 2;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else if (l_FunctionCode == 0x35)
  {
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: Frequency
      special = "FR";
      l_bShowBar = false;

      // The frequency mode was tested using a "Fluke and Philips" PM5193 programmable synthesizer / function generator (50 MHz)
      switch (l_FactorIndex)
      {
        case 0:
          // Range 40 Hz: 20 Hz is displayed as "20.000 Hz"
          l_Factor = 1e-3; // is Hz
          l_DotPos = 3;
          unit1    = "Hz";
          unit2    = "Hz";
          break;
        case 1:
          // Range 400 Hz: 200 Hz is displayed as "200.00 Hz"
          l_Factor = 1e-2; // is Hz
          l_DotPos = 4;
          unit1    = "Hz";
          unit2    = "Hz";
          break;
        case 2:
          // Range 4 kHz: 2 kHz is displayed as "2.0000 kHz"
          l_Factor = 1e-1; // is Hz
          l_DotPos = 2;
          unit1    = "kHz";
          unit2    = "kHz";
          break;
        case 3:
          // Range 40 kHz: 20 kHz is displayed as "20.000 kHz"
          l_Factor = 1; // is Hz
          l_DotPos = 3;
          unit1    = "kHz";
          unit2    = "kHz";
          break;
        case 4:
          // Range 400 kHz: 200 kHz is displayed as "200.00 kHz"
          l_Factor = 1e1; // is Hz
          l_DotPos = 4;
          unit1    = "kHz";
          unit2    = "kHz";
          break;
        case 5:
          // Range 4 Mhz: 2 MHz is displayed as "2.0000 MHz"
          l_Factor = 1e2; // is Hz
          l_DotPos = 2;
          unit1    = "MHz";
          unit2    = "MHz";
          break;
        case 6:
          // Range 40 Mhz: 20 MHz is displayed as "20.000 MHz"
          l_Factor = 1e3; // is Hz
          l_DotPos = 3;
          unit1    = "MHz";
          unit2    = "MHz";
          break;
        case 7:
          // Range 400 Mhz: 50 MHz is displayed as "050.00 MHz"
          l_Factor = 1e4; // is Hz
          l_DotPos = 4;
          unit1    = "MHz";
          unit2    = "MHz";
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else if (l_FunctionSelectCode == 0x31)
    {
      // Measurement mode: (4~20mA DC)%
      unit1      = "%";
      unit2      = "%";
      special    = "";
      switch (l_FactorIndex)
      {
        case 0:
          // 400 % (but 100 % is the maximum)
          l_Factor = 1e-2;
          l_DotPos = 4;
          l_bShowBar = false;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else if (l_FunctionCode == 0x36)
  {
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: DCuA
      unit1   = "uA";
      unit2   = "uA";
      special = "DC";
      switch (l_FactorIndex)
      {
        case 0:
          // 400 uA
          l_Factor = 1e-8;
          l_DotPos = 4;
          break;
        case 1:
          // 4000 uA
          l_Factor = 1e-7;
          l_DotPos = 5;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else if (l_FunctionSelectCode == 0x31)
    {
      // Measurement mode: ACuA
      unit1   = "uA";
      unit2   = "uA";
      special = "AC";
      switch (l_FactorIndex)
      {
        case 0:
          // 400 uA
          l_Factor = 1e-7;
          l_DotPos = 5;
          break;
        case 1:
          // 4000 uA
          l_Factor = 1e-6;
          l_DotPos = 0;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else if (l_FunctionCode == 0x37)
  {
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: DCmA
      unit1   = "mA";
      unit2   = "mA";
      special = "DC";
      switch (l_FactorIndex)
      {
        case 0:
          // 40 mA
          l_Factor = 1e-6;
          l_DotPos = 3;
          break;
        case 1:
          // 400 mA
          l_Factor = 1e-5;
          l_DotPos = 4;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else if (l_FunctionSelectCode == 0x31)
    {
      // Measurement mode: ACmA
      unit1   = "mA";
      unit2   = "mA";
      special = "AC";
      switch (l_FactorIndex)
      {
        case 0:
          // 40 mA
          l_Factor = 1e-5;
          l_DotPos = 4;
          break;
        case 1:
          // 400 mA
          l_Factor = 1e-4;
          l_DotPos = 5;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else if (l_FunctionCode == 0x38)
  {
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: DCA
      unit1   = "A";
      unit2   = "A";
      special = "DC";
      switch (l_FactorIndex)
      {
        case 0:
          // 40 A (but 10 A is the maximum)
          l_Factor = 1e-3;
          l_DotPos = 3;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else if (l_FunctionSelectCode == 0x31)
    {
      // Measurement mode: ACA
      unit1   = "A";
      unit2   = "A";
      special = "AC";
      switch (l_FactorIndex)
      {
        case 0:
          // 400 A (but 10 A is the maximum)
          l_Factor = 1e-2;
          l_DotPos = 4;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else if (l_FunctionCode == 0x39)
  {
    if (l_FunctionSelectCode == 0x30)
    {
      // Measurement mode: Active power + Apparent power
      unit1   = "W";
      unit2   = "VA";
      special = "AC";
      switch (l_FactorIndex)
      {
        case 0:
          // 4000 W / 4000 VA
          l_Factor = 1e-1;
          l_DotPos = 5;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else if (l_FunctionSelectCode == 0x31)
    {
      // Measurement mode: Power factor + Frequency
      unit1   = "cosphi";
      unit2   = "Hz";
      special = "AC";
      switch (l_FactorIndex)
      {
        case 0:
          // 40 PF
          l_Factor  = 1e-3;
          l_DotPos  = 3;

          // 4000 Hz
          l_Factor2 = 1e-1;
          l_DotPos2 = 5;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else if (l_FunctionSelectCode == 0x32)
    {
      // Measurement mode: Voltage effective value + current effective value
      unit1   = "V";
      unit2   = "A";
      special = "AC";
      switch (l_FactorIndex)
      {
        case 0:
          // 4000 V / 4000 A
          l_Factor = 1e-3;
          l_DotPos = 5;
          break;
        default:
          l_bParserError = true;
          break;
      } // switch
    }
    else
    {
      // Unknown function select code
      l_bParserError = true;
    } // else
  }
  else
  {
    // Unknown function code
    l_bParserError = true;
  } // else

  // Obtain value of the primary display
  double d_val1;
  if ((in[15] & 0x01) || (in[19] & 0x01))   // vc870_ol1 flag or open_flag
  {
    val1   = "0L";
    d_val1 = 0;
  }
  else if (in[19] & 0x04)     // lo_flag
  {
    val1   = "L 0";
    d_val1 = 0;
  }
  else if (in[19] & 0x02)     // hi_flag
  {
    val1   = "H 1";
    d_val1 = 0;
  }
  else
  {
    if (in[15] & 0x04)   // Sign1_flag
      val1 = " -";
    else
      val1 = "  ";
    // else

    val1 += makeValue(data,3,7);
    d_val1 = (l_Factor * val1.toDouble());
    if (l_DotPos)
    {
      val1 = insertCommaIT(val1, l_DotPos); // 2 ... 5
    } // if
  } // else

  // Obtain value of the secondary display
  double d_val2;
  if ((in[17] & 0x08) || (in[19] & 0x01))   // vc870_ol2 flag or open_flag
  {
    val2   = "0L";
    d_val2 = 0;
  }
  else if (in[19] & 0x04)     // lo_flag
  {
    val2   = "L 0";
    d_val2 = 0;
  }
  else if (in[19] & 0x02)     // hi_flag
  {
    val2   = "H 1";
    d_val2 = 0;
  }
  else
  {
    if (in[15] & 0x08)   // Sign2_flag
      val2 = " -";
    else
      val2 = "  ";
    // else

    val2 += makeValue(data,8,12);
    if (l_Factor2 == 0.0)
      d_val2 = (l_Factor * val2.toDouble());
    else
      d_val2 = (l_Factor2 * val2.toDouble());
    // else

    if (!l_DotPos2)
    {
      val2 = insertCommaIT(val2, l_DotPos);  // 2 ... 5
    }
    else
    {
      val2 = insertCommaIT(val2, l_DotPos2); // 2 ... 5
    } // else
  } // else

  if (!l_bParserError)
  {
    // Always update the primary display (ID 0)
    m_result.special= special;
    m_result.showBar= l_bShowBar;
    m_result.dval   = d_val1;
    m_result.val    = val1;
    m_result.unit   = unit1;
    m_result.id     = 0;
    if (in[20] == 0x31)   // LCD need Dual display
    {
      m_result.dval2  = d_val2;
      m_result.val2   = val2;
      m_result.unit2  = unit2;
      m_result.id2    = 1;
    }
    else
    {
      // Disable the secondary display (ID 1) as it is currently unused
      m_result.dval2  = 0;
      m_result.val2   = "";
      m_result.unit2  = "";
      m_result.id2    = 1;
    } // else
  }
  else
  {
    // Parser error
    m_result.error = tr("Parser errors");
  } // else

  return m_result;
}

