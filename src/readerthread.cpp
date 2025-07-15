//======================================================================
// File:		readerthread.cpp
// Author:	Matthias Toussaint
// Created:	Sat Apr 14 12:44:00 CEST 2001
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

#include <QtGui>
#include <QtWidgets>
#include <QtSerialPort>

#include "readerthread.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>

ReaderThread::ReaderThread(QObject *receiver) :
  QObject(receiver),
  m_receiver(receiver),
  m_readValue(false),
  m_format(ReadEvent::Metex14),
  m_length(0),
  m_sendRequest(true),
  m_id(0),
  m_numValues(1),
  m_consoleLogging(false),
  m_driver(Q_NULLPTR)
{
  m_port = Q_NULLPTR;
}

void ReaderThread::setFormat(ReadEvent::DataFormat format)
{
  m_format = format;
}

void ReaderThread::setHandle(QSerialPort *handle)
{
  m_port = handle;

  m_readValue = false;

  if (!m_port)
  {
    m_status = ReaderThread::NotConnected;
    m_readValue = false;
  }
  else
  {
    connect(m_port, SIGNAL(readyRead()), this, SLOT(socketNotifierSLOT()));
    connect(m_port, SIGNAL(aboutToClose()), this, SLOT(socketClose()));
  }
}

void ReaderThread::setConsoleLogging(bool on)
{
  m_consoleLogging = on;
}

void ReaderThread::setDriver(DmmDriver* driver)
{
  m_driver = driver;
}

void ReaderThread::socketClose()
{
  m_id = 0;
}

void ReaderThread::start()
{
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
  timer->start(10);
}
void ReaderThread::timer()
{
  if (m_readValue)
  {
    readDMM();
    m_readValue = false;
  }
}

void ReaderThread::startRead()
{
  //std::cerr << "start read" << std::endl;
  m_readValue = true;
  m_sendRequest = true;
}

void ReaderThread::socketNotifierSLOT()
{
  //std::cerr << "socket call" << std::endl;

  int  retval;
  char byte;
  QByteArray dataOut;

  m_status = ReaderThread::Ok;

  while ((retval = m_port->read(&byte, 1)) > 0)
  {
    m_fifo[m_length] = byte;

    if (m_consoleLogging)
      fprintf(stderr, "%02X ", static_cast<uint8_t>(byte));

    if (checkFormat())
    {
      m_length = (m_length - formatLength() + 1 + FIFO_LENGTH) % FIFO_LENGTH;

      if (m_consoleLogging)
        fprintf(stderr, "Format Ok!\n");
      dataOut = QByteArray();

      for (int i = 0; i < formatLength(); ++i)
      {
        dataOut.append(m_fifo[m_length]);
        if (m_consoleLogging)
          fprintf(stderr, "%02X ", static_cast<uint8_t>(dataOut[i]));
        m_length = (m_length + 1) % FIFO_LENGTH;
      }

      if (m_consoleLogging)
        fprintf(stderr, "\n");
      m_sendRequest = true;
      m_length = 0;

      Q_EMIT readEvent(dataOut, m_id, m_format);
      // QApplication::postEvent( m_receiver,
      // new ReadEvent( m_buffer, formatLength(), m_id, m_format ) );

      m_id = (m_id + 1) % m_numValues;
    }
    else
      m_length = (m_length + 1) % FIFO_LENGTH;
  }

  if (-1 == retval)
    m_status = ReaderThread::Error;
}

int  ReaderThread::formatLength() const
{
  return (m_driver == Q_NULLPTR) ? 0 : m_driver->getPacketLength(m_format );
}

void ReaderThread::readDMM()
{
  switch (m_format)
  {
    case ReadEvent::Metex14:
      readMetex14();
      break;
  }
}

bool ReaderThread::checkFormat()
{
  return (m_driver != Q_NULLPTR && m_driver->checkFormat(m_fifo,m_length,m_format));
}

void ReaderThread::readMetex14()
{
  if (m_sendRequest)
  {
    /* TODO: Errorhandling */
    if (m_port->write("D\n", 2) != 2)
      m_status = Error;
    //std::cerr << "WROTE: " << ret << std::endl;
    m_sendRequest = false;
  }
}
