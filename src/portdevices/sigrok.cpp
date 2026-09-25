#include "sigrok.h"

#include <QThread>

//#define SIGROK_DEBUG

SigrokDevice::SigrokDevice(const DmmDecoder::DMMInfo &info, QString device, QObject *parent)
  : QIODevice(parent)
  , m_dmmInfo(info)
  , m_device(device)
  , m_process(new QProcess(this))
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
  qint64 avail = m_outLine.size();
  if (m_buffer.indexOf('\n') >= 0)
    avail += m_fixedLineLength;
  return avail;
}


qint64 SigrokDevice::readData(char *data, qint64 maxSize)
{
  // pads sigrok line to fixed length with spaces.
  // this way it is much easier and faster to decode afterwards.
  // m_outLine holds the not-yet-delivered remainder of the current padded
  // line, since callers (ReaderThread) may read as little as one byte at a time.
  if (m_outLine.isEmpty())
  {
    int newlineIndex = m_buffer.indexOf('\n');
    if (newlineIndex < 0)
      return 0; // Not a complete line yet

    QByteArray line = m_buffer.left(newlineIndex);
    m_buffer.remove(0, newlineIndex + 1);

    line = line.trimmed();
    if (line.length() >= m_fixedLineLength - 1)
      line = line.left(m_fixedLineLength - 1);
    else
      line.prepend(QByteArray(m_fixedLineLength - 1 - line.length(), ' '));
    line.append('\n');

    m_outLine = line;
  }

  qint64 len = qMin(maxSize, qint64(m_outLine.size()));
  memcpy(data, m_outLine.constData(), len);
  m_outLine.remove(0, len);

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
  // Keyed off the process state rather than isOpen(), so a process that was
  // started by init() but never reached QIODevice::open() still gets reaped.
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
  m_outLine.clear();

  if (QIODevice::isOpen())
    QIODevice::close();
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


