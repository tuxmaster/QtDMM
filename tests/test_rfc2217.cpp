// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
//
// RFC2217SerialDevice against an in-process fake server: the client must send
// the meter's line settings as COM-PORT-OPTION commands, hide the server's
// telnet replies from the decoder and un-/escape the 0xff byte.

#include <QtCore>
#include <QtNetwork>

#include "portdevices/rfc2217serial.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAIL:" << what;
    ++failed;
  }
}

static QByteArray sub(quint8 cmd, const QByteArray &payload)
{
  QByteArray p;
  p.append(char(0xFF)).append(char(0xFA)).append(char(44)).append(char(cmd));
  for (char c : payload)
  {
    p.append(c);
    if (quint8(c) == 0xFF) p.append(c);
  }
  return p.append(char(0xFF)).append(char(0xF0));
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  QTcpServer server;
  check(server.listen(QHostAddress::LocalHost, 0), "fake server listens");

  DmmDecoder::DMMInfo info;
  info.baud = 19200;
  info.bits = 7;
  info.parity = 1;     // even (DMMInfo: 0 none, 1 even, 2 odd)
  info.stopBits = 1;
  info.dtr = true;
  info.rts = false;

  RFC2217SerialDevice dev(info, QString("127.0.0.1:%1").arg(server.serverPort()));

  check(server.waitForNewConnection(2000), "client connects");
  QTcpSocket *peer = server.nextPendingConnection();
  check(peer != nullptr, "server has the connection");

  // --- 1. the negotiation QtDMM sends: baud 19200, 7 bits, even (RFC code 3), 1 stop, DTR on, RTS off
  QByteArray expected = sub(1, QByteArray::fromHex("00004b00")) + sub(2, QByteArray(1, 7)) + sub(3, QByteArray(1, 3))
                        + sub(4, QByteArray(1, 1)) + sub(5, QByteArray(1, 8)) + sub(5, QByteArray(1, 12));
  QByteArray got;
  QElapsedTimer t;
  t.start();
  while (got.size() < expected.size() && t.elapsed() < 2000)
  {
    app.processEvents(QEventLoop::AllEvents, 50);
    got += peer->readAll();
  }
  check(got == expected, "line settings sent as COM-PORT-OPTION suboptions: " + got.toHex(' '));

  // --- 2. the server's replies must not reach the decoder; IAC IAC is one 0xff;
  //        a WILL/DO negotiation is skipped too, and a split packet survives
  QByteArray reply = sub(101, QByteArray::fromHex("00004b00")) + sub(105, QByteArray(1, 8))
                     + QByteArray::fromHex("fffb2c")                         // IAC WILL COM-PORT
                     + QByteArray("DC 1.2") + QByteArray::fromHex("ffff") + QByteArray("34 V\n")
                     + sub(112, QByteArray::fromHex("ff"));                  // escaped 0xff inside a suboption
  peer->write(reply.left(9));
  peer->flush();
  app.processEvents(QEventLoop::AllEvents, 50);
  peer->write(reply.mid(9));
  peer->flush();
  t.restart();
  QByteArray data;
  while (t.elapsed() < 1000)
  {
    app.processEvents(QEventLoop::AllEvents, 50);
    data += dev.readAll();
    if (data.endsWith("V\n") && dev.bytesAvailable() == 0 && t.elapsed() > 200)
      break;
  }
  check(data == QByteArray("DC 1.2") + QByteArray::fromHex("ff") + QByteArray("34 V\n"),
        "telnet stripped, IAC IAC unescaped: " + data.toHex(' '));

  // --- 3. outgoing 0xff is doubled (poll requests of binary protocols)
  dev.write(QByteArray::fromHex("0086ff66"));
  t.restart();
  got.clear();
  while (got.size() < 5 && t.elapsed() < 1000)
  {
    app.processEvents(QEventLoop::AllEvents, 50);
    got += peer->readAll();
  }
  check(got == QByteArray::fromHex("0086ffff66"), "outgoing IAC escaped: " + got.toHex(' '));

  if (failed == 0)
    qInfo() << "All RFC 2217 client tests passed.";
  else
    qWarning() << failed << "RFC 2217 client test(s) failed.";
  return failed == 0 ? 0 : 1;
}
