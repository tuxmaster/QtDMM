#include "dmmdecoder.h"
#include "decoders.h"

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


void DmmDecoder::formatResultValue(int commaPos, const QString& prefix, const QString& baseUnit)
{
    static const QHash<QString, double> scaleMap = {
    { "",     1.0 },
    { "k",    1e3 },
    { "M",    1e6 },
    { "G",    1e9 },
    { "m",    1e-3 },
    { "µ",    1e-6 },
    { "u",    1e-6 },  // optional: ASCII fallback
    { "n",    1e-9 },
    { "p",    1e-12 }
  };

  m_result.val = insertCommaIT(m_result.val, commaPos);
  m_result.dval = m_result.val.toDouble() * scaleMap.value(prefix, 1.0);
  m_result.unit = prefix + baseUnit;
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

std::shared_ptr<DmmDecoder> DmmDecoder::getInstance(ReadEvent::DataFormat df)
{
  switch (df)
  {
    case ReadEvent::Metex14:
    case ReadEvent::PeakTech10:
    case ReadEvent::Voltcraft14Continuous:
    case ReadEvent::Voltcraft15Continuous:  return std::make_unique<DecoderAscii>(df);
    case ReadEvent::M9803RContinuous:       return std::make_unique<DecoderM9803R>(df);
    case ReadEvent::VC820Continuous:        return std::make_unique<DecoderVC820>(df);
    case ReadEvent::VC870Continuous:        return std::make_unique<DecoderVC870>(df);
    case ReadEvent::IsoTech:                return std::make_unique<DecoderIsoTech>(df);
    case ReadEvent::VC940Continuous:        return std::make_unique<DecoderVC940>(df);
    case ReadEvent::QM1537Continuous:       return std::make_unique<DecoderQM1537>(df);
    case ReadEvent::RS22812Continuous:      return std::make_unique<DecoderRS22812>(df);
    case ReadEvent::DO3122Continuous:       return std::make_unique<DecoderDO3122>(df);
    case ReadEvent::CyrustekES51922:        return std::make_unique<DecoderCyrusTekES51922>(df);
    case ReadEvent::CyrustekES51962:        return std::make_unique<DecoderCyrusTekES51962>(df);
    case ReadEvent::CyrustekES51981:        return std::make_unique<DecoderCyrusTekES51981>(df);
    case ReadEvent::DTM0660:                return std::make_unique<DecoderDTM0660>(df);

    default: qWarning() << "invalid decoder ID"; return Q_NULLPTR;
  }
}

std::shared_ptr<DmmDecoder> DmmDecoder::getInstance(QString df)
{
    static const QMap<QString, ReadEvent::DataFormat> decoderMap = {
      { "Metex14",               ReadEvent::Metex14 },
      { "PeakTech10",            ReadEvent::PeakTech10 },
      { "Voltcraft14Continuous", ReadEvent::Voltcraft14Continuous },
      { "Voltcraft15Continuous", ReadEvent::Voltcraft15Continuous },
      { "M9803RContinuous",      ReadEvent::M9803RContinuous },
      { "VC820Continuous",       ReadEvent::VC820Continuous },
      { "IsoTech",               ReadEvent::IsoTech },
      { "VC940Continuous",       ReadEvent::VC940Continuous },
      { "QM1537Continuous",      ReadEvent::QM1537Continuous },
      { "RS22812Continuous",     ReadEvent::RS22812Continuous },
      { "VC870Continuous",       ReadEvent::VC870Continuous },
      { "DO3122Continuous",      ReadEvent::DO3122Continuous },
      { "CyrustekES51922",       ReadEvent::CyrustekES51922 },
      { "CyrustekES51962",       ReadEvent::CyrustekES51962 },
      { "DTM0660",               ReadEvent::DTM0660 },
      { "CyrustekES51981",       ReadEvent::CyrustekES51981 }
  };

  auto it = decoderMap.find(df);
  if (it != decoderMap.end())
      return DmmDecoder::getInstance(it.value());

  return Q_NULLPTR;
}




