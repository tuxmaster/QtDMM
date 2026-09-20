#pragma once

#include <QString>

/// @file
/// SI prefix handling shared by decoders, recorder and display.

/// The one place that knows SI prefixes. Decoders, the recorder's CSV
/// import/export, the numeric display and the threshold editors all used to
/// carry their own copies, which disagreed on "µ" vs "u" and on which prefixes
/// exist at all.
namespace SiPrefix
{
  /// A unit taken apart into prefix and base unit.
  struct Split
  {
    QString prefix;     ///< "k", "m", "µ", ... or empty
    QString baseUnit;   ///< "V", "Ohm", "Hz", ...
  };

  /// "k" -> 1e3. Accepts both "µ" and "u" for micro; anything unknown -> 1.0.
  double factor(const QString &prefix);

  /// "kOhm" -> {"k", "Ohm"}, "Ohm" -> {"", "Ohm"}. A leading prefix letter only
  /// counts when something follows it, so a bare "m" stays a unit.
  Split split(const QString &unit);

  /// Scales value into [1, 1000) and reports the prefix that goes with it:
  /// 0.00123 -> 1.23 with prefix "m"; 0 stays 0 with no prefix. Micro is "µ".
  double scale(double value, QString *prefixOut);

  /// scale() rendered as text with up to 12 significant digits.
  QString format(double value, QString *prefixOut = nullptr);
}
