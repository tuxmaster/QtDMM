// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
//
// MdnsMessage/MdnsBrowser on a captured answer of qtdmm-bridge 0.2.0
// (tests/data/mdns/qtdmm_bridge_answer.bin: PTR, SRV, A, TXT, NSEC with
// compression pointers), the query builder, truncated packets, and - when
// the environment allows - a live browse against a bridge started with
// --mdns (needs the zeroconf package; skipped otherwise).

#include <QtCore>
#include <QtNetwork>
#include <QtTest>

#include "mdnsbrowser.h"

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
  if (argc < 2)
  {
    qCritical() << "usage: test_mdns <tests/data/mdns> [<tools/qtdmm-bridge>]";
    return 1;
  }
  QFile f(QString::fromLocal8Bit(argv[1]) + "/qtdmm_bridge_answer.bin");
  check(f.open(QIODevice::ReadOnly), "fixture opens");
  const QByteArray packet = f.readAll();
  check(packet.size() == 380, QString("fixture is the captured 380 bytes (%1)").arg(packet.size()));

  // --- 1. records ---
  const QList<MdnsMessage::Record> records = MdnsMessage::parse(packet);
  check(records.size() == 5, QString("five records (%1)").arg(records.size()));
  const MdnsMessage::Record *ptr = nullptr, *srv = nullptr, *txt = nullptr, *a = nullptr;
  for (const MdnsMessage::Record &r : records)
  {
    if (r.type == MdnsMessage::PTR) ptr = &r;
    if (r.type == MdnsMessage::SRV) srv = &r;
    if (r.type == MdnsMessage::TXT) txt = &r;
    if (r.type == MdnsMessage::A) a = &r;
  }
  check(ptr && ptr->name == "_qtdmm-bridge._tcp.local" && ptr->target.startsWith("dory /dev/serial/by-id/usb-Prolific"),
        "PTR: service -> instance (compression pointer to the question)");
  check(srv && srv->port == 4711 && srv->target == "dory.local", "SRV: port 4711 on dory.local");
  check(txt && txt->txt.value("version") == "0.2.0" && txt->txt.value("device").startsWith("/dev/serial/by-id/")
        && txt->txt.contains("name"), "TXT: device, name, version");
  check(a && a->name == "dory.local" && a->address == QHostAddress("192.168.178.184"), "A: 192.168.178.184");

  // --- 2. the browser assembles a service from it ---
  MdnsBrowser browser;
  QList<MdnsBrowser::Service> found;
  QObject::connect(&browser, &MdnsBrowser::found, [&](const MdnsBrowser::Service &s) { found << s; });
  browser.browse("_qtdmm-bridge._tcp.local", 100);   // arms the service name; sockets may or may not open
  browser.handlePacket(packet);
  browser.handlePacket(packet);   // again: reported once
  check(found.size() == 1, QString("one service reported once (%1)").arg(found.size()));
  if (!found.isEmpty())
  {
    check(found[0].instance.startsWith("dory /dev/serial"), "instance without the service suffix: " + found[0].instance);
    check(found[0].host == "dory.local" && found[0].port == 4711, "host and port");
    check(found[0].address == QHostAddress("192.168.178.184"), "address resolved from the A record");
    check(found[0].txt.value("version") == "0.2.0", "txt carried along");
  }
  browser.stop();

  // --- 3. query bytes ---
  const QByteArray q = MdnsMessage::query("_qtdmm-bridge._tcp.local");
  check(q.left(12) == QByteArray::fromHex("000000000001000000000000"), "query header: one question");
  check(q.mid(12, 14) == QByteArray::fromHex("0d5f7174646d6d2d627269646765"), "query name label");
  check(q.right(4) == QByteArray::fromHex("000c0001"), "query type PTR class IN");

  // --- 4. garbage never throws ---
  for (int len : {0, 5, 12, 20, 60, 100, 200, 379})
    MdnsMessage::parse(packet.left(len));
  QByteArray loop = QByteArray::fromHex("000084000000000100000000") + QByteArray::fromHex("c00c000c00010000000000020c0c");
  check(MdnsMessage::parse(loop).isEmpty(), "pointer loop gives up");

  // --- 5. live, if a bridge can announce here ---
  const QString bridgeDir = argc > 2 ? QString::fromLocal8Bit(argv[2]) : QString();
  QProcess zc;
  zc.start("python3", {"-c", "import zeroconf"});
  zc.waitForFinished(5000);
  if (!bridgeDir.isEmpty() && zc.exitCode() == 0 && !qEnvironmentVariableIsSet("QTDMM_NO_LIVE_MDNS"))
  {
    QTemporaryDir tmp;
    QProcess bridge;
    // a pty pair makes a harmless serial port for the bridge to serve
    QProcess pty;
    pty.start("socat", {"-d", "-d", "pty,raw,echo=0,link=" + tmp.filePath("ttyA"), "pty,raw,echo=0,link=" + tmp.filePath("ttyB")});
    bool havePty = pty.waitForStarted(3000) && QTest::qWaitFor([&] { return QFile::exists(tmp.filePath("ttyA")); }, 3000);
    if (!havePty)
      qInfo() << "no socat/pty - live browse skipped";
    else
    {
      bridge.setWorkingDirectory(bridgeDir);
      bridge.start("python3", {"qtdmm_bridge.py", "--port", "47110=" + tmp.filePath("ttyA"), "--bind", "0.0.0.0", "--mdns"});
      check(bridge.waitForStarted(5000), "bridge starts");
      QThread::sleep(2);
      MdnsBrowser live;
      QList<MdnsBrowser::Service> hits;
      QObject::connect(&live, &MdnsBrowser::found, [&](const MdnsBrowser::Service &s) { if (s.port == 47110) hits << s; });
      QEventLoop loop;
      QObject::connect(&live, &MdnsBrowser::finished, &loop, &QEventLoop::quit);
      live.browse("_qtdmm-bridge._tcp.local", 3000);
      if (live.isActive())
        loop.exec();
      check(!hits.isEmpty(), "live: the bridge's announcement was found");
      if (!hits.isEmpty())
        check(hits[0].txt.value("device") == tmp.filePath("ttyA"), "live: txt device is the served port: " + hits[0].txt.value("device"));
      bridge.terminate();
      bridge.waitForFinished(3000);
    }
    pty.kill();
    pty.waitForFinished(1000);
  }
  else
    qInfo() << "live browse skipped (no zeroconf or no bridge dir)";

  if (failed == 0)
    qInfo() << "All mDNS tests passed.";
  else
    qWarning() << failed << "mDNS test(s) failed.";
  return failed == 0 ? 0 : 1;
}
