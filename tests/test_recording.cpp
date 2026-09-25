// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
//
// RecordingFile: the recorder's CSV formats without any widget. Reads the
// fixtures under tests/data/graph, checks the derived sample time, unit
// scaling (µ and the older ASCII u), the legacy tab format, round trips and
// the error paths.

#include <QtCore>

#include "recordingfile.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAIL:" << what;
    ++failed;
  }
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  if (argc != 2)
  {
    qCritical() << "usage: test_recording <tests/data/graph>";
    return 1;
  }
  const QString dataDir = QString::fromLocal8Bit(argv[1]);
  QTemporaryDir tmp;

  // --- 1. every fixture reads, with a plausible sample time ---
  for (const char *name : {"legacy.txt", "legacy_nan.txt", "new_dpoint.csv", "new_komma_ms.csv", "new_larger.csv"})
  {
    QString err;
    const auto rec = RecordingFile::read(dataDir + "/" + name, &err);
    check(rec.has_value(), QString("%1 reads (%2)").arg(name, err));
    if (rec)
    {
      check(!rec->values.isEmpty() && rec->start.isValid(), QString("%1: values and start").arg(name));
      check(rec->sampleTimeTenths >= 1, QString("%1: sample time >= 1").arg(name));
      check(!rec->unit.isEmpty(), QString("%1: has a unit").arg(name));
    }
  }

  // --- 2. new_larger.csv: 272 rows over ~54 s -> 2 tenths, start from the first row ---
  {
    const auto rec = RecordingFile::read(dataDir + "/new_larger.csv");
    check(rec && rec->sampleTimeTenths == 2, QString("sample time 0.2 s (got %1)").arg(rec ? rec->sampleTimeTenths : -1));
    check(rec && rec->start == QDateTime(QDate(2025, 8, 11), QTime(10, 25, 22, 289)), "start = first timestamp");
    check(rec && qFuzzyCompare(rec->values[0], 0.0327), "first value");
  }

  // --- 3. legacy: one row per second -> 10 tenths, nan -> 0 ---
  {
    const auto rec = RecordingFile::read(dataDir + "/legacy.txt");
    check(rec && rec->sampleTimeTenths == 10, "legacy: 1 s sample time");
    const auto nan = RecordingFile::read(dataDir + "/legacy_nan.txt");
    check(nan.has_value(), "legacy with nan reads");
    if (nan)
      check(std::none_of(nan->values.cbegin(), nan->values.cend(), [](double v) { return std::isnan(v); }), "nan becomes 0");
  }

  // --- 4. units: values come back in the base unit, µ and u alike; a
  //        different prefix in a later row is scaled ---
  auto writeCsv = [&](const QString &name, const QString &body)
  {
    const QString path = tmp.path() + "/" + name;
    QFile f(path);
    check(f.open(QIODevice::WriteOnly | QIODevice::Text), "temp file opens for writing");
    QTextStream(&f) << "timestamp;time (s);value;unit\n" << body;
    return path;
  };
  for (const QString &micro : {QString::fromUtf8("µ"), QString("u")})
  {
    const auto rec = RecordingFile::read(writeCsv("micro.csv",
      "2026-01-01T00:00:00,000;0;2.5;" + micro + "A\n2026-01-01T00:00:01,000;1;1.5;mA\n2026-01-01T00:00:02,000;2;3;A\n"));
    check(rec && rec->unit == "A", "micro: unit A");
    check(rec && qFuzzyCompare(rec->values[0], 2.5e-6), "micro " + micro + ": 2.5e-6 A");
    check(rec && qFuzzyCompare(rec->values[1], 1.5e-3), "milli row scaled");
    check(rec && qFuzzyCompare(rec->values[2], 3.0), "base row unscaled");
  }

  // --- 5. write -> read round trip, and the written text itself ---
  {
    Recording rec;
    rec.start = QDateTime(QDate(2026, 9, 21), QTime(14, 3, 5, 250));
    rec.sampleTimeTenths = 5;
    rec.unit = "V";
    rec.values = {0.0327, 1234.5, 2.5e-6, -0.5};
    const QString path = tmp.path() + "/out.csv";
    QString err;
    check(RecordingFile::write(rec, path, &err), "write succeeds: " + err);
    QFile f(path);
    check(f.open(QIODevice::ReadOnly | QIODevice::Text), "written file opens");   // Text: the file has \r\n on Windows
    const QStringList lines = QString::fromUtf8(f.readAll()).split('\n', Qt::SkipEmptyParts);
    check(lines.size() == 5 && lines[0] == "timestamp;time (s);value;unit", "header and four rows");
    check(lines.size() > 2 && lines[2].startsWith("2026-09-21T14:03:05,750;0.5;"), "second row 0.5 s later: " + (lines.size() > 2 ? lines[2] : ""));
    check(lines.size() > 2 && lines[2].endsWith("kV"), "1234.5 V written with the k prefix: " + (lines.size() > 2 ? lines[2] : ""));
    check(lines.size() > 3 && lines[3].endsWith(QString::fromUtf8("µV")), "2.5e-6 V written as µV");
    const auto back = RecordingFile::read(path, &err);
    check(back.has_value(), "round trip reads: " + err);
    if (back)
    {
      check(back->start == rec.start && back->sampleTimeTenths == 5 && back->unit == "V", "round trip: start, sample time, unit");
      check(back->values.size() == 4, "round trip: four values");
      for (int i = 0; i < qMin(4, back->values.size()); ++i)
        check(std::fabs(back->values[i] - rec.values[i]) <= 1e-6 * std::fabs(rec.values[i]) + 1e-12,
              QString("round trip value %1: %2 vs %3").arg(i).arg(back->values[i]).arg(rec.values[i]));
    }
  }

  // --- 6. errors: missing file, header only, garbage, nothing to write ---
  {
    QString err;
    check(!RecordingFile::read(tmp.path() + "/nope.csv", &err) && !err.isEmpty(), "missing file is an error");
    check(!RecordingFile::read(writeCsv("empty.csv", ""), &err) && err.contains("header"), "header only: " + err);
    check(!RecordingFile::read(writeCsv("bad.csv", "this is not a recording\n"), &err) && !err.isEmpty(), "garbage rejected");
    check(!RecordingFile::write(Recording(), tmp.path() + "/none.csv", &err) && !err.isEmpty(), "empty recording is not written");
  }

  // --- 6. writeAny: a spreadsheet by suffix, the CSV otherwise ---
  {
    Recording rec;
    rec.start = QDateTime(QDate(2026, 9, 21), QTime(14, 3, 5, 250));
    rec.sampleTimeTenths = 5;
    rec.unit = "V";
    rec.values = {1.0, 2.0, 3.0};
    QString err;
    check(RecordingFile::writeAny(rec, tmp.filePath("a.xlsx"), &err), "xlsx via writeAny: " + err);
    check(RecordingFile::writeAny(rec, tmp.filePath("a.ods"), &err), "ods via writeAny: " + err);
    check(RecordingFile::writeAny(rec, tmp.filePath("a.csv"), &err), "csv via writeAny: " + err);
    check(QFileInfo(tmp.filePath("a.xlsx")).size() > 1000 && QFile(tmp.filePath("a.xlsx")).open(QIODevice::ReadOnly), "xlsx exists");
    QFile z(tmp.filePath("a.xlsx"));
    (void)z.open(QIODevice::ReadOnly);
    check(z.read(2) == "PK", "xlsx is a zip");
    QFile c(tmp.filePath("a.csv"));
    c.open(QIODevice::ReadOnly | QIODevice::Text);
    check(QString::fromUtf8(c.readLine()).startsWith("timestamp;"), "csv is the csv");
    Recording empty;
    check(!RecordingFile::writeAny(empty, tmp.filePath("e.ods"), &err) && !err.isEmpty(), "empty refuses");
  }

  if (failed == 0)
    qInfo() << "All recording file tests passed.";
  else
    qWarning() << failed << "recording file test(s) failed.";
  return failed == 0 ? 0 : 1;
}
