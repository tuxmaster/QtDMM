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
static QString readFile(const QString &fileName)
{
  QFile f(fileName);
  if (!f.open(QIODevice::ReadOnly))
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
      check(sampleTime >= 1 && sampleTime <= 5,
            QString("sampleTime regression: got implausible sampleTime %1 for a ~54s/271-row recording").arg(sampleTime));
    }
  }

  if (failed == 0)
    qInfo() << "All DMMGraph baseline tests passed.";
  else
    qWarning() << failed << "DMMGraph baseline test(s) failed.";

  return failed == 0 ? 0 : 1;
}
