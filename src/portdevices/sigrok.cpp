#include "sigrok.h"

#include <QThread>

//#define SIGROK_DEBUG

SigrokDevice::SigrokDevice(const DmmDecoder::DMMInfo &info, QString device, QObject *parent)
  : QIODevice(parent)
  , m_dmmInfo(info)
  , m_process(new QProcess(this))
  , m_device(device)
{
  m_sigrok = info.sigrokExe.isEmpty() ? "sigrok-cli" : info.sigrokExe;

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

  m_process->start(m_sigrok, args);
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
  // padds sigrok line to fixed length with spaces.
  // this way it is much easier and faster to decode afterwards
  constexpr int fixedLength = 30;
  if (m_buffer.isEmpty())
    return 0;

  int newlineIndex = m_buffer.indexOf('\n');
  if (newlineIndex < 0)
    return 0; // Not a complete line yet

  QByteArray line = m_buffer.left(newlineIndex);
  m_buffer.remove(0, newlineIndex + 1);

  line = line.trimmed();
  if (line.length() >= fixedLength - 1)
    line = line.left(fixedLength - 1);
  else
    line.prepend(QByteArray(fixedLength - 1 - line.length(), ' '));
  line.append('\n');

  qint64 len = qMin(maxSize, qint64(line.size()));
  memcpy(data, line.constData(), len);

#ifdef SIGROK_DEBUG
  qInfo() << QString::fromUtf8(data, len);
#endif

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
  Q_UNUSED(list);
  // use sigrok scan
  //list << "SIGROK uni-t-ut61e-ser:conn=/dev/ttyUSB0";
  return false;
}


void SigrokDevice::onProcessReadyRead() {
  m_buffer.append(m_process->readAllStandardOutput());
  Q_EMIT readyRead();
}


