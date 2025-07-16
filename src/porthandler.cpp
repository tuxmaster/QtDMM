#include "porthandler.h"

bool PortHandler::create(const DmmDriver::DMMInfo spec, PortType t, QString device)
{
  close();

  switch (t)
  {
    case PortType::Serial: m_port = new SerialDevice(spec, device, this); break;
    case PortType::Hid:    m_port = new HidDevice(spec, device, this);    break;
    //case PortType::Sigrok: m_port = new SigrokDevice(this);       break;
    default: return false;
  }

  m_type = t;
  m_device = device;
  emit opened(m_port);
  return true;
}

void PortHandler::close()
{
  if (!m_port) return;
  m_port->close();
  m_port->deleteLater();
  m_port = Q_NULLPTR;
  m_type = PortType::None;
  m_device = "";
  emit closed();
}

int PortHandler::error()
{
  switch (m_type)
  {
    case PortType::Serial: return static_cast<SerialDevice *>(m_port)->error();
    case PortType::Hid: return 0;
    case PortType::Sigrok: return 0;
  }
  return 0;
}

PortHandler::PortType PortHandler::str2portType(const QString str)
{
  if (str=="serial") return PortType::Serial;
  if (str=="usbhid") return PortType::Hid;
  if (str=="sigrok") return PortType::Sigrok;
  return PortType::None;
}

bool PortHandler::init()
{
  switch (m_type)
  {
    case PortType::Serial: return static_cast<SerialDevice *>(m_port)->init();
    //case PortType::Hid: return true;
    //case PortType::Sigrok: return ;
    default: return true;
  }
}
