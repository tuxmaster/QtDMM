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

/// The connection to one multimeter: port, frame reader and decoder.
///
/// MainWid configures it from the settings (setDevice(), setFormat(),
/// setPortSettings(), ...) and calls open(). From then on every decoded
/// reading arrives through value() and every change of the connection state
/// as a human-readable message through error(), which the main window shows
/// in its status bar - "Connected", "Timeout", the HID hint, ...
///
/// Internally a PortHandler owns the QIODevice, a ReaderThread cuts the byte
/// stream into frames and the DmmDecoder chosen by the format decodes them.
class DMM : public QObject
{
  Q_OBJECT

public:

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

Q_SIGNALS:
  /// One decoded reading; the parameters mirror DmmDecoder::DmmResponse.
  /// Emitted twice for frames that carry a second value (id2).
  void    value(double dval, const QString &val, const QString &unit, const QString &special,
                const QString &range, bool hold, bool showBar, int id);
  /// Connection state as a message for the status bar; despite the name
  /// also "Connecting ..." and "Connected".
  void    error(const QString &);

protected:
  void                  initDecoder( ReadEvent::DataFormat df);
  /// Platform-specific advice when the port cannot be opened for lack of
  /// permission (the dialout/dialer group on Unix).
  QString               permissionHint() const;
  PortHandler          *m_portHandler;
  int                   m_speed;
  QSerialPort::Parity   m_parity;
  QSerialPort::StopBits m_stopBits;
  QSerialPort::DataBits m_dataBits;
  QString               m_device;
  QString               m_error;
  ReaderThread         *m_readerThread;
  ReaderThread::ReadStatus m_oldStatus;
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

