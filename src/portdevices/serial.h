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

    static bool availablePorts(QStringList &portlist);
    bool init();

protected:
    DmmDecoder::DMMInfo m_dmmInfo;
};
