// Baseline behavior tests for DMMGraph, written before the planned QtGraph-based
// rewrite so the current CSV import/export contract has a regression net to
// compare the replacement against.
#include <QApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>

#include "dmmgraph.h"
#include "siprefix.h"
#include "engnumbervalidator.h"
#include "settings.h"

static int failed = 0;

static void fail(const QString &what)
{
  qWarning() << "FAILED:" << what;
  failed++;
}

static void check(bool cond, const QString &what)
{
  if (!cond)
    fail(what);
}

// Reads a whole file as text (used to compare export output byte-for-byte).
// Text mode mirrors the exporter, which writes "\r\n" on Windows.
static QString readFile(const QString &fileName)
{
  QFile f(fileName);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    return QString();
  return QString::fromUtf8(f.readAll());
}

int main(int argc, char **argv)
{
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");

  QApplication app(argc, argv);

  if (argc != 2)
  {
    qCritical() << "Usage: test_graph <data-dir>";
    return 1;
  }

  QString dataDir = QString::fromLocal8Bit(argv[1]);
  QTemporaryDir tmpDir;
  check(tmpDir.isValid(), "could not create temp dir for test settings");

  Settings settings("test_graph", tmpDir.path());

  // --- 1. all known fixture formats (legacy tab-separated, legacy with nan,
  //         new semicolon CSV, CSV with comma-decimal ms, larger recording)
  //         must import without error ---
  const QStringList fixtures = {
    "legacy.txt", "legacy_nan.txt", "new_dpoint.csv", "new_komma_ms.csv", "new_larger.csv"
  };

  for (const QString &name : fixtures)
  {
    DMMGraph graph(nullptr, &settings);
    QString path = dataDir + "/" + name;
    bool ok = graph.importCsvFile(path);
    check(ok, QString("importCsvFile() failed for fixture '%1'").arg(name));
  }

  // --- 2. malformed input must be rejected, not crash or half-import ---
  {
    QTemporaryDir badDir;
    QString badFile = badDir.path() + "/broken.csv";
    QFile f(badFile);
    f.open(QIODevice::WriteOnly);
    QTextStream(&f) << "this is not a valid QtDMM export\n";
    f.close();

    DMMGraph graph(nullptr, &settings);
    bool ok = graph.importCsvFile(badFile);
    check(!ok, "importCsvFile() should reject a non-matching file, but reported success");
  }

  // --- 3. import -> export -> re-import must round-trip to the same CSV
  //         (this is the contract the QtGraph replacement needs to preserve) ---
  {
    DMMGraph graph(nullptr, &settings);
    check(graph.importCsvFile(dataDir + "/new_larger.csv"), "round-trip: initial import failed");

    QTemporaryDir outDir;
    QString exported1 = outDir.path() + "/export1.csv";
    check(graph.exportCsvFile(exported1), "round-trip: first export failed");

    DMMGraph graph2(nullptr, &settings);
    check(graph2.importCsvFile(exported1), "round-trip: re-import of exported file failed");

    QString exported2 = outDir.path() + "/export2.csv";
    check(graph2.exportCsvFile(exported2), "round-trip: second export failed");

    QString c1 = readFile(exported1);
    QString c2 = readFile(exported2);
    check(!c1.isEmpty() && c1 == c2,
          "round-trip: re-exporting a re-imported file produced different CSV content");
  }

  // --- 4. regression test for the CSV-import sample-time bug (dmmgraph.cpp):
  //         m_sampleTime used to be computed by summing a growing offset on
  //         every row instead of once after the loop, inflating it with row
  //         count. For new_larger.csv (~271 rows over ~54s) the correct
  //         sample interval is on the order of the actual ~0.2s spacing,
  //         not something that grows with the number of rows. ---
  {
    DMMGraph graph(nullptr, &settings);
    QSignalSpy spy(&graph, &DMMGraph::sampleTime);
    check(graph.importCsvFile(dataDir + "/new_larger.csv"), "sampleTime regression: import failed");
    check(spy.count() == 1, "sampleTime regression: expected exactly one sampleTime() signal");
    if (spy.count() == 1)
    {
      int sampleTime = spy.at(0).at(0).toInt();
      // buggy version summed ~271 growing offsets -> computed a wildly larger
      // value than the true ~54s / 271 rows interval; a healthy value stays small.
      check(sampleTime == 2,
            QString("sampleTime regression: got sampleTime %1 for a 54s/272-row recording, expected 2 (0.2 s)").arg(sampleTime));
    }
  }

  // --- 4b. slower recordings: the sample time must follow the timestamps,
  //         not collapse to 0.1 s (a 5 min / 0.5 s recording used to import
  //         as one minute) ---
  {
    QTemporaryDir dir;
    const QString slow = dir.path() + "/slow.csv";
    QFile f(slow);
    check(f.open(QIODevice::WriteOnly | QIODevice::Text), "slow: create fixture");
    QTextStream out(&f);
    out << "timestamp;time (s);value;unit\n";
    QDateTime t0(QDate(2026, 9, 20), QTime(14, 2, 17, 385));
    for (int i = 0; i < 600; ++i)
      out << t0.addMSecs(i * 500).toString("yyyy-MM-ddTHH:mm:ss,zzz") << ";" << i * 0.5 << ";12.0;V\n";
    f.close();

    DMMGraph graph(nullptr, &settings);
    QSignalSpy spy(&graph, &DMMGraph::sampleTime);
    check(graph.importCsvFile(slow), "slow: import failed");
    check(spy.count() == 1 && spy.at(0).at(0).toInt() == 5,
          QString("slow: sample time %1, expected 5 (0.5 s)").arg(spy.count() ? spy.at(0).at(0).toInt() : -1));

    // and it exports with the original spacing
    const QString back = dir.path() + "/back.csv";
    check(graph.exportCsvFile(back), "slow: export failed");
    const QStringList lines = readFile(back).split('\n', Qt::SkipEmptyParts);
    check(lines.size() == 601 && lines.last().startsWith("2026-09-20T14:07:16,885"),
          QString("slow: exported %1 lines, last '%2'").arg(lines.size()).arg(lines.value(lines.size() - 1).left(23)));
  }

  // --- 5. smoke test for addValue()'s live-recording ring buffer against the
  //         Qt Charts series sync (rebuildSeries()/append()): must survive a
  //         buffer wrap without crashing, and setGraphSize() must be callable
  //         again afterwards while data already exists. ---
  {
    DMMGraph graph(nullptr, &settings);
    graph.setSampleTime(1);
    graph.setGraphSize(5, 5); // small window -> wraps quickly
    graph.setMode(DMMGraph::Manual);
    graph.startSLOT();

    for (int i = 0; i < 20; i++)
      graph.addValue(i * 0.1);

    check(graph.dirty(), "ring-buffer smoke test: expected graph to be marked dirty after recording");

    graph.setGraphSize(10, 10);
    graph.addValue(1.23);
  }

  // --- 6. engineering-prefix export/import: setUnit() must strip a leading
  //         G or p prefix too (previously only n/u/m/k/M were recognized), and
  //         a value re-imported from a prefix-scaled export (e.g. "2.5;pF")
  //         must round-trip back to the same raw value, not get double-scaled
  //         into something like "ppF" on the next export. ---
  {
    QTemporaryDir outDir;

    auto exportedUnitFor = [&](const QString &unit, double rawValue) -> QString
    {
      DMMGraph graph(nullptr, &settings);
      graph.setUnit(unit);
      graph.setSampleTime(10);
      graph.setGraphSize(5, 5);
      graph.setMode(DMMGraph::Manual);
      graph.startSLOT();
      graph.addValue(rawValue);

      QString path = outDir.path() + "/prefix_probe.csv";
      if (!graph.exportCsvFile(path))
        return QString();

      QStringList lines = readFile(path).split('\n', Qt::SkipEmptyParts);
      if (lines.size() < 2)
        return QString();
      return lines[1].split(';').value(3); // timestamp;time;value;unit
    };

    check(exportedUnitFor("GHz", 2.5e9) == "GHz",
          "setUnit() should strip a leading 'G' prefix so re-exporting a GHz-range value stays 'GHz', not 'GGHz' or 'Hz'");
    check(exportedUnitFor("pF", 2.5e-12) == "pF",
          "setUnit() should strip a leading 'p' prefix so re-exporting a pF-range value stays 'pF', not doubled to 'ppF'");

    // Full round trip at an extreme prefix: import a pF-range export, export
    // again, and the two exports must be byte-for-byte identical.
    {
      DMMGraph graph(nullptr, &settings);
      graph.setUnit("F");
      graph.setSampleTime(10);
      graph.setGraphSize(5, 5);
      graph.setMode(DMMGraph::Manual);
      graph.startSLOT();
      graph.addValue(2.5e-12);

      QString exported1 = outDir.path() + "/pf_export1.csv";
      check(graph.exportCsvFile(exported1), "pF round-trip: first export failed");

      DMMGraph graph2(nullptr, &settings);
      check(graph2.importCsvFile(exported1), "pF round-trip: re-import failed");

      QString exported2 = outDir.path() + "/pf_export2.csv";
      check(graph2.exportCsvFile(exported2), "pF round-trip: second export failed");

      QString c1 = readFile(exported1);
      QString c2 = readFile(exported2);
      check(!c1.isEmpty() && c1 == c2,
            "pF round-trip: re-exporting a re-imported pF-range file produced different CSV content");
    }
  }

  // --- 7. micro: export writes "µ", and both "µ" and the ASCII "u" of older
  //         exports must import with the same 1e-6 factor. Before the shared
  //         SiPrefix table the importer only knew "u", so a "µA" file came back
  //         a million times too large. ---
  {
    QTemporaryDir outDir;

    {
      DMMGraph graph(nullptr, &settings);
      graph.setUnit("A");
      graph.setSampleTime(10);
      graph.setGraphSize(5, 5);
      graph.setMode(DMMGraph::Manual);
      graph.startSLOT();
      graph.addValue(2.5e-6);
      QString path = outDir.path() + "/micro_export.csv";
      check(graph.exportCsvFile(path), "micro: export failed");
      QStringList lines = readFile(path).split('\n', Qt::SkipEmptyParts);
      check(lines.size() >= 2 && lines[1].split(';').value(3) == QString::fromUtf8("µA"),
            "a 2.5e-6 A value should export with the unit 'µA'");
    }

    auto importedValue = [&](const QString &unitInFile) -> double
    {
      QString path = outDir.path() + "/micro_" + QString::number(qHash(unitInFile)) + ".csv";
      QFile f(path);
      if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return -1;
      QTextStream out(&f);
      out << "timestamp;time (s);value;unit\n"
          << "2026-09-19T10:00:00,000;0;2.5;" << unitInFile << "\n"
          << "2026-09-19T10:00:01,000;1;2.5;" << unitInFile << "\n";
      f.close();

      DMMGraph graph(nullptr, &settings);
      if (!graph.importCsvFile(path))
        return -1;
      QString exported = path + ".out.csv";
      if (!graph.exportCsvFile(exported))
        return -1;
      QStringList lines = readFile(exported).split('\n', Qt::SkipEmptyParts);
      if (lines.size() < 2)
        return -1;
      QStringList cols = lines[1].split(';');
      return cols.value(2).toDouble() * SiPrefix::factor(SiPrefix::split(cols.value(3)).prefix);
    };

    check(qFuzzyCompare(importedValue("µA"), 2.5e-6),
          "importing '2.5;µA' should yield 2.5e-6 A");
    check(qFuzzyCompare(importedValue("uA"), 2.5e-6),
          "importing the older ASCII spelling '2.5;uA' should yield 2.5e-6 A as well");
  }

  // --- 8. EngNumberValidator: what engValue() writes, value() must read
  //         back. engValue() emits "µ" while value() used to recognise only
  //         "u", so micro thresholds silently lost their factor. ---
  {
    struct { double v; const char *text; } cases[] = {
      {1500.0,   "1.5k"},
      {0.0015,   "1.5m"},
      {1.5e-6,   "1.5µ"},
      {2.5e9,    "2.5G"},
      {42.0,     "42"},
    };
    for (const auto &c : cases)
    {
      QString written = EngNumberValidator::engValue(c.v);
      check(written == QString::fromUtf8(c.text),
            QString("engValue(%1) should be '%2', got '%3'").arg(c.v).arg(c.text).arg(written));
      check(qFuzzyCompare(EngNumberValidator::value(written) + 1.0, c.v + 1.0),
            QString("value(engValue(%1)) should round-trip, got %2")
              .arg(c.v).arg(EngNumberValidator::value(written)));
    }
    check(qFuzzyCompare(EngNumberValidator::value("1.5u"), 1.5e-6),
          "value() should accept the ASCII 'u' for micro");
  }

  if (failed == 0)
    qInfo() << "All DMMGraph baseline tests passed.";
  else
    qWarning() << failed << "DMMGraph baseline test(s) failed.";

  return failed == 0 ? 0 : 1;
}
