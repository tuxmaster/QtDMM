// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
//
// ScpiServer: the command interpreter (process()) on program messages the
// way lxi-tools, sigrok's scpi-dmm and hand-written scripts send them, then
// the same over a real TCP connection on localhost, and MdnsResponder's
// answer to a DNS-SD question round-tripped through MdnsMessage::parse.

#include <QtCore>
#include <QtNetwork>
#include <QtTest>

#include "mdnsbrowser.h"
#include "mdnsresponder.h"
#include "scpiserver.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAIL:" << what;
    ++failed;
  }
}

static QString ask(ScpiServer &s, const char *msg)
{
  return QString::fromUtf8(s.process(msg)).trimmed();
}

static ScpiServer::Reading reading(double v, const QString &unit, const QString &special = "DC")
{
  ScpiServer::Reading r;
  r.value = v;
  r.unit = unit;
  r.special = special;
  r.range = "AUTO";
  r.valid = true;
  r.msecs = QDateTime::currentMSecsSinceEpoch();
  return r;
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);
  ScpiServer s;
  s.setModel("UNI-T UT61E");

  // --- 1. common commands ---
  check(ask(s, "*IDN?") == QString("QtDMM,UNI-T UT61E,0,%1").arg(APP_VERSION), "*IDN? " + ask(s, "*IDN?"));
  check(ask(s, "*idn?") == ask(s, "*IDN?"), "case does not matter");
  check(ask(s, "*OPC?") == "1" && ask(s, "*TST?") == "0", "*OPC? *TST?");
  check(s.process("*CLS").isEmpty(), "a command without query answers nothing");
  check(ask(s, "SYST:ERR?") == "0,\"No error\"", "error queue empty");

  // --- 2. readings ---
  check(ask(s, "READ?") == "9.91E+37", "READ? without meter: NaN " + ask(s, "READ?"));
  check(ask(s, "SYSTem:ERRor?").startsWith("-230,"), "... and -230 stale in the queue");
  check(ask(s, "SYST:ERR?").startsWith("-230,"), "(one per query)");
  check(ask(s, "SYST:ERR?") == "0,\"No error\"", "queue drained");

  s.setConnected(true);
  s.setReading(0, reading(1.2345, "V"));
  check(ask(s, "READ?") == "+1.234500E+00", "READ? " + ask(s, "READ?"));
  check(ask(s, "FETC?") == "+1.234500E+00" && ask(s, "MEAS?") == "+1.234500E+00"
        && ask(s, "MEASure:VOLTage:DC?") == "+1.234500E+00" && ask(s, "SENS:DATA?") == "+1.234500E+00"
        && ask(s, "VAL1?") == "+1.234500E+00", "FETCh? MEASure? MEAS:VOLT:DC? SENS:DATA? VAL1?");
  check(ask(s, "UNIT?") == "\"V\"", "UNIT? " + ask(s, "UNIT?"));
  check(ask(s, "CONF?") == "\"VOLT:DC AUTO\"", "CONF? " + ask(s, "CONF?"));
  check(ask(s, "READ2?") == "9.91E+37", "second value not there yet");
  s.setReading(1, reading(0.05, "A", "AC"));
  check(ask(s, "READ2?") == "+5.000000E-02" && ask(s, "VAL2?") == "+5.000000E-02", "READ2? VAL2?");
  check(ask(s, "UNIT2?") == "\"A\"" && ask(s, "CONF2?") == "\"CURR:AC AUTO\"", "UNIT2? CONF2?");
  s.process("*CLS");
  check(ask(s, "READ3?").isEmpty() && ask(s, "SYST:ERR?").startsWith("-114,"), "READ3? is out of range");
  s.process("*CLS");

  ScpiServer::Reading ol = reading(0, "Ohm");
  ol.overload = true;
  s.setReading(0, ol);
  check(ask(s, "READ?") == "9.9E+37", "overload is +INF " + ask(s, "READ?"));
  check(ask(s, "CONF?") == "\"RES AUTO\"", "resistance");
  check(ask(s, "STAT:QUES?") == "1", "questionable: overload bit " + ask(s, "STAT:QUES?"));
  ScpiServer::Reading old = reading(3.3, "V");
  old.msecs -= ScpiServer::kStaleMs + 1;
  s.setReading(0, old);
  check(ask(s, "READ?") == "9.91E+37" && ask(s, "STAT:QUES:COND?") == "8", "a stale reading is NaN, bit 3");
  s.process("*CLS");
  s.setReading(0, reading(-2.5e-3, "V"));
  check(ask(s, "READ?") == "-2.500000E-03", "negative, small");

  // --- 3. several commands in one message, the prefix rule ---
  check(ask(s, "*IDN?;READ?;UNIT?") == QString("QtDMM,UNI-T UT61E,0,%1;-2.500000E-03;\"V\"").arg(APP_VERSION),
        "three queries, one line " + ask(s, "*IDN?;READ?;UNIT?"));
  check(ask(s, "SYST:ERR?;VERS?") == "0,\"No error\";1999.0", "second unit continues at SYST: " + ask(s, "SYST:ERR?;VERS?"));
  check(ask(s, ":SYST:ERR?;:READ?") == "0,\"No error\";-2.500000E-03", "absolute headers");
  check(s.process("*RST;*CLS").isEmpty(), "two commands, no query, no answer");
  check(ask(s, "FOO?").isEmpty() && ask(s, "SYST:ERR?").startsWith("-113,"), "unknown header -> -113");
  check(s.process("READ").isEmpty() && ask(s, "SYST:ERR?").startsWith("-100,"), "READ without ? -> -100");
  check(s.process("").isEmpty() && s.process("   ").isEmpty(), "empty message");

  // --- 4. recorder and connection through signals ---
  int starts = 0, stops = 0;
  QList<bool> connects;
  QObject::connect(&s, &ScpiServer::startRecording, [&] { ++starts; });
  QObject::connect(&s, &ScpiServer::stopRecording, [&] { ++stops; });
  QObject::connect(&s, &ScpiServer::connectRequested, [&](bool on) { connects << on; });
  s.process("INIT");
  s.process("INITiate:IMMediate");
  s.process("ABOR");
  check(starts == 2 && stops == 1, "INIT/ABOR");
  s.setRecording(true);
  check(ask(s, "STAT:OPER?") == "16", "operation: measuring while recording");
  s.process("INP OFF");
  s.process("INPut:STATe 1");
  s.process("INP");
  check(connects == QList<bool>({false, true}) && ask(s, "SYST:ERR?").startsWith("-109,"), "INPut ON/OFF, missing parameter");
  check(ask(s, "INP?") == "1", "INP? reports connected");
  s.process("*CLS");

  // error queue overflow
  for (int i = 0; i < 25; ++i)
    s.process("NOPE");
  check(ask(s, "SYST:ERR:COUN?") == "20", "queue holds 20");
  QString last;
  for (int i = 0; i < 20; ++i)
    last = ask(s, "SYST:ERR?");
  check(last.startsWith("-350,"), "last entry is the overflow " + last);

  // --- 5. over TCP ---
  check(s.start(QHostAddress::LocalHost, 55025), "listens on localhost " + s.errorString());
  ScpiServer second;
  check(second.start(QHostAddress::LocalHost, 55025) && second.port() == 55026, "second instance moves to the next port");
  second.stop();
  {
    QTcpSocket c;
    c.connectToHost(QHostAddress::LocalHost, s.port());
    check(c.waitForConnected(2000), "client connects");
    c.write("*IDN?\r\nREAD?\n");
    // server and client share this thread: let the event loop run
    QByteArray got;
    for (int i = 0; i < 50 && !got.contains("E-03\n"); ++i)
    {
      QTest::qWait(20);
      got += c.readAll();
    }
    check(got == QString("QtDMM,UNI-T UT61E,0,%1\n-2.500000E-03\n").arg(APP_VERSION).toUtf8(),
          "answers over the socket: " + QString::fromUtf8(got));
    c.write("*CLS");   // no terminator: nothing happens
    c.waitForBytesWritten(500);
    QTest::qWait(100);
    check(s.clientCount() == 1, "one client");
    c.disconnectFromHost();
    if (c.state() != QAbstractSocket::UnconnectedState)
      c.waitForDisconnected(1000);
    QTest::qWait(100);
    check(s.clientCount() == 0, "client gone");
  }
  s.stop();

  // --- 6. mDNS responder: the answer to a browse ---
  MdnsResponder responder;
  // not started: build the packets only. The name parts are private, so
  // exercise start() on a machine with a network and fall back otherwise.
  QMap<QString, QString> txt;
  txt["model"] = "UNI-T UT61E";
  const bool live = responder.start("_scpi-raw._tcp", "QtDMM test", 5025, txt);
  if (live)
  {
    const QByteArray q = MdnsMessage::query("_scpi-raw._tcp.local");
    const QByteArray a = responder.answer(q, QHostAddress("192.168.1.20"));
    const QList<MdnsMessage::Record> records = MdnsMessage::parse(a);
    check(records.size() == 4, QString("PTR, SRV, TXT, A (%1)").arg(records.size()));
    MdnsBrowser browser;
    // the browser only assembles what it asked for
    browser.browse("_scpi-raw._tcp.local", 10);
    browser.handlePacket(a);
    browser.stop();
    check(browser.services().size() == 1, "the browser sees one service");
    if (!browser.services().isEmpty())
    {
      const MdnsBrowser::Service svc = browser.services().first();
      check(svc.instance == "QtDMM test" && svc.port == 5025 && svc.host == responder.hostName().toLower()
            && svc.address == QHostAddress("192.168.1.20") && svc.txt.value("model") == "UNI-T UT61E",
            "instance, port, host, address, txt: " + svc.instance + " " + svc.host);
    }
    check(responder.answer(MdnsMessage::query("_other._tcp.local"), QHostAddress("192.168.1.20")).isEmpty(),
          "no answer for another service");
    const QList<MdnsMessage::Record> enumeration =
      MdnsMessage::parse(responder.answer(MdnsMessage::query("_services._dns-sd._udp.local"), QHostAddress("192.168.1.20")));
    check(enumeration.size() == 1 && enumeration.first().target == "_scpi-raw._tcp.local", "service enumeration");
    // the goodbye carries ttl 0
    const QList<MdnsMessage::Record> bye = MdnsMessage::parse(responder.answer(q, QHostAddress("192.168.1.20"), 0));
    check(bye.size() == 4 && bye.first().ttl == 0, "goodbye packet");
    responder.stop();
  }
  else
    qInfo() << "no multicast interface, responder not exercised";

  if (failed)
    qWarning() << failed << "check(s) failed";
  else
    qInfo() << "all checks passed";
  return failed ? 1 : 0;
}
