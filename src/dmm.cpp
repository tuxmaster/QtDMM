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
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include "dmm.h"
#include "portdevices/hidserial.h"
#include "portdevices/calc.h"
#include "decoders.h"

#include <stdio.h>
#include <iostream>

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
namespace
{
// Distro-appropriate serial-group fallback, used only when the device file's
// own group can't be determined. Arch/CachyOS use "uucp" instead of Debian's
// "dialout" for serial port access; FreeBSD uses "dialer".
QString distroSuggestedSerialGroup()
{
#ifdef Q_OS_FREEBSD
  return QStringLiteral("dialer");
#endif
  QFile file("/etc/os-release");
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return QStringLiteral("dialout");

  QString id;
  QString idLike;
  while (!file.atEnd())
  {
    const QString line = QString::fromUtf8(file.readLine()).trimmed();
    if (line.startsWith("ID="))
      id = line.mid(3).remove('"');
    else if (line.startsWith("ID_LIKE="))
      idLike = line.mid(8).remove('"');
  }

  if ((id + ' ' + idLike).toLower().contains("arch"))
    return QStringLiteral("uucp");

  return QStringLiteral("dialout");
}

QString serialPermissionHintForDevice(const QString &device)
{
  QFileInfo deviceInfo(device);
  const QString deviceGroup = deviceInfo.group();
  const QString suggestedGroup = !deviceGroup.isEmpty() && deviceGroup != QLatin1String("root")
    ? deviceGroup
    : distroSuggestedSerialGroup();

  QString message = QObject::tr("No permission to access %1.").arg(device);

  if (!suggestedGroup.isEmpty())
  {
    message += QObject::tr("\n\nOn this system the device is typically accessible via group '%1'.")
      .arg(suggestedGroup);
#ifdef Q_OS_FREEBSD
    message += QObject::tr("\nAdd your user with:\n\nsudo pw groupmod %1 -m $USER")
      .arg(suggestedGroup);
#else
    message += QObject::tr("\nAdd your user with:\n\nsudo usermod -aG %1 $USER")
      .arg(suggestedGroup);
#endif
    message += QObject::tr("\n\nThen log out and back in so the new group membership becomes active.");
  }

  return message;
}
}
#endif

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
    m_delayTimer(0),
    m_decoder(Q_NULLPTR)
{
  m_portHandler  = new PortHandler(this);
  m_readerThread = new ReaderThread(this);

  connect(m_readerThread, SIGNAL(readEvent(const QByteArray &, int)),
          this, SLOT(readEventSLOT(const QByteArray &, int)));

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
  m_portType = PortHandler::str2portType(deviceList.first());
  // "calc <unit> <formula>": the formula may contain spaces, so keep
  // everything after the type token; the other types take the last token
  if (m_portType == PortHandler::PortType::Calc)
    m_device = device.section(' ', 1).trimmed();
  else
    m_device = deviceList.last();
}

void DMM::setStateManager(SharedStateManager *state)
{
  m_portHandler->setStateManager(state);
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
  if (!m_portHandler->create(m_dmmInfo, m_portType, m_device))
  {
    m_error = tr("Error creating port %1.").arg(m_device);
    Q_EMIT error(m_error);
    return false;
  }

  if (m_portHandler->port() && !m_portHandler->port()->open(QIODevice::ReadWrite))
  {
    if (m_portType == PortHandler::PortType::Calc)
    {
      // the formula did not parse; the device says where
      m_error = m_portHandler->port()->errorString();
      Q_EMIT error(m_error);
      m_portHandler->close();
      return false;
    }
    switch (m_portHandler->error())
    {
      case QSerialPort::PermissionError:
        m_error = permissionHint();
        QMessageBox::critical(nullptr, tr("Missing Permission"), m_error);
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
  if (auto *calc = dynamic_cast<CalcDevice *>(m_portHandler->port()))
  {
    // which input instance is missing or silent; shown in the status bar
    // instead of "Connected" until all inputs deliver
    connect(calc, &CalcDevice::status, this, [this](const QString &message)
    {
      m_error = message.isEmpty() ? connectedMessage() : message;
      Q_EMIT error(m_error);
    });
  }
  m_readerThread->setHandle(m_portHandler->port());
  timerEvent(0);

  // mt: added timer id
  m_delayTimer = startTimer(1000);
  return true;
}

QString DMM::connectedMessage() const
{
  if (m_portType == PortHandler::PortType::Calc)
    return tr("Calculating %1").arg(m_device.section(' ', 1));
  return tr("Connected %1").arg(m_device);
}

QString DMM::permissionHint() const
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
  if (m_portType == PortHandler::PortType::Serial && !m_device.isEmpty())
    return serialPermissionHintForDevice(m_device);
#endif

  return tr("Access denied for %1.").arg(m_device);
}

void DMM::close()
{
  // mt: added timer id
  if (m_delayTimer > 0)
  {
    killTimer(m_delayTimer);
    m_delayTimer = 0;
  }

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
      m_error = r->error.isEmpty() ? connectedMessage() : QString("%1 %2").arg(r->error, m_device);
    }
    else
      m_error = tr("Error %1").arg(m_device);
  }
  else
  {
    if (ReaderThread::Error == m_readerThread->status())
      m_error = tr("Read error on device %1.\nDMM connected and switched on?").arg(m_device);
    else if (ReaderThread::Timeout == m_readerThread->status())
    {
      // a HID cable that answers but never carries UART bytes: the meter's
      // serial output is off (UNI-T: the RS232/USB button)
      auto *hid = dynamic_cast<HIDSerialDevice *>(m_portHandler->port());
      if (hid && hid->cableAnswers() && !hid->dataSeen())
        m_error = tr("The USB cable answers, but the meter sends nothing.\n"
                     "Switch on the meter's serial output (on UNI-T meters: hold the RS232/USB button).");
      else
        m_error = tr("Timeout on device %1.\nDMM connected and switched on?").arg(m_device);
    }
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
