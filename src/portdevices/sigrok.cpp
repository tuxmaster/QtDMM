#include "sigrok.h"

#include <QThread>


SigrokDevice::SigrokDevice(const DmmDecoder::DMMInfo &info, QString device, QObject *parent)
  : QIODevice(parent), m_dmmInfo(info)
{

  auto hp = device.split(':');
  if (hp.size() == 2)
  {
    m_type   = hp[0];
    m_device = hp[1];
  }

}

SigrokDevice::~SigrokDevice()
{
}

qint64 SigrokDevice::bytesAvailable() const
{
  return 0;
}

qint64 SigrokDevice::readData(char *data, qint64 maxSize)
{
  return 0;
}

qint64 SigrokDevice::writeData(const char *, qint64)
{
  return -1; // Read-only
}

void SigrokDevice::close()
{
  if (m_isOpen)
  {
  }
}

bool SigrokDevice::availablePorts(QStringList &list)
{
  // Optional: sigrok scan oder bekannte Geräte
  list << "uni-t-ut61e-ser:/dev/ttyUSB1";
  return true;
}
