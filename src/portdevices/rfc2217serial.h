#pragma once

#include <QIODevice>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

#include "dmmdecoder.h"  // für DMMInfo

class RFC2217SerialDevice : public QIODevice
{
  Q_OBJECT
public:
  explicit RFC2217SerialDevice(const DmmDecoder::DMMInfo &info,
                               QString device,
                               QObject *parent = nullptr);
  ~RFC2217SerialDevice();

  static bool availablePorts(QStringList &portlist);

  void close() override;
  bool isOpen() const
  {
    return m_isOpen;
  }

  qint64 bytesAvailable() const override;

signals:
  void finished();


protected:
  qint64 readData(char *data, qint64 maxSize) override;
  qint64 writeData(const char *data, qint64 len) override;

private slots:
  void onReadyRead();
  void onConnected();
  void onDisconnected();
  void onError(QAbstractSocket::SocketError err);

private:
  void sendRFC2217Negotiation();
  void sendPortOption(quint8 option, const QByteArray &data = {});
  void setPortParameters();

  QTcpSocket *m_socket = nullptr;
  DmmDecoder::DMMInfo m_dmmInfo;
  QString m_host;
  quint16 m_port;
  bool m_isOpen = false;

  QByteArray m_inputBuffer;
  static constexpr unsigned int m_buflen = 1024;
};
