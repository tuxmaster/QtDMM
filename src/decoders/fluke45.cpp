// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fluke45.h"

#include "siprefix.h"

// Fluke 45 Users Manual, chapter 5 (docs/protocols/sources/); no meter at
// hand, so "*". 100000 counts (5 digits), baud rate as set on the meter.
static const bool registered = []() {
  DmmDecoder::addConfig({"Fluke", "45 *", "", 9600, ReadEvent::Fluke45, 8, 1, 1, 0, 100000, 0, 0, 0});
  return true;
}();

bool DecoderFluke45::checkFormat(const char *data, size_t idx)
{
  // Lines end with CR LF. The frame starts at index 0 (the reader restarts
  // there after each frame). A prompt line ("=>", "?>", "!>") or an echo of
  // our own query (contains '?') is not a frame: it stays in the buffer and
  // becomes the first line of the next frame, which decode() skips.
  if (data[idx] != '\n' || idx < 2 || data[idx - 1] != '\r')
    return false;
  size_t start = idx - 1;
  while (start > 0 && data[start - 1] != '\n')
    --start;
  if (idx - start <= 1)   // an empty line
    return false;
  if (data[start] == '=' || data[start] == '?' || data[start] == '!')
    return false;
  for (size_t i = start; i < idx; ++i)
    if (data[i] == '?')
      return false;
  return true;
}

std::optional<DmmDecoder::DmmResponse> DecoderFluke45::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id = id;
  m_result.showBar = true;

  // the last non-empty line; earlier ones are prompts or echoes
  const QList<QByteArray> lines = data.split('\n');
  QString line;
  for (int i = lines.size() - 1; i >= 0 && line.isEmpty(); --i)
    line = QString::fromLatin1(lines[i]).trimmed();

  // FUNC1;AUTO;MOD;VAL1 - the meter answers the compound query in one line
  const QStringList fields = line.split(';');
  if (fields.size() != 4)
    return std::nullopt;
  const QString function = fields[0].trimmed();
  m_result.range = fields[1].trimmed() == "1" ? "AUTO" : "MANU";
  const int modifiers = fields[2].trimmed().toInt();
  QString number = fields[3].trimmed();
  if (number.contains(','))   // both displays on: "VAL1" would not do that, but be safe
    number = number.section(',', 0, 0).trimmed();

  struct FunctionRow { const char *mnemonic; const char *unit; const char *special; };
  static const FunctionRow functions[] = {
    { "VDC", "V", "DC" }, { "VAC", "V", "AC" }, { "VACDC", "V", "ACDC" },
    { "ADC", "A", "DC" }, { "AAC", "A", "AC" }, { "AACDC", "A", "ACDC" },
    { "OHMS", "Ohm", "OH" }, { "FREQ", "Hz", "FR" }, { "DIODE", "V", "DI" }, { "CONT", "V", "BUZ" },
  };
  const FunctionRow *row = nullptr;
  for (const FunctionRow &f : functions)
    if (function == QLatin1String(f.mnemonic))
      row = &f;
  if (!row)
    return std::nullopt;

  QString unit = row->unit;
  m_result.special = row->special;
  m_result.hold = modifiers & 0x04;      // 1 MN, 2 MX, 4 HOLD, 8 dB, 16 dB power, 32 REL, 64 COMP
  if (modifiers & 0x10)
    unit = "W";       // dB Power: the primary display shows watts
  else if (modifiers & 0x08)
    unit = "dBm";

  if (number == "+1E+9" || number == "-1E+9")
  {
    m_result.val = number.startsWith('-') ? "-OL" : "OL";
    m_result.unit = unit;
    m_result.dval = 0;
    return m_result;
  }

  bool ok = false;
  const double value = number.toDouble(&ok);
  if (!ok)
    return std::nullopt;
  m_result.dval = value;

  // "+12.345E+6" is shown as "12.345 MOhm": keep the meter's digits when the
  // exponent is a multiple of three, otherwise let SiPrefix pick the prefix.
  const int e = number.indexOf('E', 0, Qt::CaseInsensitive);
  int exponent = 0;
  bool expOk = e >= 0;
  if (expOk)
    exponent = number.mid(e + 1).toInt(&expOk);
  static const QMap<int, QString> prefixes = {
    { -9, "n" }, { -6, "µ" }, { -3, "m" }, { 0, "" }, { 3, "k" }, { 6, "M" }, { 9, "G" } };
  if (expOk && prefixes.contains(exponent) && !unit.startsWith("dB"))
  {
    QString mantissa = number.left(e);
    if (mantissa.startsWith('+'))
      mantissa.remove(0, 1);
    m_result.val = mantissa;
    m_result.unit = prefixes.value(exponent) + unit;
  }
  else
  {
    QString prefix;
    m_result.val = SiPrefix::format(value, &prefix);
    m_result.unit = prefix + unit;
  }
  return m_result;
}
