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

#include "readerthread.h"
#include "readevent.h"
#include <fcntl.h>
#include <sys/termios.h>
#include <sys/ioctl.h>

class DMM : public QObject
{
  Q_OBJECT

	public:
	  enum {
		f_rts=1,
		f_cts=2,
		f_dsr=4,
		f_dtr=8
	  };

	  DMM(QObject *parent);
	  void						setSpeed( int );
	  void						setDevice( const QString & );
	  bool						open();
	  void						close();
	  QString					errorString() const { return m_error; }
	  bool						isOpen() const { return m_handle >= 0; }
	  void						setFormat( ReadEvent::DataFormat );
	  void						setPortSettings( int bits, int stopBits, int parity, bool externalSetup, bool rts, bool cts,
												 bool dsr, bool dtr );
	  void						setNumValues( int );
	  void						setConsoleLogging( bool on ) { m_consoleLogging = on; }

	Q_SIGNALS:
	  void						value( double dval, const QString & val, const QString & unit, const QString & special,
									   bool showBar,int id );
	  void						error( const QString & );

	protected:
	  int                       m_handle;
	  int                       m_speed;
	  int                       m_parity;
	  QString                   m_device;
	  QString                   m_error;
	  ReaderThread             *m_readerThread;
	  tcflag_t                  m_c_cflag;
	  ReaderThread::ReadStatus	m_oldStatus;
	  struct termios            m_oldSettings;
	  bool                      m_consoleLogging;
	  bool                      m_externalSetup;
	  int                       m_flags;
	  // mt: added
	  int						m_delayTimer;

	  void						timerEvent( QTimerEvent * ) Q_DECL_OVERRIDE;
	  //void customEvent( QCustomEvent * );
	  QString					insertComma( const QString &, int );
	  QString					insertCommaIT( const QString &, int );

	  void						readASCII( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readVC820Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readM9803RContinuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readIsoTechContinuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readVC940Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readQM1537Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  void						readRS22812Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df );
	  const char				*vc820Digit( int );
	  const char				*RS22812Digit( int );

	protected Q_SLOTS:
	  void						readEventSLOT( const QByteArray & str, int id, ReadEvent::DataFormat df );

};

#endif // DMM_HH
