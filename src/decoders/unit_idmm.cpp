// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "unit_idmm.h"

#include <QRegularExpression>

static const bool registered = []() {
  // Bluetooth LE: no baud rate; the address is chosen on the Multimeter page
  DmmDecoder::addConfig({"Uni-Trend", "UT60BT", "", 0, ReadEvent::UniTiDMM, 8, 1, 1, 0, 10000, 0, 0, 0});
  // same decoder by lineage (ble-multimeter), not tried with QtDMM
  DmmDecoder::addConfig({"Uni-Trend", "UT161A *", "", 0, ReadEvent::UniTiDMM, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT161B *", "", 0, ReadEvent::UniTiDMM, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT161D *", "", 0, ReadEvent::UniTiDMM, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Uni-Trend", "UT161E *", "", 0, ReadEvent::UniTiDMM, 8, 1, 1, 0, 22000, 0, 0, 0});
  return true;
}();

namespace
{
// fn (byte 3, bit 7 masked) -> special, base unit and the SI prefix per range
// digit (byte 4 - '0'). One prefix string means the unit never changes.
// Function codes from ble-multimeter (types.ts FUNCTIONS[]); ranges checked
// on a UT60BT dial by dial (2026-09-24): the V positions start in a 999.9 mV
// range (range 0, as ut61xpy has it - ble-multimeter's "V" there was a
// transition frame), the mV position has two mV ranges, OHM reaches MOhm at
// range 4. The mA position's range 1 is amperes per ut61xpy (not tried).
// Codes 22-30 are other UNI-T models, ported, not tried.
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
  if (fn >= kFunctionCount)
    return std::nullopt;
  const Function &func = kFunctions[fn];
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
