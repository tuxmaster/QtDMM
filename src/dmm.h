//======================================================================
// File:		dmm.h
// Author:	Matthias Toussaint
// Created:	Tue Apr 10 15:10:49 CEST 2001
//----------------------------------------------------------------------
// This file is part of QtDMM.
//
// QtDMM is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
//
// QtDMM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------
// Copyright (c) 2001 Matthias Toussaint
//======================================================================

#pragma once

#include <QtCore>
#include <QtSerialPort>

#include "readerthread.h"
#include "readevent.h"
#include "dmmdecoder.h"
#include "porthandler.h"

class SharedStateManager;

/// The connection to one multimeter: port, frame reader and decoder.
///
/// MainWid configures it from the settings (setDevice(), setFormat(),
/// setPortSettings(), ...) and calls open(). From then on every decoded
/// reading arrives through value() and every change of the connection state
/// through linkStateChanged() - and, as a human-readable message, through
/// error(), which the main window shows in its status bar: "Connecting",
/// "Connected", "Timeout", the HID hint, ...
///
/// The state is owned here: a one-second watchdog turns silence into
/// Timeout, a vanished port (USB unplugged, bridge gone, socket refused)
/// into Error, and while the user still wants the connection the port is
/// reopened every few seconds. Internally a PortHandler owns the QIODevice,
/// a ReaderThread cuts the byte stream into frames and the DmmDecoder chosen
/// by the format decodes them.
class DMM : public QObject
{
  Q_OBJECT

public:
  /// Where the connection stands; error() carries the matching text.
  enum class LinkState
  {
    Closed,       ///< no port, the user did not ask for one (or close()d it)
    Connecting,   ///< port open, waiting for the first frame
    Connected,    ///< frames arrive
    Timeout,      ///< port open but silent for longer than timeout()
    Error         ///< the port went away; reopened automatically while wanted
  };
  Q_ENUM(LinkState)

  DMM(QObject *parent);
  /// Baud rate; only used by the serial port types.
  void    setSpeed(int);
  /// Port to open: a serial device path, an entry from
  /// PortHandler::availablePorts() or a host:port for RFC2217.
  void    setDevice(const QString &);
  /// Creates, opens and configures the port and starts reading. On failure
  /// error() carries the reason and false is returned.
  bool    open();
  /// Stops reading and closes the port; emits error("Not connected").
  void    close();
  void    setName(const QString &name)  {  m_name = name; }
  /// The meter description; passed on to the port device and the reader.
  void    setDmmInfo(const DmmDecoder::DMMInfo info)  {  m_dmmInfo = info; }
  /// The last message emitted through error().
  QString errorString() const  { return m_error; }
  LinkState linkState() const { return m_state; }
  /// Silence longer than this (ms) is a Timeout; default 3000.
  void    setTimeout(int ms) { m_timeoutMs = ms; }
  int     timeout() const { return m_timeoutMs; }
  /// Seconds between reopen attempts after the port was lost; 0 disables.
  void    setReconnectInterval(int seconds) { m_reconnectSeconds = seconds; }
  bool    isOpen() const;
  /// Selects the decoder (see DmmDecoder::getInstance()).
  void    setFormat(ReadEvent::DataFormat);
  /// Serial line settings. With @p externalSetup the port is used as is and
  /// left open on close(), for setups where another tool configures it.
  void    setPortSettings(QSerialPort::DataBits bits, QSerialPort::StopBits stopBits, QSerialPort::Parity parity,
                                 bool externalSetup, bool rts, bool dtr);
  /// Frames per reading, see ReaderThread::setNumValues().
  void    setNumValues(int);
  /// --debug: dump every frame as hex to stdout.
  void    setConsoleLogging(bool on) { m_consoleLogging = on; }
  /// Instance coordinator, the input of a calculated value (PortType::Calc).
  void    setStateManager(SharedStateManager *state);

Q_SIGNALS:
  /// One decoded reading; the parameters mirror DmmDecoder::DmmResponse.
  /// Emitted twice for frames that carry a second value (id2).
  void    value(double dval, const QString &val, const QString &unit, const QString &special,
                const QString &range, bool hold, bool showBar, int id);
  /// Connection state as a message for the status bar; despite the name
  /// also "Connecting ..." and "Connected".
  void    error(const QString &);
  /// The connection state changed; @p detail is the status bar text.
  void    linkStateChanged(DMM::LinkState state, const QString &detail);

protected:
  void                  initDecoder( ReadEvent::DataFormat df);
  /// Platform-specific advice when the port cannot be opened for lack of
  /// permission (the dialout/dialer group on Unix).
  QString               permissionHint() const;
  /// Status bar text while readings arrive.
  QString               connectedMessage() const;
  /// Status bar text for a silent port (with the HID hint where it applies).
  QString               timeoutMessage() const;
  /// Sets the state and its message; emits linkStateChanged()/error() on change.
  void                  setState(LinkState state, const QString &message);
  /// Creates and opens the port; the common part of open() and a reconnect.
  bool                  openPort();
  /// Drops the port without touching m_wanted.
  void                  closePort();
  /// The port device reports that it is gone (socket closed, USB unplugged).
  void                  portLost(QIODevice *from, const QString &reason);
  PortHandler          *m_portHandler;
  int                   m_speed;
  QSerialPort::Parity   m_parity;
  QSerialPort::StopBits m_stopBits;
  QSerialPort::DataBits m_dataBits;
  QString               m_device;
  /// m_device without secrets (the Bluetooth key), for messages.
  QString               deviceName() const;
  QString               m_error;
  ReaderThread         *m_readerThread;
  LinkState             m_state = LinkState::Closed;
  bool                  m_wanted = false;       ///< open() called and not close()d
  int                   m_timeoutMs = 3000;
  int                   m_reconnectSeconds = 5;
  int                   m_secondsInError = 0;
  QElapsedTimer         m_lastFrame;            ///< since the last complete frame (or open)
  QString               m_name;
  bool                  m_consoleLogging;
  bool                  m_externalSetup;
  bool                  m_dtr;
  bool                  m_rts;
  int                   m_flags;
  int                   m_delayTimer;
  std::shared_ptr<DmmDecoder> m_decoder;
  DmmDecoder::DMMInfo   m_dmmInfo;
  PortHandler::PortType m_portType;

  void                  timerEvent(QTimerEvent *) Q_DECL_OVERRIDE;

protected Q_SLOTS:
  void readEventSLOT(const QByteArray &str, int id);

};

