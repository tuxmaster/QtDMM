// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
//
// DMM's connection state machine against a fake RFC 2217 server: Connecting
// until the first frame, Connected while frames arrive, Timeout when they
// stop, Error when the server goes away, reconnect when it is back, and a
// refused connection reported with its reason. Finally a polled,
// variable-length protocol (Fluke QM) through the reader.

#include <QtCore>
#include <QtNetwork>

#include "dmm.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAIL:" << what;
    ++failed;
  }
}

// Sigrok-style frame: 30 bytes, LF terminated, "<special> <value> <unit> <range>"
static QByteArray frame(const QString &value)
{
  QByteArray line = QString("DC %1 V AUTO").arg(value).toLatin1();
  line.prepend(QByteArray(29 - line.size(), ' '));
  return line + "\n";
}

static bool waitFor(const std::function<bool()> &cond, int ms)
{
  QElapsedTimer t;
  t.start();
  while (!cond() && t.elapsed() < ms)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
  return cond();
}

static const char *name(DMM::LinkState s)
{
  switch (s)
  {
    case DMM::LinkState::Closed: return "Closed";
    case DMM::LinkState::Connecting: return "Connecting";
    case DMM::LinkState::Connected: return "Connected";
    case DMM::LinkState::Timeout: return "Timeout";
    case DMM::LinkState::Error: return "Error";
  }
  return "?";
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  QTcpServer server;
  check(server.listen(QHostAddress::LocalHost, 0), "fake server listens");
  QTcpSocket *peer = nullptr;
  QObject::connect(&server, &QTcpServer::newConnection, [&] { peer = server.nextPendingConnection(); });

  DMM dmm(nullptr);
  DmmDecoder::DMMInfo info;
  info.baud = 9600;
  info.bits = 8;
  dmm.setDmmInfo(info);
  dmm.setFormat(ReadEvent::Sigrok);
  dmm.setTimeout(500);
  dmm.setReconnectInterval(1);

  QList<DMM::LinkState> states;
  QStringList messages;
  QObject::connect(&dmm, &DMM::linkStateChanged, [&](DMM::LinkState s, const QString &m)
  {
    states << s;
    messages << m;
  });
  int readings = 0;
  QObject::connect(&dmm, &DMM::value, [&] { ++readings; });

  // --- 1. open -> Connecting; first frame -> Connected ---
  dmm.setDevice(QString("RFC2217 127.0.0.1:%1").arg(server.serverPort()));
  check(dmm.open(), "open() succeeds");
  check(dmm.linkState() == DMM::LinkState::Connecting, "Connecting right after open()");
  check(waitFor([&] { return peer != nullptr; }, 2000), "client connected to the fake server");
  if (peer)
  {
    peer->readAll();   // the negotiation
    peer->write(frame("1.234"));
    peer->write(frame("1.235"));
    peer->flush();
  }
  check(waitFor([&] { return dmm.linkState() == DMM::LinkState::Connected; }, 2000),
        QString("Connected after the first frame (is %1)").arg(name(dmm.linkState())));
  check(readings >= 1, "reading emitted");
  check(dmm.errorString().startsWith("Connected"), "status text: " + dmm.errorString());

  // --- 2. frames stop -> Timeout; frames resume -> Connected ---
  check(waitFor([&] { return dmm.linkState() == DMM::LinkState::Timeout; }, 3000),
        QString("Timeout after %1 ms of silence (is %2)").arg(dmm.timeout()).arg(name(dmm.linkState())));
  check(dmm.errorString().contains("Timeout"), "timeout text: " + dmm.errorString());
  if (peer)
  {
    peer->write(frame("2.000"));
    peer->flush();
  }
  check(waitFor([&] { return dmm.linkState() == DMM::LinkState::Connected; }, 2000), "back to Connected");

  // --- 3. server drops the connection -> Error; server still there -> reconnect ---
  QTcpSocket *old = peer;
  peer = nullptr;
  if (old)
    old->disconnectFromHost();
  check(waitFor([&] { return dmm.linkState() == DMM::LinkState::Error; }, 3000),
        QString("Error after the server closed (is %1)").arg(name(dmm.linkState())));
  check(dmm.errorString().contains("Lost connection"), "loss text: " + dmm.errorString());
  check(waitFor([&] { return peer != nullptr; }, 4000), "reconnected within the retry interval");
  check(dmm.linkState() == DMM::LinkState::Connecting, "Connecting again after reconnect");
  if (peer)
  {
    peer->readAll();
    peer->write(frame("3.000"));
    peer->flush();
  }
  check(waitFor([&] { return dmm.linkState() == DMM::LinkState::Connected; }, 2000), "Connected after reconnect");

  // --- 4. close() -> Closed, no reconnect ---
  dmm.close();
  check(dmm.linkState() == DMM::LinkState::Closed, "Closed after close()");
  check(!dmm.isOpen(), "port closed");
  const int before = states.size();
  waitFor([] { return false; }, 1500);
  check(states.size() == before, "no state changes after close()");

  // --- 5. nobody listening -> Error with the socket's reason ---
  const quint16 deadPort = server.serverPort();
  server.close();
  DMM refused(nullptr);
  refused.setDmmInfo(info);
  refused.setFormat(ReadEvent::Sigrok);
  refused.setReconnectInterval(0);
  refused.setDevice(QString("RFC2217 127.0.0.1:%1").arg(deadPort));
  check(refused.open(), "open() itself succeeds (connect is asynchronous)");
  // Windows reports a refused loopback connect only after its SYN retries (~3 s)
  check(waitFor([&] { return refused.linkState() == DMM::LinkState::Error; }, 15000),
        QString("refused connection -> Error (is %1)").arg(name(refused.linkState())));
  check(!refused.errorString().isEmpty() && refused.errorString().contains("Lost connection"),
        "reason reported: " + refused.errorString());
  refused.close();

  // --- 6. malformed address fails open() at once ---
  DMM bad(nullptr);
  bad.setDmmInfo(info);
  bad.setFormat(ReadEvent::Sigrok);
  bad.setDevice("RFC2217 nonsense");
  check(!bad.open(), "open() fails for an address without port");
  check(bad.errorString().contains("host:port"), "malformed address text: " + bad.errorString());

  // --- 7. polled, variable-length protocol (Fluke QM) end to end ---
  // The reader must send the decoder's poll request and cut the answer -
  // CMD_ACK line plus reading line, arriving in pieces - into one frame.
  {
    QTcpServer fluke;
    check(fluke.listen(QHostAddress::LocalHost, 0), "fluke server listens");
    QTcpSocket *client = nullptr;
    QByteArray received;
    int polls = 0;
    QObject::connect(&fluke, &QTcpServer::newConnection, [&]
    {
      client = fluke.nextPendingConnection();
      QObject::connect(client, &QTcpSocket::readyRead, [&]
      {
        received += client->readAll();
        while (received.contains("QM\r"))
        {
          received.remove(0, received.indexOf("QM\r") + 3);
          ++polls;
          client->write("0\r-0.02");
          client->flush();
          QTimer::singleShot(50, client, [client] { client->write("3E-3,VDC,NORMAL,NONE\r"); client->flush(); });
        }
      });
    });

    DMM meter(nullptr);
    DmmDecoder::DMMInfo flukeInfo;
    flukeInfo.baud = 115200;
    flukeInfo.bits = 8;
    meter.setDmmInfo(flukeInfo);
    meter.setFormat(ReadEvent::FlukeQM);
    meter.setDevice(QString("RFC2217 127.0.0.1:%1").arg(fluke.serverPort()));
    double lastValue = 0;
    QString lastVal, lastUnit;
    QObject::connect(&meter, &DMM::value, [&](double dval, const QString &val, const QString &unit)
    {
      lastValue = dval;
      lastVal = val;
      lastUnit = unit;
    });
    check(meter.open(), "fluke open()");
    check(waitFor([&] { return polls >= 1; }, 4000), "poll request QM sent");
    check(waitFor([&] { return lastUnit == "mV"; }, 4000), "reading decoded from the split answer");
    check(lastVal == "-0.023" && qFuzzyCompare(lastValue + 1, -0.000023 + 1),
          "reading is -0.023 mV: " + lastVal + " " + lastUnit);
    check(meter.linkState() == DMM::LinkState::Connected, "fluke Connected");
    check(waitFor([&] { return polls >= 2; }, 4000), "polled again");
    meter.close();
  }

  if (failed == 0)
    qInfo() << "All DMM link state tests passed.";
  else
    qWarning() << failed << "DMM link state test(s) failed.";
  return failed == 0 ? 0 : 1;
}
