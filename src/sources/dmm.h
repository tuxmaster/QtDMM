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

#ifndef DMM_HH
#define DMM_HH

#include <QtCore>
#include <QtSerialPort>

#include "readerthread.h"
#include "readevent.h"

class DMM : public QObject
{
  Q_OBJECT

	public:

	  DMM(QObject *parent);
	  void						setSpeed( int );
	  void						setDevice( const QString & );
	  bool						open();
	  void						close();
	  void						setName( const QString &name) {m_name=name;}
	  QString					errorString() const { return m_error; }
	  bool						isOpen() const;
	  void						setFormat( ReadEvent::DataFormat );
	  void						setPortSettings(QSerialPort::DataBits bits, QSerialPort::StopBits stopBits, QSerialPort::Parity parity, bool externalSetup, bool rts, bool cts,
												 bool dsr, bool dtr );
	  void						setNumValues( int );
	  void						setConsoleLogging( bool on ) { m_consoleLogging = on; }

	Q_SIGNALS:
	  void						value( double dval, const QString & val, const QString & unit, const QString & special,
									    const QString & range, bool hold, bool showBar,int id );
	  void						error( const QString & );

	protected:
	  QSerialPort               *m_handle;
	  int                       m_speed;
	  QSerialPort::Parity       m_parity;
	  QSerialPort::StopBits		m_stopBits;
	  QSerialPort::DataBits		m_dataBits;
	  QString                   m_device;
	  QString                   m_error;
	  ReaderThread             *m_readerThread;
	  ReaderThread::ReadStatus	m_oldStatus;
	  QString                   m_name;
	  bool                      m_consoleLogging;
	  bool                      m_externalSetup;
	  bool						m_dtr;
	  bool						m_rts;
	  bool						m_cts;
	  bool						m_dsr;
	  int                       m_flags;
	  int						m_delayTimer;

	  void						timerEvent( QTimerEvent * ) Q_DECL_OVERRIDE;
	  QString					insertComma( const QString &, int );
	  QString					insertCommaIT( const QString &, int );

	  void						readASCII( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readVC820Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readVC870Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readM9803RContinuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readIsoTechContinuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readVC940Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readQM1537Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readRS22812Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
      void						readDO3122Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readCyrustekES51922(const QByteArray & data, int id, ReadEvent::DataFormat df);
	  const char				*vc820Digit( int );
	  const char				*RS22812Digit( int );
      const char                *DO3122Digit( int byte, bool *convOk );

	protected Q_SLOTS:
	  void						readEventSLOT( const QByteArray & str, int id, ReadEvent::DataFormat df );

};

#endif // DMM_HH
