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
#include "drivers/drivers.h"

#include <stdio.h>
#include <iostream>

DMM::DMM(QObject *parent)
  : QObject(parent),
    m_handle(Q_NULLPTR),
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
    m_cts(false),
    m_dsr(false),
    m_driver(Q_NULLPTR)
{
  m_readerThread = new ReaderThread(this);

  // mt: QThread emits a signal now. no more custom event
  connect(m_readerThread, SIGNAL(readEvent(const QByteArray &, int, ReadEvent::DataFormat)),
          this, SLOT(readEventSLOT(const QByteArray &, int, ReadEvent::DataFormat)));

  m_readerThread->start();

}

void DMM::setPortSettings(QSerialPort::DataBits bits, QSerialPort::StopBits stopBits, QSerialPort::Parity parity, bool externalSetup,
                          bool rts, bool cts, bool dsr, bool dtr)
{
  m_externalSetup = externalSetup;
  m_parity  = parity;
  m_stopBits = stopBits;
  m_dataBits = bits;
  m_dtr = dtr;
  m_rts = rts;
  m_cts = cts;
  m_dsr = dsr;
}

void DMM::setFormat(ReadEvent::DataFormat format)
{
  m_readerThread->setFormat(format);
}

bool DMM::isOpen() const
{
  if (!m_handle)
    return false;
  return m_handle->isOpen();
}

void DMM::setSpeed(int speed)
{
  m_speed = speed;
}

void DMM::setDevice(const QString &device)
{
  m_device = device;
}

bool DMM::open()
{
  m_handle = new QSerialPort(this);
  m_handle->setPortName(m_device);

  if (!m_handle->open(QIODevice::ReadWrite))
  {
    int errorCode = m_handle->error();
    switch (errorCode)
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
    delete m_handle;
    m_handle = Q_NULLPTR;
    return false;
  }

  if (!m_externalSetup)
  {
    m_error = tr("Error configuring serial port %1.").arg(m_device);
    if ((!m_handle->setParity(m_parity)) || (!m_handle->setBaudRate(m_speed)) || (!m_handle->setStopBits(m_stopBits)) ||
        (!m_handle->setDataBits(m_dataBits)) || (!m_handle->setDataTerminalReady(m_dtr)) || (!m_handle->setRequestToSend(m_rts)))
    {
      Q_EMIT error(m_error);
      delete m_handle;
      m_handle = Q_NULLPTR;
      return false;
    }
  }
  m_error = tr("Connecting ...");
  Q_EMIT error(m_error);
  m_readerThread->setHandle(m_handle);
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

  if (m_handle)
  {
    if (!m_externalSetup)
    {
      m_handle->close();
      delete m_handle;
      m_handle = Q_NULLPTR;
    }
  }
  m_oldStatus = ReaderThread::NotConnected;
}

void DMM::timerEvent(QTimerEvent *)
{
  if (!m_handle)
    Q_EMIT error(m_error);
  else
    m_readerThread->startRead();
}

void DMM::readEventSLOT(const QByteArray &data, int id, ReadEvent::DataFormat df)
{
  if (ReaderThread::Ok == m_readerThread->status())
  {
    if (m_consoleLogging)
    {
      for (int i = 0; i < data.size(); ++i)
        fprintf(stdout, "%02X ", data[i] & 0x0ff);
      fprintf(stdout, "\r\n");
    }
    switch (df)
    {
      case ReadEvent::Metex14:
      case ReadEvent::PeakTech10:
      case ReadEvent::Voltcraft14Continuous:
      case ReadEvent::Voltcraft15Continuous: readDMM<DrvAscii>(data, id, df);   break;
      case ReadEvent::M9803RContinuous:      readDMM<DrvM9803R>(data, id, df);  break;
      case ReadEvent::VC820Continuous:       readDMM<DrvVC820>(data, id, df);   break;
      case ReadEvent::VC870Continuous:       readDMM<DrvVC870>(data, id, df);   break;
      case ReadEvent::IsoTech:               readDMM<DrvIsoTech>(data, id, df); break;
      case ReadEvent::VC940Continuous:       readDMM<DrvVC940>(data, id, df);   break;
      case ReadEvent::QM1537Continuous:      readDMM<DrvQM1537>(data, id, df);  break;
      case ReadEvent::RS22812Continuous:     readDMM<DrvRS22812>(data, id, df); break;
      case ReadEvent::DO3122Continuous:      readDMM<DrvDO3122>(data, id, df);  break;
      case ReadEvent::CyrustekES51922:       readDMM<DrvCyrusTekES51922>(data, id, df); break;
      case ReadEvent::CyrustekES51962:       readDMM<DrvCyrusTekES51962>(data, id, df); break;
    }
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
