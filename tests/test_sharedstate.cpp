// Tests for the instance coordination: two SharedStateManagers in one process
// share a segment (private key via QTDMM_IPC_KEY so a running QtDMM is not
// disturbed), one publishes a reading, the other sees it after a poll.
#include <QCoreApplication>
#include <QDateTime>
#include <QSharedMemory>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

#include "sharedstatemanager.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAILED:" << what;
    failed++;
  }
}

int main(int argc, char **argv)
{
  const QByteArray key = "qtdmm_test_" + QByteArray::number(QCoreApplication::applicationPid());
  qputenv("QTDMM_IPC_KEY", key);
  QCoreApplication app(argc, argv);
  app.setApplicationName("qtdmm_test");

  {
    // a segment left behind by a crashed earlier run would carry stale data
    QSharedMemory leftover(QString::fromLatin1(key));
    if (leftover.attach())
      leftover.detach();
  }

  SharedStateManager u("u");
  SharedStateManager i("i");
  check(u.registerInstance(), "u registers");
  check(i.registerInstance(), "i registers");

  // --- 1. duplicate ids are refused ---
  SharedStateManager dup("u");
  check(!dup.registerInstance(), "second 'u' is refused");

  // --- 2. both see each other ---
  u.checkForChanges();
  check(u.instances().contains("u") && u.instances().contains("i"), "u sees both instances");
  check(u.readings().isEmpty(), "nothing published yet");

  // --- 3. publish and read back ---
  SharedStateManager::Reading r;
  r.value = 12.345;
  r.unit = "V";
  r.special = "DC";
  r.msecs = QDateTime::currentMSecsSinceEpoch();
  r.valid = true;
  u.publishReading(r);

  i.checkForChanges();
  const auto seen = i.readings();
  check(seen.contains("u"), "i sees u's reading");
  check(!seen.contains("i"), "i has not published itself");
  if (seen.contains("u"))
  {
    check(qFuzzyCompare(seen["u"].value, 12.345), "value round-trips");
    check(seen["u"].unit == "V" && seen["u"].special == "DC", "unit and special round-trip");
    check(seen["u"].msecs == r.msecs, "timestamp round-trips");
    check(seen["u"].valid, "valid flag round-trips");
  }

  // --- 4. an unchanged value is not rewritten within a second, a changed one is ---
  SharedStateManager::Reading r2 = r;
  r2.msecs += 10;
  u.publishReading(r2);
  i.checkForChanges();
  check(i.readings()["u"].msecs == r.msecs, "identical value within 1 s is not republished");
  r2.value = 12.5;
  u.publishReading(r2);
  i.checkForChanges();
  check(qFuzzyCompare(i.readings()["u"].value, 12.5) && i.readings()["u"].msecs == r2.msecs,
        "changed value is republished");

  // --- 5. the state channel still works alongside ---
  u.writeState("RECORD");
  i.checkForChanges();
  check(i.readings().contains("u"), "reading survives a state write");

  // --- 6. unregister removes the entry and its reading ---
  u.unregisterInstance();
  i.checkForChanges();
  check(!i.instances().contains("u") && !i.readings().contains("u"), "unregister drops the reading");

  // --- 7. an instance that died without unregistering is not "running" ---
  // (entry left in the segment with a pid that no longer exists)
  {
    QSharedMemory mem(QString::fromLocal8Bit(key));
    check(mem.attach(), "test attaches to the segment");
    mem.lock();
    QByteArray raw(static_cast<const char *>(mem.constData()), mem.size());
    raw.truncate(raw.indexOf('\0'));
    QJsonObject data = QJsonDocument::fromJson(raw).object();
    QJsonArray instances = data["instances"].toArray();
    QJsonObject ghost;
    ghost["id"] = "ghost";
    ghost["pid"] = 999999999;   // pid_t max on Linux is far below this
    QJsonObject reading;
    reading["value"] = 1.0;
    reading["unit"] = "V";
    reading["valid"] = true;
    ghost["reading"] = reading;
    instances.append(ghost);
    data["instances"] = instances;
    const QByteArray json = QJsonDocument(data).toJson(QJsonDocument::Compact);
    memset(mem.data(), 0, mem.size());
    memcpy(mem.data(), json.constData(), json.size());
    mem.unlock();
  }
  i.checkForChanges();
  check(!i.instances().contains("ghost"), "dead instance is not listed");
  check(!i.readings().contains("ghost"), "dead instance's last reading is not offered");
  check(i.instances().contains("i"), "live instance still listed");

  if (failed == 0)
    qInfo() << "All shared state tests passed.";
  else
    qWarning() << failed << "shared state test(s) failed.";
  return failed == 0 ? 0 : 1;
}
