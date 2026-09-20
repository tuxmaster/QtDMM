#include "portdevices/calc.h"
#include "sharedstatemanager.h"
#include "siprefix.h"

#include <QCoreApplication>
#include <QDateTime>
#include <cmath>

// Registered like a meter so it appears in the model list (vendor "QtDMM")
// and in the supported-devices table. Protocol Sigrok: the ASCII decoder
// reads the lines this device produces.
static const bool registered = []()
{
  DmmDecoder::addConfig({"QtDMM", "Calculated value", "", 0, ReadEvent::Sigrok, 8, 1, 1, 0, 400000, 0, 0, 0});
  DmmDecoder::addConfig({"QtDMM", "Virtual meter", "", 0, ReadEvent::Sigrok, 8, 1, 1, 0, 40000, 0, 0, 0});
  return true;
}();

CalcDevice::CalcDevice(const DmmDecoder::DMMInfo &info, const QString &device,
                       SharedStateManager *state, QObject *parent)
  : QIODevice(parent),
    m_dmmInfo(info),
    m_state(state)
{
  // "<unit> <formula>": the unit has no spaces, the formula may
  const QString d = device.trimmed();
  const int space = d.indexOf(' ');
  m_unit = space < 0 ? d : d.left(space);
  m_source = space < 0 ? QString() : d.mid(space + 1).trimmed();
  // "V/AC": coupling rides along with the unit token
  const int slash = m_unit.indexOf('/');
  if (slash >= 0)
  {
    const QString c = m_unit.mid(slash + 1).toUpper();
    m_unit = m_unit.left(slash);
    if (c == "AC" || c == "DC")
      m_special = c;
  }

  m_timer.setInterval(kIntervalMs);
  connect(&m_timer, &QTimer::timeout, this, &CalcDevice::tick);
}

bool CalcDevice::availablePorts(QStringList &portlist)
{
  Q_UNUSED(portlist);
  return false;
}

bool CalcDevice::open(OpenMode mode)
{
  if (m_unit.isEmpty())
  {
    setErrorString(QCoreApplication::translate("CalcDevice", "No unit given for the calculated value."));
    return false;
  }
  QString error;
  int pos = -1;
  m_expr = CalcExpr::parse(m_source, &error, &pos);
  if (!m_expr)
  {
    setErrorString(QCoreApplication::translate("CalcDevice", "Formula error at position %1: %2").arg(pos + 1).arg(error));
    return false;
  }
  if (!m_state)
  {
    setErrorString(QCoreApplication::translate("CalcDevice", "No instance data available."));
    return false;
  }

  if (!QIODevice::open(mode & ~WriteOnly))
    return false;
  m_pending.clear();
  m_lastStatus.clear();
  m_statusSent = false;
  m_openedMs = QDateTime::currentMSecsSinceEpoch();
  m_timer.start();
  return true;
}

void CalcDevice::close()
{
  m_timer.stop();
  m_pending.clear();
  if (QIODevice::isOpen())
    QIODevice::close();
}

qint64 CalcDevice::bytesAvailable() const
{
  return m_pending.size() + QIODevice::bytesAvailable();
}

QByteArray CalcDevice::currentLine(qint64 now, QString *status) const
{
  QString message;
  QByteArray line;

  if (!m_expr || !m_state)
    line = m_special.toUtf8() + " inf " + m_unit.toUtf8();
  else
  {
    // instance ids as variables; "uni-t_803" is reachable as uni_t_803
    const auto readings = m_state->readings();
    QMap<QString, double> values;
    values.insert("t", (now - m_openedMs) / 1000.0);
    QStringList missing, stale, invalid;
    for (const QString &var : m_expr->variables())
    {
      if (var == "t")
        continue;
      QString id = var;
      if (!readings.contains(id))
      {
        for (auto it = readings.constBegin(); it != readings.constEnd(); ++it)
          if (QString(it.key()).replace('-', '_') == var)
            id = it.key();
      }
      if (!readings.contains(id))
      {
        missing << var;
        continue;
      }
      const SharedStateManager::Reading &r = readings[id];
      if (now - r.msecs > kStaleMs)
        stale << var;
      else if (!r.valid)
        invalid << var;
      else
        values.insert(var, r.value);
    }

    std::optional<double> result;
    if (missing.isEmpty() && stale.isEmpty() && invalid.isEmpty())
      result = m_expr->eval(values);

    if (!missing.isEmpty())
      message = QCoreApplication::translate("CalcDevice", "Waiting for instance '%1'").arg(missing.join("', '"));
    else if (!stale.isEmpty())
      message = QCoreApplication::translate("CalcDevice", "No current value from instance '%1'").arg(stale.join("', '"));
    else if (!invalid.isEmpty())
      message = QCoreApplication::translate("CalcDevice", "Instance '%1' shows no numeric value").arg(invalid.join("', '"));
    else if (!result)
      message = QCoreApplication::translate("CalcDevice", "Formula has no result (division by zero?)");

    if (result)
    {
      QString prefix;
      const QString value = formatValue(*result, m_dmmInfo.display, &prefix);
      line = m_special.toUtf8() + " " + value.toUtf8() + " " + (prefix + m_unit).toUtf8() + " AUTO";
    }
    else
      line = m_special.toUtf8() + " inf " + m_unit.toUtf8() + " AUTO";
  }

  // fixed length, like SigrokDevice: the decoder's frame is 30 bytes ending in LF
  if (line.size() >= kLineLength - 1)
    line = line.left(kLineLength - 1);
  else
    line.prepend(QByteArray(kLineLength - 1 - line.size(), ' '));
  line.append('\n');

  if (status)
    *status = message;
  return line;
}

QString CalcDevice::formatValue(double value, int counts, QString *prefix)
{
  const double scaled = SiPrefix::scale(value, prefix);
  // a 40000 count display has 5 digits; keep that many significant digits
  int digits = 0;
  for (int c = qMax(1, counts); c > 0; c /= 10)
    digits++;
  digits = qBound(3, digits, 9);
  const double mag = scaled == 0.0 ? 0.0 : std::floor(std::log10(std::fabs(scaled)));
  const int decimals = qBound(0, digits - 1 - static_cast<int>(mag), 9);
  return QString::number(scaled, 'f', decimals);
}

void CalcDevice::tick()
{
  QString message;
  m_pending = currentLine(QDateTime::currentMSecsSinceEpoch(), &message);
  if (!m_statusSent || message != m_lastStatus)
  {
    m_statusSent = true;
    m_lastStatus = message;
    Q_EMIT status(message);
  }
  Q_EMIT readyRead();
}

qint64 CalcDevice::readData(char *data, qint64 maxSize)
{
  const qint64 len = qMin(maxSize, qint64(m_pending.size()));
  if (len <= 0)
    return 0;
  memcpy(data, m_pending.constData(), len);
  m_pending.remove(0, len);
  return len;
}

qint64 CalcDevice::writeData(const char *, qint64)
{
  return -1;
}
