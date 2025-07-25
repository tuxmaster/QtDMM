//======================================================================
// File:		dmm.cpp
// Author:	Matthias Toussaint
// Created:	Tue Apr 10 15:10:29 CEST 2001
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

#include "dmm.h"
#include "decoders.h"

#include <stdio.h>
#include <iostream>

DMM::DMM(QObject *parent)
  : QObject(parent),
    m_speed(600),
    m_parity(QSerialPort::NoParity),
    m_stopBits(QSerialPort::OneStop),
    m_dataBits(QSerialPort::Data7),
    m_device(""),
    m_oldStatus(ReaderThread::NotConnected),
    m_consoleLogging(false),
    m_externalSetup(false),
    m_dtr(false),
    m_rts(false),
    m_decoder(Q_NULLPTR)
{
  m_portHandler  = new PortHandler(this);
  m_readerThread = new ReaderThread(this);

  connect(m_readerThread, SIGNAL(readEvent(const QByteArray &, int, ReadEvent::DataFormat)),
          this, SLOT(readEventSLOT(const QByteArray &, int, ReadEvent::DataFormat)));

  m_readerThread->start();

}

void DMM::setPortSettings(QSerialPort::DataBits bits, QSerialPort::StopBits stopBits,
                          QSerialPort::Parity parity, bool externalSetup, bool rts, bool dtr)
{
  m_externalSetup = externalSetup;
  m_parity  = parity;
  m_stopBits = stopBits;
  m_dataBits = bits;
  m_dtr = dtr;
  m_rts = rts;
}

void DMM::setFormat(ReadEvent::DataFormat format)
{
  initDecoder(format);
  m_readerThread->setFormat(format);
}

bool DMM::isOpen() const
{
  return m_portHandler->isOpen();
}

void DMM::setSpeed(int speed)
{
  m_speed = speed;
}

void DMM::setDevice(const QString &device)
{
  QStringList deviceList = device.split( " " );
  m_device = deviceList.last();
  m_portType = PortHandler::str2portType(deviceList.first());
}

void DMM::initDecoder( ReadEvent::DataFormat df)
{
  if (m_decoder == Q_NULLPTR || m_decoder->getType() != df)
  {
    m_decoder = DmmDecoder::getInstance(df);
    m_readerThread->setDecoder(m_decoder);
  }
}


bool DMM::open()
{
  m_portHandler->create(m_dmmInfo, m_portType, m_device);

  if (m_portHandler->port() && !m_portHandler->port()->open(QIODevice::ReadWrite))
  {
    switch (m_portHandler->error())
    {
      case QSerialPort::PermissionError:
        m_error = tr("Access denied for %1.").arg(m_device);
        break;
      case QSerialPort::DeviceNotFoundError:
        m_error = tr("No such device %1.").arg(m_device);
        break;
      default:
        m_error = tr("Error opening %1.\nDMM connected and switched on?").arg(m_device);
        break;
    }
    Q_EMIT error(m_error);
    m_portHandler->close();
    return false;
  }

  if (!m_externalSetup)
  {
    if (!m_portHandler->init())
    {
      m_error = tr("Error configuring serial port %1.").arg(m_device);
      Q_EMIT error(m_error);
      m_portHandler->close();
      return false;
    }
  }
  m_error = tr("Connecting ...");
  Q_EMIT error(m_error);
  m_readerThread->setHandle(m_portHandler->port());
  timerEvent(0);

  // mt: added timer id
  m_delayTimer = startTimer(1000);
  return true;
}

void DMM::close()
{
  // mt: added timer id
  killTimer(m_delayTimer);

  m_error = tr("Not connected");
  Q_EMIT error(m_error);

  m_readerThread->setHandle(Q_NULLPTR);

  if (m_portHandler->port())
  {
    if (!m_externalSetup)
    {
      m_portHandler->close();
    }
  }
  m_oldStatus = ReaderThread::NotConnected;
}

void DMM::timerEvent(QTimerEvent *)
{
  if (!m_portHandler->port())
    Q_EMIT error(m_error);
  else
    m_readerThread->startRead();
}

void DMM::readEventSLOT(const QByteArray &data, int id)
{
  if (ReaderThread::Ok == m_readerThread->status() )
  {
    if (m_consoleLogging)
    {
      for (int i = 0; i < data.size(); ++i)
        fprintf(stdout, "%02X ", data[i] & 0x0ff);
      fprintf(stdout, "\r\n");
    }
    if (m_decoder == Q_NULLPTR)
      return;

    // call decode of current decoder to convert data into distinct values
    if (auto r = m_decoder->decode(data, id); r)
    {
      Q_EMIT value(r->dval, r->val, r->unit, r->special, r->range, r->hold, r->showBar, r->id);
      if (r->id2 > 0)
        Q_EMIT value(r->dval2, r->val2, r->unit2, r->special, r->range, r->hold, r->showBar, r->id2);
      m_error = r->error.isEmpty() ? tr("Connected %1").arg(m_device) : tr("%1 %2").arg(r->error, m_device);
    }
    else
      m_error = tr("Error %1").arg(m_device);
  }
  else
  {
    if (ReaderThread::Error == m_readerThread->status())
      m_error = tr("Read error on device %1.\nDMM connected and switched on?").arg(m_device);
    else if (ReaderThread::Timeout == m_readerThread->status())
      m_error = tr("Timeout on device %1.\nDMM connected and switched on?").arg(m_device);
    else if (ReaderThread::NotConnected == m_readerThread->status())
      m_error = tr("Not connected");
  }
  if (m_oldStatus != m_readerThread->status())
    Q_EMIT error(m_error);
  m_oldStatus = m_readerThread->status();
}

void DMM::setNumValues(int numValues)
{
  m_readerThread->setNumValues(numValues);
}
