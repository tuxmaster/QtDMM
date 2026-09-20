//======================================================================
// File:		readerthread.h
// Author:	Matthias Toussaint
// Created:	Sat Apr 14 12:42:06 CEST 2001
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
#include <QIODevice>
#include "readevent.h"
#include "dmmdecoder.h"

class QSerialPort;

/// Collects bytes from the port and cuts them into decoder frames.
///
/// Despite the name this is no thread any more: it listens to
/// QIODevice::readyRead() of the port set with setHandle() and feeds every
/// byte into a ring buffer. After each byte the decoder's
/// DmmDecoder::checkFormat() is asked whether the buffer now ends with a
/// complete frame; if so the frame is emitted through readEvent() and the
/// value id advances (meters that send several lines per reading, see
/// setNumValues()). For polled protocols (Metex14) a request is written to
/// the port once per second (start(), startRead()).
class ReaderThread : public QObject
{
  Q_OBJECT
public:
  /// Outcome of the last read attempt, reported by status().
  enum ReadStatus
  {
    Ok,
    Timeout,
    Error,
    NotConnected
  };
  ReaderThread(QObject *receiver);
  /// Starts the one-second timer that sends poll requests (see startRead()).
  void        start();
  /// Arms the next poll request; DMM calls this from its own timer.
  void        startRead();
  /// Switches to a new port (or none). Connections to the old port are dropped.
  void        setHandle(QIODevice *handle);
  /// Protocol of the connected meter; decides whether polling is needed.
  void        setFormat(ReadEvent::DataFormat format) { m_format = format; };
  /// Decoder whose checkFormat()/getPacketLength() delimit the frames.
  void        setDecoder(std::shared_ptr<DmmDecoder> decoder) { m_decoder = decoder; };

  ReadStatus  status() const  { return m_status;  }
  /// Number of frames one reading consists of; the id passed with
  /// readEvent() cycles through 0..num-1.
  void        setNumValues(int num)  { m_numValues = num; }

Q_SIGNALS:
  /// A complete frame, exactly DmmDecoder::getPacketLength() bytes long.
  void        readEvent(const QByteArray &, int id);

protected:
  ReadStatus            m_status;
  bool                  m_readValue;
  char                  m_fifo[FIFO_LENGTH];
  char                  m_buffer[FIFO_LENGTH];
  ReadEvent::DataFormat m_format;
  int                   m_length;
  bool                  m_sendRequest;
  int                   m_id;
  int                   m_numValues;
  std::shared_ptr<DmmDecoder> m_decoder;
  void sendReadRequest();

protected Q_SLOTS:
  void socketNotifierSLOT();
  void socketClose();

private:
  QIODevice *m_port;

private Q_SLOTS:
  void timer();
};

