#include "porthandler.h"
#include "portdevices/serial.h"
#include "portdevices/hidserial.h"
#include "portdevices/rfc2217serial.h"
#include "portdevices/sigrok.h"
#include "portdevices/calc.h"
#ifdef QTDMM_WITH_BLE
#include "portdevices/ble.h"
#endif

#include <QSerialPortInfo>

bool PortHandler::create(const DmmDecoder::DMMInfo spec, PortType t, QString device)
{
  close();

  switch (t)
  {
    case PortType::Serial: m_port = new SerialDevice(spec, device);      break;
    case PortType::Hid:    m_port = new HIDSerialDevice(spec, device);   break;
    case PortType::Sigrok: m_port = new SigrokDevice(spec,device);       break;
    case PortType::RFC2217:m_port = new RFC2217SerialDevice(spec,device);break;
    case PortType::Calc:   m_port = new CalcDevice(spec, device, m_state); break;
#ifdef QTDMM_WITH_BLE
    case PortType::Ble:    m_port = new BleAdvertisementDevice(spec, device); break;
#endif
    default: return false;
  }

  m_type   = t;
  m_device = device;
  return true;
}

void PortHandler::close()
{
  if (!m_port) return;
  // forget the port before closing it: close() may emit signals whose
  // handlers ask port() and must not see the dying device
  QIODevice *port = m_port;
  m_port   = Q_NULLPTR;
  m_type   = PortType::None;
  m_device = "";
  port->close();
  port->deleteLater();
}

int PortHandler::error()
{
  switch (m_type)
  {
    case PortType::Serial: return static_cast<SerialDevice *>(m_port)->error();
    case PortType::Hid:    return 0;
    case PortType::Sigrok: return 0;
    case PortType::RFC2217:return 0;
    default:               return 0;
  }
}

PortHandler::PortType PortHandler::str2portType(const QString str)
{
  if (str.toLower() == "serial")  return PortType::Serial;
  if (str.toLower() == "hid")     return PortType::Hid;
  if (str.toLower() == "sigrok")  return PortType::Sigrok;
  if (str.toLower() == "rfc2217") return PortType::RFC2217;
  if (str.toLower() == "calc")    return PortType::Calc;
  if (str.toLower() == "ble")     return PortType::Ble;

  return PortType::None;
}

bool PortHandler::init()
{
  switch (m_type)
  {
    case PortType::Serial: return static_cast<SerialDevice *>(m_port)->init();
    case PortType::Hid:    return true;
    case PortType::Sigrok: return  static_cast<SigrokDevice *>(m_port)->init();
    default:               return true;
  }
}

QStringList PortHandler::availablePorts()
{
  QStringList portlist;
  SerialDevice::availablePorts(portlist);
  HIDSerialDevice::availablePorts(portlist);
  RFC2217SerialDevice::availablePorts(portlist);
  SigrokDevice::availablePorts(portlist);

  return portlist;
}
