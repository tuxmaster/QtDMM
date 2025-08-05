#include "serial.h"
#include <QSerialPortInfo>

static QStringList getExistingUartTtyS()
{
  QStringList result;
  QRegularExpression re("ttyS\\d+");

  QDirIterator it("/sys/devices/pnp0", QDirIterator::Subdirectories);
  while (it.hasNext()) {
    const QString path = it.next();
    const QFileInfo fi(path);
    const QString name = fi.fileName();

    if (re.match(name).hasMatch() && (fi.isDir() || fi.isSymLink())) {
      result << "/dev/" + name;
    }
  }

  return result;
}

bool SerialDevice::availablePorts(QStringList &portlist)
{
  QStringList validTtyS = getExistingUartTtyS();
  portlist.clear();

  for (const QSerialPortInfo &port : QSerialPortInfo::availablePorts()) {
    QString portName =
#ifdef Q_OS_WIN
      port.portName();
#else
      port.systemLocation();
#endif

#ifdef Q_OS_LINUX
    if (portName.startsWith("/dev/ttyS")) {
      if (!validTtyS.contains(portName))
        continue;
    }
#endif

    portlist << "SERIAL " + portName;
  }

  return !portlist.isEmpty();
}

bool SerialDevice::init()
{
  Parity parity = NoParity;
  switch (m_dmmInfo.parity)
  {
    case 0: parity = NoParity;   break;
    case 1: parity = EvenParity; break;
    case 2: parity = OddParity;  break;
  }

  return setParity(parity)
    && setBaudRate(m_dmmInfo.baud)
    && setStopBits(static_cast<StopBits>(m_dmmInfo.stopBits))
    && setDataBits(static_cast<DataBits>(m_dmmInfo.bits))
    && setDataTerminalReady(m_dmmInfo.dtr)
    && setRequestToSend(m_dmmInfo.rts);
}
