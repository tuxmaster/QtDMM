#include "dmmdecoder.h"
#include "decoders.h"
#include "siprefix.h"

std::vector<DmmDecoder::DMMInfo> *DmmDecoder::m_configurations;

DmmDecoder::DmmDecoder(ReadEvent::DataFormat df)
  : m_type(df)
{}

void DmmDecoder::addConfig(DMMInfo info)
{
    if(!DmmDecoder::m_configurations)
        DmmDecoder::m_configurations = new std::vector<DMMInfo>();
  info.name = info.vendor + " " + info.model;
  DmmDecoder::m_configurations->push_back(info);
}

std::vector<DmmDecoder::DMMInfo> DmmDecoder::getDeviceConfigurations()
{
  if(!DmmDecoder::m_configurations)
    DmmDecoder::m_configurations = new std::vector<DMMInfo>();
  return *m_configurations;
}


QString DmmDecoder::insertComma(const QString &val, int pos)
{
  return val.left(2 + pos) + "." + val.right(4 - pos);
}


QString DmmDecoder::insertCommaIT(const QString &val, int pos)
{
  if (pos == 0)
    return val;

  if (val[0] == '-' || val[0] == ' ')
    return (val.left(pos + 1) + "." + val.mid(pos + 1));

  return (val.left(pos) + "." + val.mid(pos));
}

bool DmmDecoder::bit(const QByteArray &data, int byte, int bit) const {
  return data[byte] & (1 << bit);
}


double DmmDecoder::prefixFactor(const QString &prefix)
{
  return SiPrefix::factor(prefix);
}

void DmmDecoder::formatResultValue(int commaPos, const QString& prefix, const QString& baseUnit)
{
  m_result.val = insertCommaIT(m_result.val, commaPos);
  m_result.dval = m_result.val.toDouble() * prefixFactor(prefix);

  m_result.unit = prefix + baseUnit;
}

QString DmmDecoder::makeValue(const QByteArray &data, int first, int last, bool neg)
{
  QString val = neg ? "-" : "";

  for(int i=first; i<=last && i<data.size(); i++)
    val += data[i];

  return val;
}

QString DmmDecoder::toString() const {
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

// getInstance(): see protocols.cpp, the table has the factories




