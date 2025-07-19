#pragma once

#include <QtCore>
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
