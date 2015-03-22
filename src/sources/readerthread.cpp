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

#include "readerthread.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>

ReaderThread::ReaderThread( QObject *receiver ) :
  QThread(),
  m_receiver( receiver ),
  m_readValue( false ),
  m_format( ReadEvent::Metex14 ),
  m_notifier( 0 ),
  m_length( 0 ),
  m_sendRequest( true ),
  m_id( 0 ),
  m_numValues( 1 )
{
  m_buffer[14] = '\0';
}

void ReaderThread::setFormat( ReadEvent::DataFormat format )
{
  m_format = format;
}

void ReaderThread::setHandle( int handle )
{
  delete m_notifier;
  m_notifier = 0;

  m_handle = handle;
  m_readValue = false;

  if (-1 == m_handle)
  {
	m_status = ReaderThread::NotConnected;
	m_readValue = false;
  }
  else
  {
	m_notifier = new QSocketNotifier( m_handle, QSocketNotifier::Read );

	connect( m_notifier, SIGNAL( activated(int) ), this, SLOT( socketNotifierSLOT(int) ));
  }
}

void ReaderThread::run()
{
  while (1)
  {
	if (m_readValue)
	{
	  //std::cerr << "going to read" << std::endl;
	  readDMM();
	  m_readValue = false;
	}
	msleep( 10 );
  }
}

void ReaderThread::startRead()
{
  //std::cerr << "start read" << std::endl;
  m_readValue = true;
  m_sendRequest = true;
}

void ReaderThread::socketNotifierSLOT( int socket )
{
  //std::cerr << "socket call" << std::endl;

  if (m_handle != socket)
	  return;

  if (-1 == m_handle)
  {
	m_status = ReaderThread::NotConnected;
	return;
  }

  int  retval;
  char byte;

  m_status = ReaderThread::Ok;

  {
	retval = ::read( m_handle, &byte, 1);

	if (-1 == retval)
	{
	  m_status = ReaderThread::Error;
	  return;
	}
	else if (0 == retval)
	{
	  m_status = ReaderThread::Timeout;
	  return;
	}
	else
	{
	  m_fifo[m_length] = byte;

	  //fprintf( stderr, "READ: %02X - %02X %d\n", (unsigned)byte, (unsigned)byte & 0xf0, m_length );

	  if (checkFormat())
	  {
		m_length = (m_length-formatLength()+1+FIFO_LENGTH)%FIFO_LENGTH;

		//fprintf( stderr, "Format Ok!\n" );

		for (int i=0; i<formatLength(); ++i)
		{
		  m_buffer[i] = m_fifo[m_length];
		  m_length = (m_length+1)%FIFO_LENGTH;
		}

		m_sendRequest = true;
		m_length = 0;

		Q_EMIT readEvent( m_buffer, m_id, m_format );
		//QApplication::postEvent( m_receiver,
		//    new ReadEvent( m_buffer, formatLength(), m_id, m_format ) );

		m_id = (m_id+1) % m_numValues;
	  }
	  else
		m_length = (m_length+1) % FIFO_LENGTH;
	}
  }
}

int  ReaderThread::formatLength() const
{
  switch (m_format)
  {
	  case ReadEvent::Metex14:
		return 14;
	  case ReadEvent::Voltcraft14Continuous:
		return 14;
	  case ReadEvent::Voltcraft15Continuous:
		return 15;
	  case ReadEvent::M9803RContinuous:
		return 11;
	  case ReadEvent::PeakTech10:
		return 11;
	  case ReadEvent::VC820Continuous:
		return 14;
	  case ReadEvent::IsoTech:
		return 22;
	  case ReadEvent::VC940Continuous:
		return 11;
	  case ReadEvent::QM1537Continuous:
		return 14;
	  case ReadEvent::RS22812Continuous:
		return 9;
  }
  return 0;
}

void ReaderThread::readDMM()
{
  m_id = 0;
  memset( m_buffer, 0, 20 );

  if (m_format == ReadEvent::Metex14)
	readMetex14();
  else if (m_format == ReadEvent::Voltcraft14Continuous)
	readVoltcraft14Continuous();
  else if (m_format == ReadEvent::Voltcraft15Continuous)
	readVoltcraft15Continuous();
  else if (m_format == ReadEvent::M9803RContinuous)
	readM9803RContinuous();
  else if (m_format == ReadEvent::PeakTech10)
	readPeakTech10();
  else if (m_format == ReadEvent::IsoTech)
	readIsoTech();
  else if (m_format == ReadEvent::QM1537Continuous)
	readQM1537Continuous();
  else if (m_format == ReadEvent::VC940Continuous)
	readVC940();
  else if (m_format == ReadEvent::RS22812Continuous)
	readRS22812Continuous();

}

