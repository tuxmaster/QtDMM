#pragma once
#include <QtCore>
#include <QIODevice>

#include "dmmdecoder.h"

class PortHandler : public QObject
{
    Q_OBJECT
public:
    enum class PortType { None, Serial, Hid, Sigrok, RFC2217 };

    explicit PortHandler(QObject *parent = Q_NULLPTR) : QObject(parent) {}
    bool create(const DmmDecoder::DMMInfo spec, PortType type, QString device);
    void close();

    QIODevice*  port()     const { return m_port;  }
    PortType    portType() const { return m_type; }
    static QStringList   availablePorts();
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
