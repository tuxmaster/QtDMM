// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
//
// AlarmManager: every condition, minimum duration, hysteresis, acknowledge,
// the no-readings watchdog, JSON round trip and the description text.

#include <QtCore>

#include "alarm.h"
#include "engnumbervalidator.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAIL:" << what;
    ++failed;
  }
}

struct Log
{
  QStringList events;
  Log(AlarmManager &m)
  {
    QObject::connect(&m, &AlarmManager::raised, [this](int i, const Alarm &a, double v) { events << QString("raised %1 %2 %3").arg(i).arg(a.name).arg(v); });
    QObject::connect(&m, &AlarmManager::cleared, [this](int i, const Alarm &a) { events << QString("cleared %1 %2").arg(i).arg(a.name); });
  }
};

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  // --- 1. conditions ---
  Alarm low; low.name = "low"; low.condition = Alarm::Below; low.a = 5;
  Alarm high; high.name = "high"; high.condition = Alarm::Above; high.a = 12;
  Alarm out; out.name = "out"; out.condition = Alarm::Outside; out.a = 11; out.b = 13;
  Alarm in; in.name = "in"; in.condition = Alarm::Inside; in.a = 1; in.b = 2;
  Alarm ol; ol.name = "ol"; ol.condition = Alarm::Overload;
  check(low.matches(4.9, false) && !low.matches(5, false) && !low.matches(4, true), "below");
  check(high.matches(12.1, false) && !high.matches(12, false), "above");
  check(out.matches(10.9, false) && out.matches(13.1, false) && !out.matches(12, false), "outside");
  check(in.matches(1.5, false) && !in.matches(2.5, false), "inside");
  check(ol.matches(0, true) && !ol.matches(0, false), "overload");
  Alarm swapped = out; swapped.a = 13; swapped.b = 11;
  check(swapped.matches(10.9, false) && !swapped.matches(12, false), "bounds in either order");

  // --- 2. raise at once, clear with hysteresis ---
  {
    AlarmManager m;
    Log log(m);
    Alarm h = low; h.hysteresis = 0.5;
    m.setAlarms({h});
    m.feed(6, false, 0);
    check(log.events.isEmpty() && m.state(0) == AlarmManager::Off, "6 V: off");
    m.feed(4.9, false, 1000);
    check(log.events == QStringList{"raised 0 low 4.9"} && m.state(0) == AlarmManager::Raised, "4.9 V: raised at once (no duration)");
    m.feed(5.2, false, 2000);
    check(log.events.size() == 1 && m.state(0) == AlarmManager::Raised, "5.2 V: still raised, inside the hysteresis band");
    m.feed(5.5, false, 3000);
    check(log.events.last() == "cleared 0 low" && m.state(0) == AlarmManager::Off, "5.5 V: cleared (5 + 0.5)");
    m.feed(4, false, 4000);
    check(log.events.size() == 3 && log.events.last().startsWith("raised"), "raises again");
  }

  // --- 3. minimum duration suppresses spikes ---
  {
    AlarmManager m;
    Log log(m);
    Alarm d = high; d.seconds = 2;
    m.setAlarms({d});
    m.feed(13, false, 0);
    check(m.state(0) == AlarmManager::Pending && log.events.isEmpty(), "pending, not raised");
    m.feed(13, false, 1500);
    check(log.events.isEmpty(), "1.5 s: still pending");
    m.feed(11, false, 1800);
    check(m.state(0) == AlarmManager::Off, "spike over: back to off");
    m.feed(13, false, 2000);
    m.feed(13, false, 4100);
    check(log.events == QStringList{"raised 0 high 13"}, "raised after 2 s of holding");
  }

  // --- 4. acknowledge: banner may go, state clears only with the condition ---
  {
    AlarmManager m;
    Log log(m);
    m.setAlarms({low});
    m.feed(1, false, 0);
    check(m.anyRaised(), "raised");
    m.acknowledge(0);
    check(m.state(0) == AlarmManager::Acknowledged && !m.anyRaised(), "acknowledged");
    m.feed(1, false, 1000);
    check(log.events.size() == 1, "no second raise while acknowledged and still low");
    m.feed(9, false, 2000);
    check(log.events.last() == "cleared 0 low", "cleared when the reading is back");
  }

  // --- 5. no readings ---
  {
    AlarmManager m;
    Log log(m);
    Alarm nr; nr.name = "silence"; nr.condition = Alarm::NoReadings; nr.seconds = 3;
    m.setAlarms({nr});
    m.feed(1, false, 0);
    m.tick(2000);
    check(log.events.isEmpty(), "2 s silence: nothing");
    m.tick(3100);
    check(log.events == QStringList{"raised 0 silence 1"}, "3 s silence: raised");
    m.tick(5000);
    check(log.events.size() == 1, "raised once");
    m.feed(2, false, 6000);
    check(log.events.last() == "cleared 0 silence", "a reading clears it");
  }

  // --- 5b. silence before the first reading: only a connected meter can be
  // silent. Without this the alarm raised on the first tick after the start
  // (epoch as the time base) and ran its actions - recorder, command.
  {
    AlarmManager m;
    Log log(m);
    Alarm nr; nr.name = "silence"; nr.condition = Alarm::NoReadings; nr.seconds = 60;
    m.setAlarms({nr});
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    m.tick(now);
    m.tick(now + 3600 * 1000);
    check(log.events.isEmpty(), "not connected, never a reading: no alarm");

    m.setConnected(true, now);
    m.tick(now + 59000);
    check(log.events.isEmpty(), "connected 59 s ago: not yet");
    m.tick(now + 60000);
    check(log.events == QStringList{"raised 0 silence 0"}, "connected 60 s ago without a reading: raised");

    m.setConnected(false, now + 61000);
    check(log.events.last() == "cleared 0 silence", "disconnecting clears it");
    m.tick(now + 7200 * 1000);
    check(log.events.size() == 2, "and it stays quiet while disconnected");

    // after reconnecting the clock starts again, and a reading takes over
    m.setConnected(true, now + 100000);
    m.feed(1, false, now + 110000);
    m.tick(now + 165000);
    check(log.events.size() == 2, "silence is measured from the last reading");
    m.tick(now + 171000);
    check(log.events.last() == "raised 0 silence 1", "60 s after that reading: raised");
  }

  // --- 6. disabled alarms and edits ---
  {
    AlarmManager m;
    Log log(m);
    Alarm off = low; off.enabled = false;
    m.setAlarms({off, high});
    m.feed(1, false, 0);
    check(log.events.isEmpty(), "disabled alarm does not raise");
    m.feed(20, false, 100);
    check(log.events == QStringList{"raised 1 high 20"}, "second alarm has index 1");
    Alarm high2 = high; high2.message = "edited";
    m.setAlarms({high2});
    check(m.state(0) == AlarmManager::Raised, "a raised alarm keeps its state through an edit (by name)");
    m.setAlarms({});
    check(!m.anyRaised(), "no alarms, nothing raised");
  }

  // --- 7. JSON round trip and description ---
  {
    Alarm a = out; a.seconds = 2.5; a.hysteresis = 0.1; a.message = "Voltage odd"; a.color = QColor("#123456");
    a.beep = false; a.popup = true; a.command = "notify-send %n %v%u"; a.recorder = Alarm::RecorderStart; a.markTable = false;
    const QString json = Alarm::listToJson({a, low});
    const QList<Alarm> back = Alarm::listFromJson(json);
    check(back.size() == 2, "two alarms back");
    if (back.size() == 2)
    {
      const Alarm &r = back[0];
      check(r.name == "out" && r.condition == Alarm::Outside && r.a == 11 && r.b == 13 && r.seconds == 2.5 && r.hysteresis == 0.1, "condition round trip");
      check(r.message == "Voltage odd" && r.color == QColor("#123456") && !r.beep && r.popup && r.command == "notify-send %n %v%u"
            && r.recorder == Alarm::RecorderStart && !r.markTable && r.markGraph, "actions round trip");
      check(back[1].condition == Alarm::Below && back[1].beep && back[1].banner, "defaults survive");
    }
    check(Alarm::listFromJson("not json").isEmpty(), "junk gives no alarms");
    check(low.describe("V") == "below 5V", "describe: " + low.describe("V"));
    check(a.describe("V") == "outside 11V to 13V for 2.5 s", "describe range+duration: " + a.describe("V"));
    Alarm nr; nr.condition = Alarm::NoReadings; nr.seconds = 10;
    check(nr.describe("V") == "no readings for 10 s", "describe silence: " + nr.describe("V"));
    Alarm milli = low; milli.a = 0.0047;
    check(milli.describe("A") == "below 4.7mA", "describe with SI prefix: " + milli.describe("A"));
    // a threshold keeps its digits: the dialog reads its fields back, and a
    // banner that says "below 4.8V" for an alarm raising at 4.7512 is a lie
    Alarm exact = low; exact.a = 4.7512;
    check(exact.describe("V") == "below 4.7512V", "describe keeps the digits: " + exact.describe("V"));
    Alarm ohms = low; ohms.a = 1234;
    check(ohms.describe("Ohm") == "below 1.234kOhm", "... with a prefix: " + ohms.describe("Ohm"));
    for (double v : {4.7512, 1234.0, 0.04712, -2.5e-6, 0.0})
      check(qFuzzyCompare(EngNumberValidator::value(EngNumberValidator::engText(v)) + 1.0, v + 1.0),
            QString("engText(%1) reads back as %2").arg(v).arg(EngNumberValidator::value(EngNumberValidator::engText(v))));
  }

  if (failed == 0)
    qInfo() << "All alarm tests passed.";
  else
    qWarning() << failed << "alarm test(s) failed.";
  return failed == 0 ? 0 : 1;
}
