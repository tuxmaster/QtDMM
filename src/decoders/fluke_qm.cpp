// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "fluke_qm.h"

#include <cmath>
#include <limits>

#include "siprefix.h"

// Fluke Remote Interface Specifications (docs/protocols/sources/fluke_18x_remote.pdf,
// fluke_28x_remote.pdf) and libsigrok fluke-dmm. No meter at hand: all "*".
// Display counts: 87-IV 20000, the others 50000.
static const bool registered = []() {
  DmmDecoder::addConfig({"Fluke", "87-IV *", "", 9600, ReadEvent::FlukeQM, 8, 1, 1, 0, 20000, 0, 0, 0});
  DmmDecoder::addConfig({"Fluke", "89-IV *", "", 9600, ReadEvent::FlukeQM, 8, 1, 1, 0, 50000, 0, 0, 0});
  DmmDecoder::addConfig({"Fluke", "187 *", "", 9600, ReadEvent::FlukeQM, 8, 1, 1, 0, 50000, 0, 0, 0});
  DmmDecoder::addConfig({"Fluke", "189 *", "", 9600, ReadEvent::FlukeQM, 8, 1, 1, 0, 50000, 0, 0, 0});
  DmmDecoder::addConfig({"Fluke", "287 *", "", 115200, ReadEvent::FlukeQM, 8, 1, 1, 0, 50000, 0, 0, 0});
  DmmDecoder::addConfig({"Fluke", "289 *", "", 115200, ReadEvent::FlukeQM, 8, 1, 1, 0, 50000, 0, 0, 0});
  return true;
}();

bool DecoderFlukeQM::checkFormat(const char *data, size_t idx)
{
  // A line ends with CR. The one-character CMD_ACK ("0\r") is left in the
  // buffer on purpose: it becomes the first line of the next frame, and
  // decode() takes the last line. m_length restarts at 0 after each frame,
  // so idx is the line length so far.
  return data[idx] == '\r' && idx >= 2;
}

std::optional<DmmDecoder::DmmResponse> DecoderFlukeQM::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id = id;
  m_result.showBar = true;
  m_result.range = "AUTO";

  // last non-empty line of the frame; the lines before it are CMD_ACKs
  const QList<QByteArray> lines = data.split('\r');
  QString line;
  for (int i = lines.size() - 1; i >= 0 && line.isEmpty(); --i)
    line = QString::fromLatin1(lines[i]).trimmed();
  if (line.isEmpty())
    return std::nullopt;

  bool ok = false;
  if (line.startsWith("QM,"))
    ok = decode18x(line.mid(3).trimmed());
  else
  {
    const QStringList fields = line.split(',');
    if (fields.size() == 4)
      ok = decode28x(fields);
  }
  if (!ok)
    return std::nullopt;
  return m_result;
}

// ---- 87-IV/89-IV/187/189: "QM,+47.66 KOhms" ------------------------------

bool DecoderFlukeQM::unit18x(const QString &word, QString &prefix, QString &unit, QString &special)
{
  // word is the unit text without blanks: "VDC", "mVAC+DC", "KOhms", "DegC"
  QString w = word;
  prefix.clear();
  special.clear();

  auto coupling = [&](const QString &base) -> bool
  {
    // <prefix><base><AC|DC|AC+DC>
    QString s = w;
    if (s.endsWith("AC+DC"))      { special = "ACDC"; s.chop(5); }
    else if (s.endsWith("AC"))    { special = "AC";   s.chop(2); }
    else if (s.endsWith("DC"))    { special = "DC";   s.chop(2); }
    else
      return false;
    if (!s.endsWith(base))
      return false;
    s.chop(base.size());
    if (s.size() > 1)
      return false;
    prefix = s == "u" ? QStringLiteral("µ") : s;
    unit = base;
    return true;
  };

  if (w.endsWith("Ohms"))
  {
    w.chop(4);
    if (w.size() > 1) return false;
    prefix = w == "K" ? QStringLiteral("k") : w;
    unit = "Ohm";
    special = "OH";
    return true;
  }
  if (w.endsWith("Farads"))
  {
    w.chop(6);
    if (w.size() > 1) return false;
    prefix = w == "u" ? QStringLiteral("µ") : w;
    unit = "F";
    special = "CA";
    return true;
  }
  if (w == "DegC")  { unit = "C";   special = "TE"; return true; }
  if (w == "DegF")  { unit = "dF";  special = "TE"; return true; }
  if (w == "dBm")   { unit = "dBm"; special = "AC"; return true; }
  if (w == "dBV")   { unit = "dBV"; special = "AC"; return true; }
  if (w == "nS")    { prefix = "n"; unit = "S"; special = "OH"; return true; }
  if (w == "%")     { unit = "%";   special = "FR"; return true; }
  if (w == "mS" || w == "ms") { prefix = "m"; unit = "s"; special = "FR"; return true; }
  if (w.endsWith("Hz"))
  {
    w.chop(2);
    if (w.size() > 1) return false;
    prefix = w;
    unit = "Hz";
    special = "FR";
    return true;
  }
  if (coupling("V") || coupling("A"))
    return true;
  return false;
}

