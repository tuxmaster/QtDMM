#include "sigrok.h"

#include <QThread>


SigrokDevice::SigrokDevice(const DmmDecoder::DMMInfo &info, QString device, QObject *parent)
  : QIODevice(parent)
  , m_dmmInfo(info)
  , m_process(new QProcess(this))
{
  m_device = device;
  /*auto hp = device.split(':');
  if (hp.size() == 2)
  {
    m_type   = hp[0];
    m_device = hp[1];
  }
  */
  connect(m_process, &QProcess::readyReadStandardOutput, this, &SigrokDevice::onProcessReadyRead);
}

SigrokDevice::~SigrokDevice()
{
  close();
}


bool SigrokDevice::init() {
  if (m_process->state() != QProcess::NotRunning)
    return false;

  QStringList args;
  args << "--driver" << m_device << "--continuous";

  m_process->start("sigrok-cli", args);
  if (!m_process->waitForStarted())
    return false;

  open(QIODevice::ReadOnly);
  return true;
}


qint64 SigrokDevice::bytesAvailable() const
{
  return 0;
}

qint64 SigrokDevice::readData(char *data, qint64 maxSize)
{
  qint64 len = qMin(maxSize, qint64(m_buffer.size()));
  if (len > 0) {
    memcpy(data, m_buffer.constData(), len);
    m_buffer.remove(0, len);
    QString str = QString::fromUtf8(data, len);
    qInfo() << str;
  }
  return len;
}

qint64 SigrokDevice::writeData(const char *, qint64)
{
  return -1; // Read-only
}

void SigrokDevice::close()
{
  if (isOpen())
  {
    if (m_process->state() != QProcess::NotRunning)
    {
      m_process->terminate();
      if (!m_process->waitForFinished(2000))
      {
        m_process->kill();
        m_process->waitForFinished();
      }
    }

    m_buffer.clear();
    QIODevice::close();
  }
}

bool SigrokDevice::isOpen() const
{
  return m_process && m_process->isOpen() && (m_process->state() == QProcess::Running || m_process->state() == QProcess::Starting);
}

bool SigrokDevice::availablePorts(QStringList &list)
{
  // Optional: sigrok scan oder bekannte Geräte
  list << "SIGROK uni-t-ut61e-ser:conn=/dev/ttyUSB0";
  return true;
}


void SigrokDevice::onProcessReadyRead() {
  m_buffer.append(m_process->readAllStandardOutput());
  Q_EMIT readyRead();
}


