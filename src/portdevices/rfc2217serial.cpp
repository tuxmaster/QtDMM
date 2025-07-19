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
  qInfo() << "c";
  close();
}

bool RFC2217SerialDevice::availablePorts(QStringList &list)
{
  list << "RFC2217 192.168.178.138:4000"; // Beispiel, später automatisieren
  list << "RFC2217 127.0.0.1:4000"; // Beispiel, später automatisieren
  return true;
}


void RFC2217SerialDevice::onConnected()
{
  qInfo() << "oc";
  m_isOpen = true;
  QIODevice::open(ReadWrite);
  sendRFC2217Negotiation(); // optional
  setPortParameters();
}

void RFC2217SerialDevice::onDisconnected()
{
  m_isOpen = false;
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
  m_isOpen = false;
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
  return m_socket->write(data, len);
}

void RFC2217SerialDevice::onReadyRead()
{
  m_inputBuffer.append(m_socket->readAll());
  emit readyRead();
}


void RFC2217SerialDevice::sendRFC2217Negotiation()
{
    // RFC2217 ist ein Telnet-Protokoll mit speziellen Steuerbytes
    // z. B. IAC 0xFF, SB 0xFA, COM-PORT-OPTION 0x2C, etc.
    // => komplex, besser später via libserialport oder einfacher State-Machine
}

void RFC2217SerialDevice::setPortParameters()
{
    // 1 = SET-BAUDRATE
    quint32 baud = m_dmmInfo.baud;
    qInfo() <<  m_dmmInfo.baud;
    QByteArray baudBytes(4, 0x00);
    baudBytes[0] = (baud >> 24) & 0xFF;
    baudBytes[1] = (baud >> 16) & 0xFF;
    baudBytes[2] = (baud >> 8) & 0xFF;
    baudBytes[3] = baud & 0xFF;
    sendPortOption(1, baudBytes);

    // 2 = SET-DATASIZE
    qInfo() <<  m_dmmInfo.bits;
    sendPortOption(2, QByteArray(1, char(m_dmmInfo.bits)));

    // 3 = SET-PARITY
    quint8 parity = 1; // NONE
    qInfo() <<  m_dmmInfo.parity;
    switch (m_dmmInfo.parity) {
        case 0: parity = 1; break; // none
        case 1: parity = 3; break; // odd
        case 2: parity = 2; break; // even
        default: break;
    }
    sendPortOption(3, QByteArray(1, char(parity)));

    // 4 = SET-STOPSIZE
    qInfo() <<  m_dmmInfo.stopBits;
    sendPortOption(4, QByteArray(1, char(m_dmmInfo.stopBits)));

    // 5 = SET-FLOWCONTROL (wir deaktivieren HW/SW)
    sendPortOption(5, QByteArray(1, char(0))); // none

    // 6 = SET-RTS
    qInfo() <<  m_dmmInfo.rts;
    sendPortOption(6, QByteArray(1, char(m_dmmInfo.rts ? 1 : 0)));

    // 8 = SET-DTR
    qInfo() <<  m_dmmInfo.dtr;
    sendPortOption(8, QByteArray(1, char(m_dmmInfo.dtr ? 1 : 0)));
}

// Sende IAC SB 0x2C <option> <data...> IAC SE
void RFC2217SerialDevice::sendPortOption(quint8 option, const QByteArray &data)
{
    if (!m_socket || !m_socket->isOpen()) return;
qInfo() << "sendO";
    QByteArray pkt;
    pkt.append(char(0xFF)); // IAC
    pkt.append(char(0xFA)); // SB
    pkt.append(char(0x2C)); // COM-PORT-OPTION
    pkt.append(char(option));
    pkt.append(data);

    // Telnet IAC im Payload doppeln (escaping)
    for (int i = 4; i < pkt.size(); ++i) {
        if ((quint8)pkt[i] == 0xFF)
            pkt.insert(i++, 0xFF);
    }

    pkt.append(char(0xFF)); // IAC
    pkt.append(char(0xF0)); // SE
    m_socket->write(pkt);
}
