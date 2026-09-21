#pragma once

#include <QIODevice>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

#include "dmmdecoder.h"

/// A meter on a serial port shared over the network (RFC 2217, "telnet com
/// port control"), e.g. by ser2net or ESP-Link.
///
/// On connect the telnet options are negotiated and the meter's line settings
/// from the DMMInfo are sent as COM-PORT-OPTION commands. The socket's data
/// is buffered and handed to ReaderThread through the QIODevice interface.
class RFC2217SerialDevice : public QIODevice
{
  Q_OBJECT
public:
  /// @param info   the meter, for the line settings sent to the server
  /// @param device "host:port"; the TCP connection is started right away
  /// @param parent parent object
  explicit RFC2217SerialDevice(const DmmDecoder::DMMInfo &info,
                               QString device,
                               QObject *parent = nullptr);
  ~RFC2217SerialDevice();

  /// Nothing can be enumerated on the network; adds nothing and returns false.
  /// The user types the address into the port field.
  static bool availablePorts(QStringList &portlist);

  void close() override;

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

  /// Strips telnet commands/subnegotiations (the server's option replies)
  /// from the stream and undoes the IAC IAC escaping; the rest is meter data.
  void filterTelnet(const QByteArray &raw);

  QByteArray m_inputBuffer;
  static constexpr unsigned int m_buflen = 1024;
  /// telnet parser state between socket reads
  enum class TelnetState { Data, Iac, Option, Sub, SubIac } m_telnet = TelnetState::Data;
};
