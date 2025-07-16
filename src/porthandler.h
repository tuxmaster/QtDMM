#pragma once
#include <QIODevice>
#include <QVariant>
#include <QSerialPort>
#include "dmmdecoder.h"


// serialdevice.h
class SerialDevice : public QSerialPort {
    Q_OBJECT
public:
    explicit SerialDevice(const DmmDecoder::DMMInfo info, QString device, QObject *p = Q_NULLPTR) : QSerialPort(p), m_dmmInfo(info) {
        setPortName(device);
    };

    bool init()
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
protected:
    DmmDecoder::DMMInfo m_dmmInfo;
};

// hiddevice.h  (wrapp‑t hidapi oder libusb)
class HidDevice : public QIODevice {
    Q_OBJECT
public:
    explicit HidDevice(const DmmDecoder::DMMInfo info, QString device, QObject *p = Q_NULLPTR): QIODevice(p), m_dmmInfo(info) {};

protected:
    DmmDecoder::DMMInfo m_dmmInfo;

    qint64 readData(char *d, qint64 max)  override  { return 0; };
    qint64 writeData(const char *d, qint64 len) override  { return 0; };
};


class PortHandler : public QObject
{
    Q_OBJECT
public:
    enum class PortType { None, Serial, Hid, Sigrok };

    explicit PortHandler(QObject *parent = Q_NULLPTR) : QObject(parent) {}
    bool create(const DmmDecoder::DMMInfo spec, PortType type, QString device);
    void close();

    QIODevice*  port()     const { return m_port;  }
    PortType    portType() const { return m_type; }
    int         error();
    bool        init();
    bool        isOpen() const { return m_port != Q_NULLPTR && m_port->isOpen(); };

    static PortType str2portType(const QString str);
signals:
    void opened(QIODevice *dev);
    void closed();

private:
    QIODevice *m_port   = Q_NULLPTR;
    PortType   m_type   = PortType::None;
    QString    m_device;
};
