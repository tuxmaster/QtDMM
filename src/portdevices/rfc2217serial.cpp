#include "rfc2217serial.h"

#include <QDebug>

RFC2217SerialDevice::RFC2217SerialDevice(const DmmDecoder::DMMInfo &info, QString device, QObject *parent)
  : QIODevice{parent}
  , m_dmmInfo(info)
{
  auto hp = device.split(':');
  if (hp.size() == 2)
  {
    m_host = hp[0];
    m_port = hp[1].toUShort();
  }

  m_socket = new QTcpSocket(this);
  connect(m_socket, &QTcpSocket::readyRead, this, &RFC2217SerialDevice::onReadyRead);
  connect(m_socket, &QTcpSocket::connected, this, &RFC2217SerialDevice::onConnected);
  connect(m_socket, &QTcpSocket::disconnected, this, &RFC2217SerialDevice::onDisconnected);
  connect(m_socket, &QTcpSocket::errorOccurred, this, &RFC2217SerialDevice::onError);
  m_socket->connectToHost(m_host, m_port);

}

RFC2217SerialDevice::~RFC2217SerialDevice()
{
  close();
}

bool RFC2217SerialDevice::availablePorts(QStringList &list)
{
  Q_UNUSED(list)
  //list << "RFC2217 192.168.178.138:4000";
  //list << "RFC2217 127.0.0.1:4000";
  return false;
}


void RFC2217SerialDevice::onConnected()
{
  QIODevice::open(ReadWrite);
  sendRFC2217Negotiation();
}

void RFC2217SerialDevice::onDisconnected()
{
  // Has to go through the base class: PortHandler::isOpen() asks QIODevice, so
  // without this a dropped connection still looked open to the rest of the app.
  QIODevice::close();
  emit finished();
}

void RFC2217SerialDevice::onError(QAbstractSocket::SocketError err)
{
  qWarning() << "RFC2217: Socket error:" << err;
  emit finished();
}

void RFC2217SerialDevice::close()
{
  if (m_socket)
  {
    m_socket->disconnectFromHost();
    m_socket->deleteLater();
    m_socket = nullptr;
  }
  QIODevice::close();
}

qint64 RFC2217SerialDevice::bytesAvailable() const
{
 return m_inputBuffer.size() + QIODevice::bytesAvailable();
}

qint64 RFC2217SerialDevice::readData(char *data, qint64 maxSize)
{
  qint64 bytes = qMin(maxSize, qint64(m_inputBuffer.size()));
  memcpy(data, m_inputBuffer.constData(), bytes);
  m_inputBuffer.remove(0, bytes);
  return bytes;
}

qint64 RFC2217SerialDevice::writeData(const char *data, qint64 len)
{
  if (!m_socket || !m_socket->isOpen())
    return -1;
  // 0xff is the telnet IAC and has to be doubled in the data stream
  QByteArray escaped;
  escaped.reserve(int(len) + 4);
  for (qint64 i = 0; i < len; ++i)
  {
    escaped.append(data[i]);
    if (quint8(data[i]) == 0xFF)
      escaped.append(char(0xFF));
  }
  return m_socket->write(escaped) < 0 ? -1 : len;
}

void RFC2217SerialDevice::onReadyRead()
{
  filterTelnet(m_socket->readAll());
  if (!m_inputBuffer.isEmpty())
    emit readyRead();
}

// The server answers every COM-PORT-OPTION with IAC SB 44 <cmd+100> ... IAC SE
// and may negotiate options with IAC WILL/DO/...; none of that is meter data.
// A binary protocol's 0xff bytes arrive as IAC IAC.
void RFC2217SerialDevice::filterTelnet(const QByteArray &raw)
{
  const quint8 IAC = 0xFF, SB = 0xFA, SE = 0xF0, WILL = 0xFB, DONT = 0xFE;
  for (char c : raw)
  {
    const quint8 b = quint8(c);
    switch (m_telnet)
    {
      case TelnetState::Data:
        if (b == IAC) m_telnet = TelnetState::Iac;
        else m_inputBuffer.append(c);
        break;
      case TelnetState::Iac:
        if (b == IAC) { m_inputBuffer.append(c); m_telnet = TelnetState::Data; }
        else if (b == SB) m_telnet = TelnetState::Sub;
        else if (b >= WILL && b <= DONT) m_telnet = TelnetState::Option;   // one option byte follows
        else m_telnet = TelnetState::Data;                                 // NOP and friends
        break;
      case TelnetState::Option:
        m_telnet = TelnetState::Data;
        break;
      case TelnetState::Sub:
        if (b == IAC) m_telnet = TelnetState::SubIac;
        break;
      case TelnetState::SubIac:
        m_telnet = (b == SE || b != IAC) ? TelnetState::Data : TelnetState::Sub;
        break;
    }
  }
}


void RFC2217SerialDevice::sendRFC2217Negotiation()
{
  // 1 = SET-BAUDRATE
  quint32 baud = m_dmmInfo.baud;
  QByteArray baudBytes(4, 0x00);
  baudBytes[0] = (baud >> 24) & 0xFF;
  baudBytes[1] = (baud >> 16) & 0xFF;
  baudBytes[2] = (baud >> 8) & 0xFF;
  baudBytes[3] = baud & 0xFF;
  sendPortOption(1, baudBytes);

  // 2 = SET-DATASIZE
  sendPortOption(2, QByteArray(1, char(m_dmmInfo.bits)));

  // 3 = SET-PARITY
  quint8 parity = 1; // NONE
  switch (m_dmmInfo.parity)
  {
    case 0: parity = 1; break; // none
    case 1: parity = 3; break; // odd
    case 2: parity = 2; break; // even
    default: break;
  }
  sendPortOption(3, QByteArray(1, char(parity)));

  // 4 = SET-STOPSIZE
  sendPortOption(4, QByteArray(1, char(m_dmmInfo.stopBits)));

  // 5 = SET-CONTROL: DTR and RTS are separate SET-CONTROL suboptions, not a bitmask
  sendPortOption(5, QByteArray(1, char(m_dmmInfo.dtr ? 0x08 : 0x09))); // DTR ON/OFF
  sendPortOption(5, QByteArray(1, char(m_dmmInfo.rts ? 0x0b : 0x0c))); // RTS ON/OFF
}

// Sende IAC SB 0x2C <option> <data...> IAC SE
void RFC2217SerialDevice::sendPortOption(quint8 option, const QByteArray &data)
{
  if (!m_socket || !m_socket->isOpen()) return;

  QByteArray pkt;
  pkt.append(char(0xFF)); // IAC
  pkt.append(char(0xFA)); // SB
  pkt.append(char(0x2C)); // COM-PORT-OPTION
  pkt.append(char(option));
  pkt.append(data);

  // Telnet IAC im Payload doppeln (escaping)
  for (int i = 4; i < pkt.size(); ++i)
  {
    if ((quint8)pkt[i] == 0xFF)
      pkt.insert(i++, static_cast<char>(0xFF));
  }

  pkt.append(char(0xFF)); // IAC
  pkt.append(char(0xF0)); // SE
  m_socket->write(pkt);
}