bool ReaderThread::checkFormat()
{
  if (m_format == ReadEvent::Metex14)
  {
	if (m_fifo[m_length] == 0x0d)
		return true;
  }
  else if (m_format == ReadEvent::Voltcraft14Continuous)
  {
	if (m_fifo[m_length] == 0x0d)
		return true;
  }
  else if (m_format == ReadEvent::Voltcraft15Continuous)
  {
	if (m_fifo[(m_length-1+FIFO_LENGTH)%FIFO_LENGTH] == 0x0d && m_fifo[m_length] == 0x0a)
		return true;
  }
  else if (m_format == ReadEvent::M9803RContinuous && m_length >= 10)
  {
	if (m_fifo[(m_length-1+FIFO_LENGTH)%FIFO_LENGTH] == 0x0d && m_fifo[m_length] == 0x0a)
		return true;
  }
  else if (m_format == ReadEvent::VC940Continuous && m_length >= 12)
  {
	if (m_fifo[(m_length-1+FIFO_LENGTH)%FIFO_LENGTH] == 0x0d && m_fifo[m_length] == 0x0a)
		return true;
  }
  else if (m_format == ReadEvent::PeakTech10 && m_length >= 11)
  {
	if (m_fifo[(m_length-11+FIFO_LENGTH)%FIFO_LENGTH] == '#')
		return true;
  }
  else if (m_format == ReadEvent::VC820Continuous)// && m_length >= 13)
  {
	//fprintf( stderr, "CHECK: %02x %02x %s\n", m_fifo[m_length], (m_fifo[m_length] & 0xf0),
	//         ((m_fifo[m_length] & 0xf0) == 0xe0) ? "OK" : "BAD" );
	if ((m_fifo[m_length] & 0xf0) == 0xe0)
		return true;
	//if (m_fifo[(m_length-1+FIFO_LENGTH)%FIFO_LENGTH] & 0xf0 == 0xe0) return true;
  }
  else if (m_format == ReadEvent::IsoTech && m_length >= 22)
  {
	for (int i=0; i<11; ++i)
	{
	  if (m_fifo[m_length-22+i] != m_fifo[m_length-22+11+i])
		return false;
	}
	if (m_fifo[m_length-22+9] != 0x0d ||  m_fifo[m_length-22+10] != 0x0a)
		return false;
	return true;
  }
  else if (m_format == ReadEvent::QM1537Continuous)
  {
	if (m_fifo[m_length] == 0x0d)
		return true;
  }
  else if (m_format == ReadEvent::RS22812Continuous)
  {
	unsigned int checksum = 0x00;
	unsigned char byte;
	char mode, unit, multiplier, dp, minmax, rs232;
	int offset = 0;

	if (m_length > 9)
		offset = m_length - 9;
	// mode
	byte = (unsigned char)(m_fifo[offset]);
	mode = byte;
	if (mode > 25)
	{
	  (void)fprintf(stderr, "bad mode %#x\n", mode);
	  return false;
	}
	// units and multipliers
	unit = 0;
	multiplier = 0;
	minmax = 0;
	if (m_length >= 1)
	{
	  byte = (unsigned char)(m_fifo[offset + 1]);
	  if (byte & 0x01) // m
		multiplier++;
	  if (byte & 0x02) { // V
		unit++;
		if ((mode != 0) && (mode != 1) && (mode != 19) && (mode != 22))
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  if (byte & 0x04)
	  { // A
		unit++;
		if ((mode < 2) && (mode > 7))
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  if (byte & 0x08)
	  { // F
		unit++;
		if (mode != 9)
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  if (byte & 0x10) // M
		multiplier++;
	  if (byte & 0x20) // k
		multiplier++;
	  if (byte & 0x40)
	  { // ohms
		unit++;
		if (mode != 8)
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  if (byte & 0x80)
	  { // Hz
		unit++;
		if ((mode < 10) && (mode > 12))
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  if (unit > 1)
	  {
		(void)fprintf(stderr, "too many units in 2nd byte %#x\n", byte);
		return false;
	  }
	  if (multiplier > 1)
	  {
		(void)fprintf(stderr, "too many multipliers in 2nd byte %#x\n", byte);
		return false;
	  }
	  if (mode < 2)
	  {
		if (!(byte & 0x02))
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  else if (mode < 8)
	  {
		if (!(byte & 0x04))
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  else if (mode < 9)
	  {
		if (!(byte & 0x40))
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  else if (mode < 10)
	  {
		if (!(byte & 0x08))
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  else if (mode < 13)
	  {
		if (!(byte & 0x80))
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	}
	if (m_length >= 2)
	{
	  byte = (unsigned char)(m_fifo[offset + 2]);
	  if (byte & 0x01) // MIN
		minmax++;
	  if (byte & 0x02) // REL
		minmax++;
	  if (byte & 0x04)
	  { // hFE
		unit++;
		if (mode != 21)
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  if (byte & 0x10)
	  { // S
		unit++;
		if ((mode < 16) && (mode > 18))
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  if (byte & 0x20)
	  { // dBm
		unit++;
		if (mode != 23)
		{
		  (void)fprintf(stderr, "mode vs. units mismatch: mode %#x, units %#x\n", mode, byte);
		  return false;
		}
	  }
	  if (byte & 0x40) // n
		multiplier++;
	  if (byte & 0x80) // micro
		multiplier++;
	  if (unit > 1)
	  {
		(void)fprintf(stderr, "too many units in 3rd byte %#x\n", byte);
		return false;
	  }
	  if (multiplier > 1)
	  {
		(void)fprintf(stderr, "too many multipliers in 3rd byte %#x\n", byte);
		return false;
	  }
	  if (minmax > 1)
	  {
		(void)fprintf(stderr, "too many min/max/rel indications in 3rd byte %#x\n", byte);
		return false;
	  }
	}
	dp = 0;
	if (m_length >= 3)
	{
	  byte = (unsigned char)(m_fifo[offset + 3]);
	  if (byte & 0x08)
		dp++;
	  if (m_length >= 4)
	  {
		byte = (unsigned char)(m_fifo[offset + 4]);
		if (byte & 0x08)
		  dp++;
		if (m_length >= 5)
		{
		  byte = (unsigned char)(m_fifo[offset + 5]);
		  if (byte & 0x08)
			dp++;
		}
	  }
	}
	if (dp > 1)
		return false;
	if (m_length >= 6)
	{
	  byte = (unsigned char)(m_fifo[offset + 6]);
	  if (byte & 0x08) // MAX
		minmax++;
	  if (minmax > 1)
	  {
		(void)fprintf(stderr, "too many min/max/rel indications in 7th byte %#x\n", byte);
		return false;
	  }
	}
	if (m_length >= 7)
	{
	  rs232 = 0;
	  byte = (unsigned char)(m_fifo[offset + 7]);
	  if (byte & 0x02) // RS232
		rs232++;
	  if (rs232 != 1)
	  {
		(void)fprintf(stderr, "rs232 flag not set in 8th byte %#x\n", byte);
		return false;
	  }
	}
	if (m_length >= offset + 8)
	{
	  // XXX compute and validate checksum
	  for (int i=0; i<8; ++i)
	  {
		byte = (unsigned char)(m_fifo[offset + i] & 0x0ff);
		checksum += byte;
	  }
	  byte = (unsigned char)(m_fifo[offset + 8] & 0x0ff);
	  if (((checksum + 57) & 0x0ff) == byte)
		return true;
	  else
	  {
		(void)fprintf(stderr, "bad checksum byte %#x should be %#x\n", byte, (checksum + 57) & 0x0ff);
		return false;
	  }
	}
  }
  return false;
}

void ReaderThread::readMetex14()
{
  if (m_sendRequest)
  {
	/* TODO: Errorhandling */
	size_t ret = ::write( m_handle, "D\n", 2 );
	if (ret != 2) m_status = Error;
	//std::cerr << "WROTE: " << ret << std::endl;
	m_sendRequest = false;
  }
}

void ReaderThread::readVoltcraft14Continuous()
{
}


void ReaderThread::readVoltcraft15Continuous()
{
}

void ReaderThread::readM9803RContinuous()
{
}

void ReaderThread::readIsoTech()
{
}

void ReaderThread::readQM1537Continuous()
{
}

void ReaderThread::readVC820()
{
}

void ReaderThread::readVC940()
{
}

void ReaderThread::readPeakTech10()
{
}

void ReaderThread::readRS22812Continuous()
{
}
