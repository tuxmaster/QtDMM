// Tests for CalcDevice, the virtual meter that evaluates a formula over the
// other instances' readings: line format through the real ASCII decoder,
// missing/stale/invalid inputs, the hyphen/underscore alias, open() errors.
#include <QCoreApplication>
#include <QDateTime>
#include <QSharedMemory>
#include <QDebug>
#include <cmath>

#include "sharedstatemanager.h"
#include "portdevices/calc.h"
#include "dmmdecoder.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAILED:" << what;
    failed++;
  }
}

static SharedStateManager::Reading reading(double value, const QString &unit, qint64 now, bool valid = true)
{
  SharedStateManager::Reading r;
  r.value = value;
  r.unit = unit;
  r.special = "DC";
  r.msecs = now;
  r.valid = valid;
  return r;
}

// runs the 30-byte line through DecoderAscii exactly like ReaderThread does
static std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &line)
{
  auto decoder = DmmDecoder::getInstance(ReadEvent::Sigrok);
  if (!decoder || line.size() != int(decoder->getPacketLength()))
    return std::nullopt;
  if (!decoder->checkFormat(line.constData(), line.size() - 1))
    return std::nullopt;
  return decoder->decode(line, 0);
}

int main(int argc, char **argv)
{
  const QByteArray key = "qtdmm_test_calc_" + QByteArray::number(QCoreApplication::applicationPid());
  qputenv("QTDMM_IPC_KEY", key);
  QCoreApplication app(argc, argv);
  app.setApplicationName("qtdmm_test");
  {
    QSharedMemory leftover(QString::fromLatin1(key));
    if (leftover.attach())
      leftover.detach();
  }

  SharedStateManager u("u"), i("i"), p("p"), odd("uni-t_803");
  check(u.registerInstance() && i.registerInstance() && p.registerInstance() && odd.registerInstance(), "all register");

  DmmDecoder::DMMInfo info{};
  info.protocol = ReadEvent::Sigrok;
  info.display = 40000;
  const qint64 now = QDateTime::currentMSecsSinceEpoch();

  // --- 1. the device registered itself as a model ---
  {
    bool found = false;
    for (const auto &cfg : DmmDecoder::getDeviceConfigurations())
      if (cfg.vendor == "QtDMM" && cfg.model == "Calculated value" && cfg.protocol == ReadEvent::Sigrok)
        found = true;
    check(found, "QtDMM / Calculated value is a registered model with the Sigrok protocol");
  }

  // --- 2. open() rejects bad input with a message ---
  {
    CalcDevice bad(info, "W u * ", &p);
    check(!bad.open(QIODevice::ReadWrite), "unparsable formula does not open");
    check(bad.errorString().contains("osition"), "error names the position: " + bad.errorString());
    CalcDevice noUnit(info, "", &p);
    check(!noUnit.open(QIODevice::ReadWrite), "empty device string does not open");
    CalcDevice noState(info, "W u", nullptr);
    check(!noState.open(QIODevice::ReadWrite), "no state manager does not open");
  }

  // --- 3. P = U * I through the decoder ---
  CalcDevice calc(info, "W u * i", &p);
  check(calc.open(QIODevice::ReadWrite), "formula opens: " + calc.errorString());
  check(calc.unit() == "W" && calc.formula() == "u * i", "unit and formula split off the device string");

  QString status;
  QByteArray line = calc.currentLine(now, &status);
  check(line.size() == 30 && line.endsWith('\n'), "line is 30 bytes ending in LF");
  check(status.contains("'u'") || status.contains("'i'"), "nothing published yet -> waiting message: " + status);
  auto r = decode(line);
  check(r && r->val.trimmed() == "OL", "no inputs -> OL");

  u.publishReading(reading(12.0, "V", now));
  i.publishReading(reading(0.5, "A", now));
  p.checkForChanges();
  line = calc.currentLine(now, &status);
  r = decode(line);
  check(status.isEmpty(), "all inputs present -> no status message, got: " + status);
  check(r.has_value(), "line decodes: " + QString::fromLatin1(line.trimmed()));
  if (r)
  {
    check(qFuzzyCompare(r->dval, 6.0), QString("dval %1, expected 6").arg(r->dval));
    check(r->unit == "W", "unit W, got " + r->unit);
    check(r->special == "DC" && r->range == "AUTO", "coupling DC, range AUTO");
  }

  // small results get an SI prefix and still decode to base units
  i.publishReading(reading(0.0005, "A", now + 1));
  p.checkForChanges();
  r = decode(calc.currentLine(now, nullptr));
  check(r && std::fabs(r->dval - 0.006) < 1e-9 && r->unit == "mW", QString("6 mW: dval %1 unit %2").arg(r ? r->dval : -1).arg(r ? r->unit : "-"));

  // --- 4. stale and invalid inputs ---
  r = decode(calc.currentLine(now + CalcDevice::kStaleMs + 1, &status));
  check(r && r->val.trimmed() == "OL" && status.contains("No current value"), "stale input -> OL + message: " + status);

  i.publishReading(reading(0, "A", now + 2, false));
  p.checkForChanges();
  r = decode(calc.currentLine(now + 2, &status));
  check(r && r->val.trimmed() == "OL" && status.contains("'i'") && status.contains("numeric"), "overloaded input -> OL + message: " + status);

  // --- 5. division by zero, and an instance id with a hyphen ---
  i.publishReading(reading(0.0, "A", now + 3, true));
  p.checkForChanges();
  CalcDevice div(info, "Ohm u / i", &p);
  check(div.open(QIODevice::ReadWrite), "u / i opens");
  r = decode(div.currentLine(now + 3, &status));
  check(r && r->val.trimmed() == "OL" && status.contains("division"), "1/0 -> OL + message: " + status);

  odd.publishReading(reading(3.0, "V", now + 3));
  p.checkForChanges();
  CalcDevice alias(info, "V uni_t_803 * 2", &p);
  check(alias.open(QIODevice::ReadWrite), "aliased id opens");
  r = decode(alias.currentLine(now + 3, &status));
  check(r && qFuzzyCompare(r->dval, 6.0) && status.isEmpty(), "uni-t_803 is reachable as uni_t_803");

  // --- 5a. rounding to the display's digits ---
  {
    QString pre;
    check(CalcDevice::formatValue(6.6242363, 40000, &pre) == "6.6242" && pre.isEmpty(), "5 digits: 6.6242");
    check(CalcDevice::formatValue(9.88170524, 40000, &pre) == "9.8817", "5 digits: 9.8817");
    check(CalcDevice::formatValue(123.456789, 40000, &pre) == "123.46", "5 digits: 123.46");
    check(CalcDevice::formatValue(0.0012345678, 40000, &pre) == "1.2346" && pre == "m", "prefix then round: 1.2346 m");
    check(CalcDevice::formatValue(6.0, 400000, &pre) == "6.00000", "6 digits: 6.00000");
    check(CalcDevice::formatValue(0.0, 40000, &pre) == "0.0000", "zero keeps decimals");
    check(CalcDevice::formatValue(-2.5, 4000, &pre) == "-2.500", "negative, 4 digits");
  }

  // --- 5b. time and coupling: a signal generator needs no other instance ---
  {
    CalcDevice gen(info, "V/AC 10 + t", &p);
    check(gen.open(QIODevice::ReadWrite), "generator opens");
    check(gen.unit() == "V" && gen.coupling() == "AC", "unit/coupling split: " + gen.unit() + "/" + gen.coupling());
    // t counts from open(); currentLine() takes the reference time
    const qint64 opened = QDateTime::currentMSecsSinceEpoch();
    r = decode(gen.currentLine(opened + 2500, &status));
    check(r && status.isEmpty() && r->special == "AC", "AC coupling in the line");
    check(r && std::fabs(r->dval - 12.5) < 0.01, QString("t = 2.5 s -> 12.5, got %1").arg(r ? r->dval : -1));
    CalcDevice rnd(info, "V 5 + rand()", &p);
    check(rnd.open(QIODevice::ReadWrite), "rand() opens");
    r = decode(rnd.currentLine(opened, nullptr));
    check(r && r->dval >= 5.0 && r->dval < 6.0, "rand() based reading in range");
  }

  // --- 6. the QIODevice side: tick() offers the line, readData hands it out bytewise ---
  {
    int emitted = 0;
    QObject::connect(&calc, &QIODevice::readyRead, [&]{ emitted++; });
    QString lastStatus = "unset";
    QObject::connect(&calc, &CalcDevice::status, [&](const QString &s){ lastStatus = s; });
    // wait for two polls
    QDateTime until = QDateTime::currentDateTime().addMSecs(CalcDevice::kIntervalMs * 2 + 100);
    while (QDateTime::currentDateTime() < until)
      app.processEvents();
    check(emitted >= 1, "readyRead after a tick");
    check(calc.bytesAvailable() == 30, QString("30 bytes available, got %1").arg(calc.bytesAvailable()));
    QByteArray got;
    char c;
    while (calc.read(&c, 1) == 1)
      got.append(c);
    check(got.size() == 30 && got.endsWith('\n'), "bytewise read returns the whole line");
    check(lastStatus != "unset", "status() was emitted");
    calc.close();
    check(!calc.isOpen() && calc.bytesAvailable() == 0, "close clears");
  }

  if (failed == 0)
    qInfo() << "All calc device tests passed.";
  else
    qWarning() << failed << "calc device test(s) failed.";
  return failed == 0 ? 0 : 1;
}