bool DecoderFlukeQM::decode18x(const QString &reading)
{
  // "<sign><number> <unit words>" or "Out of Range <unit words>"
  const bool overload = reading.startsWith("Out of Range", Qt::CaseInsensitive);
  QString number, rest;
  if (overload)
    rest = reading.mid(12);
  else
  {
    const int blank = reading.indexOf(' ');
    if (blank < 0)
      return false;
    number = reading.left(blank);
    rest = reading.mid(blank);
  }
  rest.remove(' ');

  QString prefix, unit, special;
  if (!unit18x(rest, prefix, unit, special))
    return false;

  m_result.unit = prefix + unit;
  m_result.special = special;
  if (overload)
  {
    m_result.val = "OL";
    m_result.dval = 0;
    return true;
  }
  bool ok = false;
  const double value = number.toDouble(&ok);
  if (!ok)
    return false;   // happens while the meter switches functions
  m_result.val = number.startsWith('+') ? number.mid(1) : number;
  m_result.dval = value * SiPrefix::factor(prefix);
  return true;
}

// ---- 287/289: "-0.023E-3,VDC,NORMAL,NONE" ------------------------------

bool DecoderFlukeQM::decode28x(const QStringList &fields)
{
  const QString number = fields[0].trimmed();
  const QString unitToken = fields[1].trimmed();
  const QString state = fields[2].trimmed();
  const QString attribute = fields[3].trimmed();

  struct UnitRow { const char *token; const char *unit; const char *special; };
  static const UnitRow units[] = {
    { "VDC", "V", "DC" }, { "VAC", "V", "AC" }, { "VAC_PLUS_DC", "V", "ACDC" }, { "V", "V", "" },
    { "ADC", "A", "DC" }, { "AAC", "A", "AC" }, { "AAC_PLUS_DC", "A", "ACDC" }, { "A", "A", "" },
    { "OHM", "Ohm", "OH" }, { "SIE", "S", "OH" }, { "Hz", "Hz", "FR" }, { "S", "s", "FR" },
    { "F", "F", "CA" }, { "CEL", "C", "TE" }, { "FAR", "dF", "TE" }, { "PCT", "%", "FR" },
    { "dBm", "dBm", "AC" }, { "dBV", "dBV", "AC" }, { "dB", "dB", "AC" }, { "CREST_FACTOR", "", "AC" },
  };
  const UnitRow *row = nullptr;
  for (const UnitRow &u : units)
    if (unitToken == QLatin1String(u.token))
      row = &u;
  if (!row)
    return false;   // NONE, or a unit this table does not know

  QString unit = row->unit;
  m_result.special = row->special;
  if (unitToken == "OHM" && (attribute == "OPEN_CIRCUIT" || attribute == "SHORT_CIRCUIT"))
    m_result.special = "BUZ";
  else if (unitToken == "VDC" && attribute == "GOOD_DIODE")
    m_result.special = "DI";

  if (state == "OL" || state == "OL_MINUS")
  {
    m_result.val = state == "OL" ? "OL" : "-OL";
    m_result.unit = unit;
    m_result.dval = 0;
    return true;
  }
  if (state == "OPEN_TC")
  {
    m_result.val = "OPEN";   // no thermocouple
    m_result.unit = unit;
    m_result.dval = 0;
    return true;
  }
  if (state != "NORMAL")
    return false;   // INVALID, BLANK, DISCHARGE: nothing to show

  bool ok = false;
  const double value = number.toDouble(&ok);
  if (!ok || !std::isfinite(value))
    return false;
  m_result.dval = value;

  // Keep the meter's digits: "-0.023E-3" is "-0.023 mV", not "-23 µV". The
  // exponent is a multiple of three whenever the meter uses a prefixed range.
  const int e = number.indexOf('E', 0, Qt::CaseInsensitive);
  int exponent = 0;
  bool expOk = e >= 0;
  if (expOk)
    exponent = number.mid(e + 1).toInt(&expOk);
  static const QMap<int, QString> prefixes = {
    { -12, "p" }, { -9, "n" }, { -6, "µ" }, { -3, "m" }, { 0, "" }, { 3, "k" }, { 6, "M" }, { 9, "G" } };
  if (expOk && prefixes.contains(exponent))
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
  return true;
}
