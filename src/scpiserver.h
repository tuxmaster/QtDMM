// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QObject>
#include <QString>

#include <functional>

class QTcpServer;
class QTcpSocket;

/// A small SCPI instrument on a raw TCP socket (port 5025 by default), the
/// way bench meters and lxi-tools speak it: one program message per line,
/// commands separated by ';', queries answered in the same order. It turns
/// the meter QtDMM reads into a network instrument, so anything that talks
/// SCPI - lxi-tools, sigrok's scpi-dmm, a Python script - can read a
/// consumer multimeter.
///
/// The server only reports; it never writes to the meter. It knows the
/// current reading (setReading()), whether the meter is connected and
/// whether the recorder runs, and asks for the recorder / connection
/// through signals. process() is the whole command interpreter and is
/// public so the test can drive it without sockets.
class ScpiServer : public QObject
{
  Q_OBJECT
public:
  /// What the meter last delivered, in SI base units.
  struct Reading
  {
    double value = 0;
    QString unit;      ///< "V", "A", "Ohm" ... without prefix
    QString special;   ///< "DC", "AC", "DI" ...
    QString range;     ///< "AUTO", "MANU" or the meter's range text
    bool overload = false;
    bool hold = false;
    bool valid = false;
    qint64 msecs = 0;
  };

  explicit ScpiServer(QObject *parent = nullptr);
  ~ScpiServer() override;

  /// Listens on @p address:@p port; when the port is taken the next
  /// @p tries ports are tried (several QtDMM instances on one machine).
  /// Returns false when none could be bound; errorString() says why.
  bool start(const QHostAddress &address, quint16 port, int tries = 10);
  void stop();
  bool isListening() const;
  quint16 port() const;
  QHostAddress address() const;
  int clientCount() const { return m_clients.size(); }
  QString errorString() const { return m_error; }

  /// The meter shown in *IDN? ("UNI-T UT61E"); the manufacturer is QtDMM.
  void setModel(const QString &model) { m_model = model; }
  /// Reading of value @p id (0 = main, 1 = second) as it arrives.
  void setReading(int id, const Reading &reading);
  void setConnected(bool on) { m_connected = on; }
  void setRecording(bool on) { m_recording = on; }
  bool connected() const { return m_connected; }
  bool recording() const { return m_recording; }

  /// Supplies the screen dump for HCOPy:SDUMp:DATA?: the main window
  /// encoded in @p format ("PNG", "BMP", "JPG"); empty when not possible.
  using ScreenshotSource = std::function<QByteArray(const QByteArray &format)>;
  void setScreenshotSource(ScreenshotSource source) { m_screenshot = std::move(source); }
  /// Wraps @p data as an IEEE 488.2 definite-length block ("#6123456...").
  static QByteArray block(const QByteArray &data);

  /// Runs one program message (without the terminator) and returns the
  /// response: the query answers joined by ';', with a trailing newline,
  /// or nothing when no command was a query. Errors go to the error
  /// queue (SYST:ERR?).
  QByteArray process(const QByteArray &message);

  /// Number of readings considered current: older ones answer as stale.
  static constexpr qint64 kStaleMs = 5000;

Q_SIGNALS:
  void clientsChanged(int count);
  /// INITiate / ABORt: the client wants the recorder started or stopped.
  void startRecording();
  void stopRecording();
  /// INPut ON|OFF: the client wants the meter connected or disconnected.
  void connectRequested(bool on);

private:
  struct Command
  {
    QStringList path;     ///< mnemonics, upper case, numeric suffix stripped
    QList<int> suffix;    ///< numeric suffix per mnemonic (1 when absent)
    bool query = false;
    QString args;
  };

  void onNewConnection();
  void onReadyRead(QTcpSocket *socket);
  QByteArray handle(const Command &cmd, bool &isQuery);
  QString handleText(const Command &cmd, bool &isQuery);
  void pushError(int code, const QString &text);
  static bool matches(const QString &mnemonic, const char *longForm);
  static QString number(double v);
  QString function(const Reading &r) const;
  const Reading *reading(int channel);

  QTcpServer *m_server;
  QList<QTcpSocket *> m_clients;
  QString m_error;
  QString m_model;
  Reading m_readings[2];
  bool m_connected = false;
  bool m_recording = false;
  QList<QPair<int, QString>> m_errors;
  ScreenshotSource m_screenshot;
  QByteArray m_screenshotFormat = "PNG";
  QStringList m_lastPath;   ///< for the "compound header" rule of ';'
};
