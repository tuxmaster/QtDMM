// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "unit_idmm.h"

#include <QRegularExpression>

static const bool registered = []() {
  // Bluetooth LE: no baud rate; the address is chosen on the Multimeter page
  DmmDecoder::addConfig({"Uni-Trend", "UT60BT", "", 0, ReadEvent::UniTiDMM, 8, 1, 1, 0, 10000, 0, 0, 0});
  // UT161 series: Bluetooth built in like the UT60BT, ranges of the UT61+
  // (vendor tables funOl_UT161B/D/E); not tried with QtDMM
  DmmDecoder::addConfig({"Uni-Trend", "UT161B *", "", 0, ReadEvent::UniTUT61Plus, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT161D *", "", 0, ReadEvent::UniTUT61Plus, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT161E *", "", 0, ReadEvent::UniTUT61Plus, 8, 1, 1, 0, 22000, 0, 0, 0});
  // UT61B+/D+/E+ (per ut61xpy): the USB cable UT-D09 is a USB-HID UART (both
  // revisions, CP2110 and CH9329) at 9600 8N1; the UT-D07B Bluetooth adapter
  // carries the same frames over the same GATT service as the UT60BT
  DmmDecoder::addConfig({"Uni-Trend", "UT61E+ *", "", 9600, ReadEvent::UniTUT61Plus, 8, 1, 1, 0, 22000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT61D+ *", "", 9600, ReadEvent::UniTUT61Plus, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT61B+ *", "", 9600, ReadEvent::UniTUT61Plus, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT61E+ (UT-D07B Bluetooth) *", "", 0, ReadEvent::UniTUT61Plus, 8, 1, 1, 0, 22000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT61D+ (UT-D07B Bluetooth) *", "", 0, ReadEvent::UniTUT61Plus, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT61B+ (UT-D07B Bluetooth) *", "", 0, ReadEvent::UniTUT61Plus, 8, 1, 1, 0, 6000, 0, 0, 0});
  return true;
}();

namespace
{
// fn (byte 3, bit 7 masked) -> special, base unit and the SI prefix per range
// digit (byte 4 - '0'). One prefix string means the unit never changes.
// Function codes and ranges as UNI-T's own app has them (funOl_UT60BT.json and
// TestDataModel.functionStrings, saved by the unit_ut61eplus project - see
// ablage/unit_ut61eplus/from_vendor): V positions start in a 999.9 mV range,
// two mV ranges, OHM kOhm from range 1 and MOhm from range 4, CAP nF up to
// range 2 (999.9 nF), Hz up to range 2 (999.90 Hz), the mA position's range
// 1 is amperes. Checked on a UT60BT dial by dial (2026-09-24) for V, mV,
// OHM, the lowest CAP and Hz ranges. Codes 22-30 belong to other models.
struct Function
{
  const char *name;       ///< for the spec/debug output
  const char *special;    ///< "" = AC/DC from flags C bit 3
  const char *unit;
  const char *prefixes[8];
};

const Function kFunctions[] = {
  /*  0 */ { "ACV",    "",    "V",   { "m", "", "", "" } },
  /*  1 */ { "ACmV",   "",    "V",   { "m" } },
  /*  2 */ { "DCV",    "",    "V",   { "m", "", "", "" } },
  /*  3 */ { "DCmV",   "",    "V",   { "m" } },
  /*  4 */ { "Hz",     "HZ",  "Hz",  { "", "", "", "k", "k", "k", "M", "M" } },
  /*  5 */ { "%",      "DU",  "%",   { "" } },
  /*  6 */ { "OHM",    "OH",  "Ohm", { "", "k", "k", "k", "M", "M", "M" } },
  /*  7 */ { "CONT",   "BUZ", "Ohm", { "" } },
  /*  8 */ { "DIODE",  "DI",  "V",   { "" } },
  /*  9 */ { "CAP",    "CA",  "F",   { "n", "n", "n", "µ", "µ", "µ", "m", "m" } },
  /* 10 */ { "°C",     "TE",  "C",   { "" } },
  /* 11 */ { "°F",     "TE",  "dF",  { "" } },
  /* 12 */ { "DCuA",   "",    "A",   { "µ" } },
  /* 13 */ { "ACuA",   "",    "A",   { "µ" } },
  /* 14 */ { "DCmA",   "",    "A",   { "m", "" } },
  /* 15 */ { "ACmA",   "",    "A",   { "m", "" } },
  /* 16 */ { "DCA",    "",    "A",   { "" } },
  /* 17 */ { "ACA",    "",    "A",   { "" } },
  /* 18 */ { "HFE",    "HFE", "",    { "" } },
  /* 19 */ { "Live",   "NCV", "",    { "" } },
  /* 20 */ { "NCV",    "NCV", "",    { "" } },
  /* 21 */ { "LozV",   "",    "V",   { "" } },
  /* 22 */ { "ACA",    "",    "A",   { "" } },
  /* 23 */ { "DCA",    "",    "A",   { "" } },
  /* 24 */ { "LPF",    "",    "V",   { "" } },
  /* 25 */ { "AC/DC",  "",    "V",   { "" } },
  /* 26 */ { "LPF",    "",    "V",   { "" } },
  /* 27 */ { "AC+DC",  "",    "A",   { "" } },
  /* 28 */ { "LPFA",   "",    "V",   { "" } },
  /* 29 */ { "AC+DC2", "",    "A",   { "" } },
  /* 30 */ { "INRUSH", "",    "V",   { "" } },
};
constexpr int kFunctionCount = int(sizeof(kFunctions) / sizeof(kFunctions[0]));

// The UT61B+/D+/E+ and the UT161 series speak the same frame with other
// ranges - the vendor tables funOl_UT61E+/D+/B+ and funOl_UT161E/D/B agree
// with each other and with ut61xpy: the V positions start in volts (2.2 V
// resp. 6 V), the mA position stays in mA, CAP is nF up to range 1, Hz is
// Hz up to range 1. Function 25 (AC/DC) is the DC V position that
// alternates DC and AC readings - flags C bit 3 marks the AC ones.
const Function kFunctions61Plus[] = {
  /*  0 */ { "ACV",    "",    "V",   { "" } },
  /*  1 */ { "ACmV",   "",    "V",   { "m" } },
  /*  2 */ { "DCV",    "",    "V",   { "" } },
  /*  3 */ { "DCmV",   "",    "V",   { "m" } },
  /*  4 */ { "Hz",     "HZ",  "Hz",  { "", "", "k", "k", "k", "M", "M", "M" } },
  /*  5 */ { "%",      "DU",  "%",   { "" } },
  /*  6 */ { "OHM",    "OH",  "Ohm", { "", "k", "k", "k", "M", "M", "M" } },
  /*  7 */ { "CONT",   "BUZ", "Ohm", { "" } },
  /*  8 */ { "DIODE",  "DI",  "V",   { "" } },
  /*  9 */ { "CAP",    "CA",  "F",   { "n", "n", "µ", "µ", "µ", "m", "m", "m" } },
  /* 10 */ { "°C",     "TE",  "C",   { "" } },
  /* 11 */ { "°F",     "TE",  "dF",  { "" } },
  /* 12 */ { "DCuA",   "",    "A",   { "µ" } },
  /* 13 */ { "ACuA",   "",    "A",   { "µ" } },
  /* 14 */ { "DCmA",   "",    "A",   { "m" } },
  /* 15 */ { "ACmA",   "",    "A",   { "m" } },
  /* 16 */ { "DCA",    "",    "A",   { "" } },
  /* 17 */ { "ACA",    "",    "A",   { "" } },
  /* 18 */ { "HFE",    "HFE", "",    { "" } },
  /* 19 */ { "Live",   "NCV", "",    { "" } },
  /* 20 */ { "NCV",    "NCV", "",    { "" } },
  /* 21 */ { "LozV",   "",    "V",   { "" } },
  /* 22 */ { "ACA",    "",    "A",   { "" } },
  /* 23 */ { "DCA",    "",    "A",   { "" } },
  /* 24 */ { "LPF",    "",    "V",   { "" } },
  /* 25 */ { "AC/DC",  "",    "V",   { "" } },
  /* 26 */ { "LPF",    "",    "V",   { "" } },
  /* 27 */ { "AC+DC",  "",    "A",   { "" } },
  /* 28 */ { "LPF",    "",    "V",   { "" } },
  /* 29 */ { "AC+DC2", "",    "A",   { "" } },
  /* 30 */ { "INRUSH", "",    "V",   { "" } },
};
constexpr int kFunctionCount61Plus = int(sizeof(kFunctions61Plus) / sizeof(kFunctions61Plus[0]));

// The poll: AB CD, length 3, '^' (0x5E), 0x01, 16-bit BE sum of the first four
const QByteArray kPoll = QByteArray::fromHex("abcd035e01d9");
}

QByteArray DecoderUniTiDMM::pollRequest() const
{
  return kPoll;
}

bool DecoderUniTiDMM::frameValid(const unsigned char *f)
{
  if (f[0] != 0xAB || f[1] != 0xCD || f[2] != kFrameLength - 3)
    return false;
  unsigned sum = 0;
  for (int i = 0; i < kFrameLength - 2; ++i)
    sum += f[i];
  return ((sum >> 8) & 0xFF) == f[kFrameLength - 2] && (sum & 0xFF) == f[kFrameLength - 1];
}

// The last kFrameLength bytes form a frame: header, length and checksum all
// have to agree, so a stray name answer (11 bytes) or a split notification
// only delays the next frame, it never produces a false one.
bool DecoderUniTiDMM::checkFormat(const char *data, size_t idx)
{
  if (idx + 1 < size_t(kFrameLength))
    return false;
  return frameValid(reinterpret_cast<const unsigned char *>(data) + idx + 1 - kFrameLength);
}

std::optional<DmmDecoder::DmmResponse> DecoderUniTiDMM::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id = id;
  m_result.showBar = true;
  if (data.size() < kFrameLength)
    return std::nullopt;
  const auto *f = reinterpret_cast<const unsigned char *>(data.constData()) + data.size() - kFrameLength;
  if (!frameValid(f))
    return std::nullopt;

  const int fn = f[3] & 0x7F;
  const bool plus = m_type == ReadEvent::UniTUT61Plus;
  if (fn >= (plus ? kFunctionCount61Plus : kFunctionCount))
    return std::nullopt;
  const Function &func = plus ? kFunctions61Plus[fn] : kFunctions[fn];
  const int range = f[4] - '0';
  const unsigned char flagsA = f[14], flagsB = f[15], flagsC = f[16];

  // prefix for this range; past the listed ranges the last one listed
  const char *prefix = func.prefixes[0];
  for (int r = qBound(0, range, 7); r >= 0; --r)
    if (func.prefixes[r])
    {
      prefix = func.prefixes[r];
      break;
    }

  m_result.special = *func.special ? QString::fromLatin1(func.special)
                                   : QString::fromLatin1(flagsC & 0x08 ? "AC" : "DC");
  // UT61E+ DC V position with AC shown alongside: the meter alternates, the
  // AC frames become the second value (display line 2, "2nd" in the table)
  if (plus && fn == 25 && (flagsC & 0x08))
    m_result.id = id + 1;
  m_result.hold = flagsA & 0x02;
  m_result.range = (flagsB & 0x04) ? "MANU" : "AUTO";
  m_result.unit = QString::fromUtf8(prefix) + QString::fromLatin1(func.unit);

  // the LCD text: sign and decimal point included; the dot also floats
  // inside "OL" ("OL.", "O.L", ".OL") depending on the range
  QString text = QString::fromLatin1(reinterpret_cast<const char *>(f + 5), 7).trimmed();
  QString bare = text;
  bare.remove('.').remove(' ');
  static const QRegularExpression overload("^-?OL$");
  if (overload.match(bare).hasMatch())
  {
    m_result.val = bare.startsWith('-') ? "-OL" : "OL";
    m_result.dval = 0;
    return m_result;
  }
  text.remove(' ');
  bool ok = false;
  const double shown = text.toDouble(&ok);
  if (!ok)
  {
    // NCV strength bar ("EF", "----"), no number: show it as it is
    m_result.val = text;
    m_result.dval = 0;
    return m_result;
  }
  m_result.val = text;
  m_result.dval = shown * prefixFactor(QString::fromUtf8(prefix));
  return m_result;
}
