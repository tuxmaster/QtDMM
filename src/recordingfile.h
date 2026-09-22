// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QDateTime>
#include <QString>
#include <QVector>
#include <optional>

/// A recording as it goes to and comes from a CSV file: when it started, how
/// often it sampled, its unit and the values in that unit.
struct Recording
{
  QDateTime start;
  int sampleTimeTenths = 1;   ///< sample interval in tenths of a second (>= 1)
  QString unit;               ///< the graph's unit, without SI prefix ("V")
  QVector<double> values;
};

/// The CSV file formats of the recorder, free of widgets and settings so they
/// can be tested on their own. DMMGraph hands a Recording in and out.
///
/// Written: `timestamp;time (s);value;unit` with ISO 8601 timestamps
/// (`2026-09-21T14:03:05,250`), the value with an SI prefix on the unit.
/// Read: that format, and the legacy tab-separated export of QtDMM < 0.9
/// (`21.09.2026<TAB>14:03:05[:250]<TAB>value<TAB>unit`). The sample time is
/// derived from the time span and the row count.
namespace RecordingFile
{
/// Reads @p path; on failure returns nothing and sets @p error (translated).
std::optional<Recording> read(const QString &path, QString *error = nullptr);
/// Writes @p recording to @p path; false with @p error on failure or when
/// there is nothing to write.
bool write(const Recording &recording, const QString &path, QString *error = nullptr);
/// The same columns as an Excel (.xlsx) or OpenDocument (.ods) sheet, with
/// real date and number cells (SpreadsheetWriter); the format follows the
/// suffix. Falls back to CSV for any other suffix.
bool writeAny(const Recording &recording, const QString &path, QString *error = nullptr);

/// Factor that brings a value in @p unit to @p baseUnit: 1e-3 for "mV" and
/// "V", 1.0 when the units are the same or unrelated. Older exports wrote the
/// ASCII "u" for micro; SiPrefix::factor() takes both.
double unitScaleFactor(const QString &unit, const QString &baseUnit);
}
