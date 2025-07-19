#include "serial.h"
#include <QSerialPortInfo>

bool SerialDevice::availablePorts(QStringList &portlist)
{
  QString portName;
  for (auto port : QSerialPortInfo::availablePorts())
  {
    //qDebug() << port.portName() << "--" << port.manufacturer() << "--" << port.description() << "--" << port.systemLocation();
#ifdef Q_OS_WIN
    portName = port.portName();
#else
    portName = port.systemLocation();
#endif
    // some quick'n'dirty hack to get rid of all those useless ports
    // discard all ttyS above 9. they don't exist anyway
    if (portName.startsWith("/dev/ttyS") && portName.sliced(9).toInt()>9)
      continue;

    portlist << "SERIAL "+portName;
  }
  return !QSerialPortInfo::availablePorts().empty();
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
