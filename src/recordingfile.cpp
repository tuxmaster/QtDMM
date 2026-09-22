// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "recordingfile.h"

#include <QCoreApplication>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include "siprefix.h"
#include "spreadsheet.h"

namespace
{
QString tr(const char *text)
{
  return QCoreApplication::translate("RecordingFile", text);
}

void setError(QString *error, const QString &text)
{
  if (error)
    *error = text;
}
}

double RecordingFile::unitScaleFactor(const QString &unit, const QString &baseUnit)
{
  if (unit != baseUnit && unit.endsWith(baseUnit))
    return SiPrefix::factor(unit.left(unit.size() - baseUnit.size()));
  return 1.0;
}

std::optional<Recording> RecordingFile::read(const QString &path, QString *error)
{
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly))
  {
    setError(error, tr("Cannot open file."));
    return std::nullopt;
  }
  QTextStream ts(&file);

  QString line = ts.readLine();
  if (line.isNull())
  {
    setError(error, tr("Oops! Seems not to be a valid file"));
    return std::nullopt;
  }
  if (line.startsWith("timestamp"))   // the CSV header
  {
    line = ts.readLine();
    if (line.isNull())
    {
      setError(error, tr("File contains only header"));
      return std::nullopt;
    }
  }

  static const QRegularExpression reLegacy(
    R"(^(?<day>\d{2})\.(?<month>\d{2})\.(?<year>\d{4})\t(?<hour>\d{2}):(?<minute>\d{2}):(?<second>\d{2})(?::(?<ms>\d{1,3}))?\t(?<value>-?\d+(?:\.\d+)?|nan)\t(?<unit>.*)$)",
    QRegularExpression::CaseInsensitiveOption);
  static const QRegularExpression reCsv(
    R"(^(?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2})T(?<hour>\d{2}):(?<minute>\d{2}):(?<second>\d{2})[,\.](?<ms>\d{1,3});(?<delta>-?\d+(?:\.\d+)?|nan);(?<value>-?\d+(?:\.\d+)?|nan);(?<unit>.*)$)",
    QRegularExpression::CaseInsensitiveOption);

  const bool isLegacy = reLegacy.match(line).hasMatch();
  Recording rec;
  QDateTime end;
  do
  {
    if (!line.trimmed().isEmpty())
    {
      const QRegularExpressionMatch match = isLegacy ? reLegacy.match(line) : reCsv.match(line);
      if (!match.hasMatch())
      {
        setError(error, tr("Oops! Seems not to be a valid file"));
        return std::nullopt;
      }
      const QDateTime when(QDate(match.captured("year").toInt(), match.captured("month").toInt(), match.captured("day").toInt()),
                           QTime(match.captured("hour").toInt(), match.captured("minute").toInt(),
                                 match.captured("second").toInt(), match.captured("ms").toInt()));
      if (rec.values.isEmpty())
      {
        // the recording's unit is the first row's without its SI prefix;
        // every row is scaled to that base unit
        rec.unit = SiPrefix::split(match.captured("unit")).baseUnit;
        rec.start = when;
      }
      end = when;
      const QString value = match.captured("value");
      rec.values << (value.compare("nan", Qt::CaseInsensitive) == 0
                       ? 0.0
                       : value.toDouble() * unitScaleFactor(match.captured("unit"), rec.unit));
    }
    line = ts.readLine();
  }
  while (!line.isNull());

  // Sample time from the span and the row count. (Integer seconds divided by
  // the row count and then by ten, the old formula, came out as 0 -> 1 for
  // anything sampled slower than every 0.1 s.)
  const int count = rec.values.size();
  const qint64 spanMs = rec.start.msecsTo(end);
  rec.sampleTimeTenths = count > 1 ? qMax(1, qRound(spanMs / 100.0 / (count - 1))) : 1;
  return rec;
}

bool RecordingFile::write(const Recording &recording, const QString &path, QString *error)
{
  if (recording.values.isEmpty())
  {
    setError(error, tr("Nothing to export."));
    return false;
  }
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    setError(error, tr("Cannot open file."));
    return false;
  }

  QTextStream ts(&file);
  ts << "timestamp;time (s);value;unit\n";
  const qint64 startMs = recording.start.toMSecsSinceEpoch();
  for (int i = 0; i < recording.values.size(); ++i)
  {
    const QDateTime dt = recording.start.addMSecs(qint64(i) * recording.sampleTimeTenths * 100);
    const double deltaTime = (dt.toMSecsSinceEpoch() - startMs) / 1000.0;
    QString prefix;
    const QString value = SiPrefix::format(recording.values[i], &prefix);
    ts << QString("%1;%2;%3;%4\n")
            .arg(dt.toString("yyyy-MM-ddTHH:mm:ss,zzz"))
            .arg(deltaTime, 0, 'g', 12)
            .arg(value)
            .arg(prefix + recording.unit);
  }
  return true;
}

bool RecordingFile::writeAny(const Recording &recording, const QString &path, QString *error)
{
  const auto format = SpreadsheetWriter::formatForFile(path);
  if (!format)
    return write(recording, path, error);
  if (recording.values.isEmpty())
  {
    setError(error, tr("Nothing to export."));
    return false;
  }
  SpreadsheetWriter sheet(tr("Recording"));
  sheet.setHeader({tr("timestamp"), tr("time (s)"), tr("value"), tr("unit")});
  for (int i = 0; i < recording.values.size(); ++i)
  {
    const QDateTime dt = recording.start.addMSecs(qint64(i) * recording.sampleTimeTenths * 100);
    sheet.addRow({dt, i * recording.sampleTimeTenths / 10.0, recording.values[i], recording.unit});
  }
  return sheet.write(path, *format, error);
}
