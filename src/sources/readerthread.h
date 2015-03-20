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

#ifndef READERTHREAD_HH
#define READERTHREAD_HH

#include <qobject.h>
#include <qthread.h>
#include <readevent.h>

#define FIFO_LENGTH 100

class QSocketNotifier;

class ReaderThread : public QThread
{
  Q_OBJECT
      
public:
  enum ReadStatus
  {
    Ok,
    Timeout,
    Error,
    NotConnected
  };
  
  ReaderThread( QObject *receiver );
  virtual ~ReaderThread();
  
  void run(); 
  void startRead();
  void setHandle( int handle );
  void setFormat( ReadEvent::DataFormat );
  
  ReadStatus status() const { return m_status; }
  void setNumValues( int num ) { m_numValues = num; }
  
signals:
  void readEvent( const QByteArray &, int id, ReadEvent::DataFormat df );
  
protected:
  QObject              *m_receiver;
  int                   m_handle;
  ReadStatus            m_status;
  bool                  m_readValue;
  char                  m_fifo[FIFO_LENGTH];
  char                  m_buffer[FIFO_LENGTH];
  ReadEvent::DataFormat m_format;
  QSocketNotifier      *m_notifier;
  int                   m_length;
  bool                  m_sendRequest;
  int                   m_id;
  int                   m_numValues;
  
  void readDMM();
  void readMetex14();
  void readVoltcraft14Continuous();
  void readVoltcraft15Continuous();
  void readM9803RContinuous();
  void readPeakTech10();
  void readIsoTech();
  void readQM1537Continuous();
  void readVC820();
  void readVC940();
  void readRS22812Continuous();
  
  int formatLength() const;
  bool checkFormat();
  
protected slots:
  void socketNotifierSLOT( int );

};

#endif // READERTHREAD_HH
