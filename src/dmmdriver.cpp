#include "dmmdriver.h"

std::vector<DmmDriver::DMMInfo> DmmDriver::m_configurations;

void DmmDriver::addConfig(DMMInfo info)
{
  DmmDriver::m_configurations.push_back(info);
}


QString DmmDriver::insertComma(const QString &val, int pos)
{
  return val.left(2 + pos) + "." + val.right(4 - pos);
}

QString DmmDriver::insertCommaIT(const QString &val, int pos)
{
  if (pos == 0)
    return val;

  if (val[0] == '-' || val[0] == ' ')
    return (val.left(pos + 1) + "." + val.mid(pos + 1));

  return (val.left(pos) + "." + val.mid(pos));
}

bool DmmDriver::bit(const QByteArray &data, int byte, int bit) const {
  return data[byte] & (1 << bit);
}

void DmmDriver::formatResultValue(int commaPos, const QString& prefix, const QString& baseUnit)
{
    static const QHash<QString, double> scaleMap = {
    { "",     1.0 },
    { "k",    1e3 },
    { "M",    1e6 },
    { "G",    1e9 },
    { "m",    1e-3 },
    { "µ",    1e-6 },
    { "u",    1e-6 },  // optional: ASCII fallback
    { "n",    1e-9 }
  };

  m_result.val = insertCommaIT(m_result.val, commaPos);
  m_result.dval = m_result.val.toDouble() * scaleMap.value(prefix, 1.0);
  m_result.unit = prefix + baseUnit;
}

QString DmmDriver::toString() const {
  return QString("id=%1: %2 %3 (%4) [%5] range=%6 hold=%7 bar=%8")
                   .arg(m_result.id)
                   .arg(m_result.val)
                   .arg(m_result.unit)
                   .arg(m_result.dval)
                   .arg(m_result.special)
                   .arg(m_result.range)
                   .arg(m_result.hold)
                   .arg(m_result.showBar);
}
