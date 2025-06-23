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

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>


DMM::DMM(QObject *parent)
  : QObject( parent),
  m_handle( Q_NULLPTR ),
  m_speed( 600 ),
  m_parity( QSerialPort::NoParity ),
  m_stopBits(QSerialPort::OneStop),
  m_dataBits(QSerialPort::Data7),
  m_device( "" ),
  m_oldStatus( ReaderThread::NotConnected ),
  m_consoleLogging( false ),
  m_externalSetup( false ),
  m_dtr(false),
  m_rts(false),
  m_cts(false),
  m_dsr(false)
{
  m_readerThread = new ReaderThread( this );

  // mt: QThread emits a signal now. no more custom event
  connect( m_readerThread, SIGNAL(readEvent(const QByteArray &, int, ReadEvent::DataFormat)),
	   this, SLOT(readEventSLOT(const QByteArray &, int, ReadEvent::DataFormat)));

  m_readerThread->start();

}

void DMM::setPortSettings( QSerialPort::DataBits bits, QSerialPort::StopBits stopBits, QSerialPort::Parity parity, bool externalSetup,
						   bool rts, bool cts, bool dsr, bool dtr )
{
  m_externalSetup = externalSetup;
  m_parity  = parity;
  m_stopBits=stopBits;
  m_dataBits=bits;
  m_dtr=dtr;
  m_rts=rts;
  m_cts=cts;
  m_dsr=dsr;
}

void DMM::setFormat( ReadEvent::DataFormat format )
{
  m_readerThread->setFormat( format );
}

bool DMM::isOpen() const
{
	if(!m_handle)
		return false;
	return m_handle->isOpen();
}

void DMM::setSpeed( int speed )
{
  m_speed = speed;
}

void DMM::setDevice( const QString & device )
{
  m_device = device;
}

bool DMM::open()
{
  m_handle=new QSerialPort(this);
  m_handle->setPortName(m_device);

  if( !m_handle->open(QIODevice::ReadWrite))
  {
	int errorCode=m_handle->error();
	switch(errorCode)
	{
		case QSerialPort::PermissionError:
			m_error = tr( "Access denied for %1.").arg(m_device);
			break;
		case QSerialPort::DeviceNotFoundError:
			m_error = tr( "No such device %1." ).arg(m_device);
			break;
		default:
			m_error = tr( "Error opening %1.\nDMM connected and switched on?" ).arg(m_device);
			break;
	}
	Q_EMIT error( m_error );
	delete m_handle;
	m_handle=Q_NULLPTR;
	return false;
  }

  if (!m_externalSetup)
  {
	m_error = tr( "Error configuring serial port %1." ).arg(m_device);
	if((!m_handle->setParity(m_parity)) || (!m_handle->setBaudRate(m_speed)) || (!m_handle->setStopBits(m_stopBits)) ||
	   (!m_handle->setDataBits(m_dataBits)) || (!m_handle->setDataTerminalReady(m_dtr)) || (!m_handle->setRequestToSend(m_rts)))
	{
		Q_EMIT error( m_error );
		delete m_handle;
		m_handle=Q_NULLPTR;
		return false;
	}
  }
  m_error = tr( "Connecting ..." );
  Q_EMIT error( m_error );
  m_readerThread->setHandle( m_handle );
  timerEvent( 0 );

  // mt: added timer id
  m_delayTimer = startTimer( 1000 );
  return true;
}

void DMM::close()
{
  // mt: added timer id
  killTimer(m_delayTimer);

  m_error = tr( "Not connected" );
  Q_EMIT error( m_error );

  m_readerThread->setHandle( Q_NULLPTR );

  if (m_handle)
  {
	if (!m_externalSetup)
	{
	  m_handle->close();
	  delete m_handle;
	  m_handle=Q_NULLPTR;
	}
  }
  m_oldStatus = ReaderThread::NotConnected;
}

void DMM::timerEvent( QTimerEvent * )
{
  if (!m_handle)
	Q_EMIT error( m_error );
  else
	m_readerThread->startRead();
}

void DMM::readEventSLOT( const QByteArray & data, int id, ReadEvent::DataFormat df )
{
	if (ReaderThread::Ok == m_readerThread->status())
	{
	  if (m_consoleLogging)
	  {
		 for (int i=0; i<data.size(); ++i)
			fprintf( stdout, "%02X ", data[i] & 0x0ff );
		 fprintf( stdout, "\r\n" );
	  }
	  switch (df)
	  {
		  case ReadEvent::Metex14:
		  case ReadEvent::PeakTech10:
		  case ReadEvent::Voltcraft14Continuous:
		  case ReadEvent::Voltcraft15Continuous:
				readASCII( data, id, df );
			   break;
		  case ReadEvent::M9803RContinuous:
				readM9803RContinuous( data, id, df );
			   break;
		  case ReadEvent::VC820Continuous:
				readVC820Continuous( data, id, df );
			   break;
		  case ReadEvent::VC870Continuous:
				readVC870Continuous( data, id, df );
			   break;
		  case ReadEvent::IsoTech:
				readIsoTechContinuous( data, id, df );
			   break;
		  case ReadEvent::VC940Continuous:
				readVC940Continuous( data, id, df );
			   break;
		  case ReadEvent::QM1537Continuous:
				readQM1537Continuous( data, id, df );
			   break;
		  case ReadEvent::RS22812Continuous:
				readRS22812Continuous( data, id, df );
			   break;
          case ReadEvent::DO3122Continuous:
                readDO3122Continuous( data, id, df );
               break;
          case ReadEvent::CyrustekES51922:
                readCyrustekES51922( data, id, df );
               break;
	  }
	}
	else
	{
	  if (ReaderThread::Error == m_readerThread->status())
		  m_error = tr( "Read error on device %1.\nDMM connected and switched on?" ).arg(m_device);
	  else if (ReaderThread::Timeout == m_readerThread->status())
		  m_error = tr( "Timeout on device %1.\nDMM connected and switched on?" ).arg(m_device);
	  else if (ReaderThread::NotConnected == m_readerThread->status())
		  m_error = tr( "Not connected" );
	}
	if (m_oldStatus != m_readerThread->status())
		Q_EMIT error( m_error );
	m_oldStatus = m_readerThread->status();
}

QString DMM::insertComma( const QString & val, int pos )
{
  return val.left(2+pos) + "." + val.right(4-pos);
}

QString DMM::insertCommaIT( const QString & val, int pos )
{
  QString res;
  if (val[0] == '-' || val[0] == ' ')
	res = val.left(pos+1) + "." + val.mid(pos+1);
  else
	res = val.left(pos) + "." + val.mid(pos);
  return res;
}

void DMM::setNumValues( int numValues )
{
  m_readerThread->setNumValues( numValues );
}

void DMM::readVC940Continuous( const QByteArray & data, int id, ReadEvent::DataFormat /*df*/ )
{
  QString val = " ";
  QString special;
  QString unit;

  QString str(data);

  double scale = 1.0;

  int function = data[6] & 0x0f;
  int range    = data[5] & 0x0f;
  int mode     = data[7];
  int mode2    = data[8];

  if (function != 12 && mode2 & 0x04) val = "-";

  for (int i=0; i<4; ++i)
   val += str[i];
  if (data[4] != 'A')
	val += str[4];
  switch (function)
  {
	case 0:
	  val = insertCommaIT( val, 3 );
	  special = "AC";
	  unit = "mV";
	  scale = 1e-3;
	  break;
	case 1:
	  val = insertCommaIT( val, range );
	  special = "DC";
	  unit = "V";
	  break;
	case 2:
	  val = insertCommaIT( val, range );
	  special = "AC";
	  unit = "V";
	  break;
	case 3:
	  val = insertCommaIT( val, 3 );
	  special = "DC";
	  unit = "mV";
	  scale = 1e-3;
	  break;
	case 4:
	  unit = "Ohm";
	  switch (range)
	  {
		case 1:
		  val = insertCommaIT( val, 3 );
		  break;
		case 2:
		  val = insertCommaIT( val, 1 );
		  unit = "kOhm";
		  scale = 1e3;
		  break;
		case 3:
		  val = insertCommaIT( val, 2 );
		  unit = "kOhm";
		  scale = 1e3;
		  break;
		case 4:
		  val = insertCommaIT( val, 3 );
		  unit = "kOhm";
		  scale = 1e3;
		  break;
		case 5:
		  val = insertCommaIT( val, 1 );
		  unit = "MOhm";
		  scale = 1e6;
		  break;
		case 6:
		  val = insertCommaIT( val, 2 );
		  unit = "MOhm";
		  scale = 1e6;
		  break;
	  }
	  special = "OH";
	  break;
	case 5:
	  unit = "F";
	  switch (range)
	  {
		case 1:
		  val = insertCommaIT( val, 2 );
		  unit = "nF";
		  scale = 1e-9;
		  break;
		case 2:
		  val = insertCommaIT( val, 3 );
		  unit = "nF";
		  scale = 1e-9;
		  break;
		case 3:
		  val = insertCommaIT( val, 1 );
		  unit = "uF";
		  scale = 1e-6;
		  break;
		case 4:
		  val = insertCommaIT( val, 2 );
		  unit = "uF";
		  scale = 1e-6;
		  break;
		case 5:
		  val = insertCommaIT( val, 3 );
		  unit = "uF";
		  scale = 1e-6;
		  break;
		case 6:
		  val = insertCommaIT( val, 4 );
		  unit = "uF";
		  scale = 1e-6;
		  break;
		case 7:
		  val = insertCommaIT( val, 2 );
		  unit = "mF";
		  scale = 1e-3;
		  break;
	  }
	  special = "CA";
	  break;
	case 6:
	  special = "TE";
	  unit = "C";
	  val = insertCommaIT( val, 4 );
	  break;
	case 7:
	  if (mode & 0x01)
	  {
		// can't handle AC+DC
		special = "AC";
	  }
	  else
		  special = "DC";
	  switch (range)
	  {
		case 0:
		  val = insertCommaIT( val, 3 );
		  break;
		case 1:
		  val = insertCommaIT( val, 4 );
		  break;
	  }
	  unit = "uA";
	  scale = 1e-6;
	  break;
	case 8:
	  if (mode & 0x01)
	  {
		// can't handle AC+DC
		special = "AC";
	  }
	  else
		  special = "DC";
	  switch (range)
	  {
		case 0:
		  val = insertCommaIT( val, 2 );
		  break;
		case 1:
		  val = insertCommaIT( val, 3 );
		  break;
	  }
	  unit = "mA";
	  scale = 1e-3;
	  break;
	case 9:
	  if (mode & 0x01)
	  {
		// can't handle AC+DC
		special = "AC";
	  }
	  else
		  special = "DC";
	  val = insertCommaIT( val, 2 );
	  unit = "A";
	  break;
	case 10:   // buzzer
	  special = "BUZ";
	  unit = "Ohm";
	  val = insertCommaIT( val, 3 );
	  break;
	case 11:
	  special = "DI";
	  unit = "V";
	  val = insertCommaIT( val, 1 );
	  break;
	case 12:
	  if (mode2 & 0x04)
	  {
		special = "PC";
		unit = "%";
		val = insertCommaIT( val, 3 );
	  }
	  else
	  {
		special = "FR";
		unit = "Hz";
		switch (range)
		{
		  case 0:
			val = insertCommaIT( val, 2 );
			break;
		  case 1:
			val = insertCommaIT( val, 3 );
			break;
		  case 2:
			val = insertCommaIT( val, 1 );
			unit = "kHz";
			scale = 1e3;
			break;
		  case 3:
			val = insertCommaIT( val, 2 );
			unit = "kHz";
			scale = 1e3;
			break;
		  case 4:
			val = insertCommaIT( val, 3 );
			unit = "kHz";
			scale = 1e3;
			break;
		  case 5:
			val = insertCommaIT( val, 1 );
			unit = "MHz";
			scale = 1e6;
			break;
		  case 6:
			val = insertCommaIT( val, 2 );
			unit = "MHz";
			scale = 1e6;
			break;
		  case 7:
			val = insertCommaIT( val, 3 );
			unit = "MHz";
			scale = 1e6;
			break;
		}
	  }
	  break;
	case 13:
	  special = "TE";
	  unit = "F";
	  val = insertCommaIT( val, 4 );
	  break;

  }

  double d_val = val.toDouble() * scale;

  //printf( "d_val=%f val=%s unit=%s special=%s\n", d_val, val.latin1(), unit.latin1(), special.latin1() );

  Q_EMIT value( d_val, val, unit, special, "", false, true, id );
  m_error = tr( "Connected %1" ).arg(m_device);
}

void DMM::readRS22812Continuous( const QByteArray & data, int id, ReadEvent::DataFormat /*df*/ )
{
  QString val;
  QString special;
  QString unit;

  const char *in = data.data();

  if (m_consoleLogging)
  {
	for (int i=0; i<data.size(); ++i)
		fprintf( stdout, "%02X ", in[i] & 0x0ff );
	fprintf( stdout, "\n" );
  }

  // check for overflow else find sign and fill in digits
  //
  if (((in[3] & 0x0f7) == 0x000) &&
	  ((in[4] & 0x0f7) == 0x027) &&
	  ((in[5] & 0x0f7) == 0x0d7))
  {
	val = "   0L ";
  }
  else
  {
	if(in[7] & 0x08)
		val = " -";   // negative;
	else
		val = "  ";
	// create string;
	//
	for (int i=0; i<4; ++i)
		val += RS22812Digit( in[6-i] );
  }

  // find comma (really decimal point) [germans use commas instead of decimal points] position
  //
  if (in[3] & 0x08)
	val = insertComma( val, 3 );
  else if (in[4] & 0x08)
	val = insertComma( val, 2 );
  else if(in[5] & 0x08)
	val = insertComma( val, 1 );

  double d_val = val.toDouble();

  // try to find some special modes
  //
  if (in[7] & 0x40)
	special = "DI";
  if (in[7] & 0x04)
	special = "AC";
  else
	special = "DC";

  // try to find mode
  //
  if (in[1] & 0x08)
  {
	unit    = "F";
	special = "CA";
  }
  else if (in[1] & 0x40)
  {
	unit    = "Ohm";
	special = "OH";
  }
  else if (in[1] & 0x04)
	unit = "A";
  else if (in[1] & 0x80)
  {
	unit    = "Hz";
	special = "HZ";
  }
  else if (in[1] & 0x02)
	unit = "V";
  else if (in[2] & 0x80)
  {
	unit    = "%";
	special = "PC";
  }
  else
	std::cerr << "Unknown unit!" << std::endl;

  // try to find prefix
  //
  if (in[2] & 0x40)
  {
	d_val /= 1e9;
	unit.prepend( "n" );
  }
  else if (in[2] & 0x80)
  {
	d_val /= 1e6;
	unit.prepend( "u" );
  }
  else if (in[1] & 0x01)
  {
	d_val /= 1e3;
	unit.prepend( "m" );
  }
  else if (in[1] & 0x20)
  {
	d_val *= 1e3;
	unit.prepend( "k" );
  }
  else if (in[1] & 0x10)
  {
	d_val *= 1e6;
	unit.prepend( "M" );
  }

  Q_EMIT value( d_val, val, unit, special, "", false, true, id );

  m_error = tr( "Connected %1" ).arg(m_device);
}

const char *DMM::RS22812Digit( int byte )
{
  int           digit[10] = { 0xd7, 0x50, 0xb5, 0xf1, 0x72, 0xe3, 0xe7, 0x51, 0xf7, 0xf3 };
  const char *c_digit[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

  byte &= 0x0f7;

  for (int n=0; n<10; n++)
  {
	if (byte == digit[n])
		return c_digit[n];
  }
  return 0;
}

// mt: not using ReadEvent any more
void DMM::readASCII( const QByteArray & data, int id, ReadEvent::DataFormat df )
{
  QString val;
  QString special;
  QString unit;

  QString str(data);

  if (df == ReadEvent::Metex14 ||
	  df == ReadEvent::Voltcraft14Continuous ||
	  df == ReadEvent::Voltcraft15Continuous)
  {
	val     = str.mid( 2, 7 ); //.stripWhiteSpace();
	unit    = str.mid( 9, 4 ).trimmed();
	special = str.left( 3 ).trimmed();
  }
  else if (df == ReadEvent::PeakTech10)
  {
	val     = str.mid( 1, 6 ); //.stripWhiteSpace();
	unit    = str.mid( 7, 4 ).trimmed();
  }
  double d_val = val.toDouble();
   //std::cerr << "d_val=" << d_val << " val=" << val.latin1()
  //    << " unit=" << unit.latin1() << " special=" << special.latin1() << std::endl;

  if (unit.left(1) == "p")
	d_val /= 1.0E12;
  else if (unit.left(1) == "n")
	d_val /= 1.0E9;
  else if (unit.left(1) == "u")
	d_val /= 1.0E6;
  else if (unit.left(1) == "m")
	d_val /= 1.0E3;
  else if (unit.left(1) == "k")
	d_val *= 1.0E3;
  else if (unit.left(1) == "M")
	d_val *= 1.0E6;
  else if (unit.left(1) == "G")
	d_val *= 1.0E9;
  Q_EMIT value( d_val, val, unit, special, "", false, true, id );

 m_error = tr( "Connected %1" ).arg(m_device);
}

void DMM::readCyrustekES51922(const QByteArray & data, int id, ReadEvent::DataFormat /*df*/)
{
  QString val;
  QString special;
  QString unit;
  QString power;
  QString range;
  bool batteryLow;
  bool relative;
  bool hold;

  const char *pStr = data.data();

  const size_t CyrustekES51922_Range = 0;
  const size_t CyrustekES51922_Function = 6;
  const size_t CyrustekES51922_Status = 7;
  const size_t CyrustekES51922_Option1 = 8;
  const size_t CyrustekES51922_Option2 = 9;
  const size_t CyrustekES51922_Option3 = 10;
  const size_t CyrustekES51922_Option4 = 11;

  if (pStr[CyrustekES51922_Status] & 0x04) {
    val = "-";
  }
  else {
    val = " ";
  }

  val += pStr[1];
  val += pStr[2];
  val += pStr[3];
  val += pStr[4];
  val += pStr[5];

  double dVal = val.toDouble();
  double dMult = 1.0;

  switch (pStr[CyrustekES51922_Function]) {
  case 0x30:
    unit = "A";

    switch (pStr[CyrustekES51922_Range]) {
    case 0x30:
      dMult = 1.0e-3;
      break;
    }
    break;

  case 0x31:
    // Diode
    unit = "V";
    special = "DI";
    dMult = 1.0e-3;
    break;

  case 0x32:
    if (pStr[CyrustekES51922_Status] & 8)
      unit = "%";
    else
      unit = "Hz";

    switch (pStr[CyrustekES51922_Range]) {
    case 0x30:
      dMult = 1.0e-2;
      break;
    case 0x31:
      dMult = 1.0e-1;
      break;
    case 0x33:
      dMult = 1.0;
      break;
    case 0x34:
      dMult = 1.0e1;
      break;
    case 0x35:
      dMult = 1.0e2;
      break;
    case 0x36:
      dMult = 1.0e3;
      break;
    case 0x37:
      dMult = 1.0e4;
      break;
    }
    break;

  case 0x33:
    unit = "Ohm";

    switch (pStr[CyrustekES51922_Range]) {
    case 0x30:
      dMult = 1.0e-2;
      break;
    case 0x31:
      dMult = 1.0e-1;
      break;
    case 0x32:
      dMult = 1.0;
      break;
    case 0x33:
      dMult = 1.0e1;
      break;
    case 0x34:
      dMult = 1.0e2;
      break;
    case 0x35:
      dMult = 1.0e3;
      break;
    case 0x36:
      dMult = 1.0e4;
      break;
    }
    // printf("Found OHM reading! dVal= %f  dMult= %f\n", dVal, dMult);

    break;

  case 0x34:
    unit = "degC";
    break;

  case 0x35:
    unit = "CONDUCTANCE";
    break;

  case 0x36:
    unit = "F";

    switch (pStr[CyrustekES51922_Range]) {
    case 0x30:
      dMult = 1.0e-12;
      break;
    case 0x31:
      dMult = 1.0e-11;
      break;
    case 0x32:
      dMult = 1.0e-10;
      break;
    case 0x33:
      dMult = 1.0e-9;
      break;
    case 0x34:
      dMult = 1.0e-8;
      break;
    case 0x35:
      dMult = 1.0e-7;
      break;
    case 0x36:
      dMult = 1.0e-6;
      break;
    case 0x37:
      dMult = 1.0e-5;
      break;
    }
    break;

  case 0x3b:
    unit = "V";

    switch (pStr[CyrustekES51922_Range]) {
    case 0x30:
      dMult = 1.0e-4;
      break;
    case 0x31:
      dMult = 1.0e-3;
      break;
    case 0x32:
      dMult = 1.0e-2;
      break;
    case 0x33:
      dMult = 1.0e-1;
      break;
    case 0x34:
      dMult = 1.0e-5;
      break;
    }
    break;

  case 0x3d: // uA
    unit = "A";

    switch (pStr[CyrustekES51922_Range]) {
    case 0x30:
      dMult = 1.0e-8;
      break;
    case 0x31:
      dMult = 1.0e-7;
      break;
    }
    break;

  case 0x3f: // mA
    unit = "A";

    switch (pStr[CyrustekES51922_Range]) {
    case 0x30:
      dMult = 1.0e-6;
      break;
    case 0x31:
      dMult = 1.0e-5;
      break;
    }
    break;
  }

  if (pStr[CyrustekES51922_Status] & 1) {
    val = "  OL  ";
    dVal = NAN;
  }
  else if (pStr[CyrustekES51922_Option2] & 8) {
    val = "  UL  ";
    dVal = NAN;
  }
  else {
    while (fabs(dVal) >= 1.0) {
      dVal /= 10.0;
      dMult *= 10.0;
    }

    double displayValue = 0;

    if (dMult >= 1.0e9) {
      unit.prepend("G");
      displayValue = dVal * (dMult / 1.0e9);
    }
    else if (dMult >= 1.0e6) {
      unit.prepend("M");
      displayValue = dVal * (dMult / 1.0e6);
    }
    else if (dMult >= 1.0e3) {
      unit.prepend("k");
      displayValue = dVal * (dMult / 1.0e3);
    }
    else if (dMult >= 1.0e0) {
      displayValue = dVal * (dMult / 1.0e0);
    }
    else if (dMult >= 1.0e-3) {
      unit.prepend("m");
      displayValue = dVal * (dMult / 1.0e-3);
    }
    else if (dMult >= 1.0e-6) {
      unit.prepend("u");
      displayValue = dVal * (dMult / 1.0e-6);
    }
    else if (dMult >= 1.0e-9) {
      unit.prepend("n");
      displayValue = dVal * (dMult / 1.0e-9);
    }
    else if (dMult >= 1.0e-12) {
      unit.prepend("p");
      displayValue = dVal * (dMult / 1.0e-12);
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(4) << displayValue;
    val = QString::fromStdString(ss.str());

    if (dVal < 0.0) {
      val = val.left(7);
    }
    else {
      val = val.left(6);
    }

    dVal *= dMult;
  }

  batteryLow = pStr[CyrustekES51922_Status] & 2;
  relative = pStr[CyrustekES51922_Option1] & 2;

  if (pStr[CyrustekES51922_Option3] & 2)
  {
    range = "AUTO";
  }
  else
  {
    range = "MANU";
  };

  if (pStr[CyrustekES51922_Option3] & 8)
    special = "DC";
  else if (pStr[CyrustekES51922_Option3] & 4)
    special = "AC";

  hold = pStr[CyrustekES51922_Option4] & 2;

  Q_EMIT value(dVal, val, unit, special, range, hold, true, id);
  m_error = tr("Connected %1").arg(m_device);
}


void DMM::readQM1537Continuous( const QByteArray & data, int id, ReadEvent::DataFormat /*df*/ )
{
  QString val;
  QString special;
  QString unit;
  const char *pStr = data.data();

  if (pStr[0]!=0x0A)
	  return;
  if (pStr[1] == '-')
	val = " -";
  else
	val = "  ";

  if ((pStr[2] == ';') &&
	  (pStr[3] == '0') &&
	  (pStr[4] == ':') &&
	  (pStr[5] == ';'))
  {
	 val += "  0L";
  }
  else
  {
	val += pStr[2];
	val += pStr[3];
	val += pStr[4];
	val += pStr[5];
  }

  bool showBar = true;
  bool doACDC = false;
  bool doUnits = true;

  switch (pStr[7])
  {
	case 0x31:
	  val = insertComma (val,1);
	  break;
	case 0x32:
	  val = insertComma (val,2);
	  break;
	case 0x34:
	  val = insertComma (val,3);
	  break;
	// default case is no comma/decimal point at all.
  }

  double d_val = val.toDouble();

  /* OK, now let's figure out what we're looking at. */
  if (pStr[11] & 0x80)
  {
	/* Voltage, including diode test */
	unit = "V";
	if (pStr[10] & 0x04)
	{
	  /* Diode test */
	  special = "DI";
	  unit = "V";
	}
	else
	  doACDC = true;
  }
  else if (pStr[11] & 0x40)
  {
	/* Current */
	unit = "A";
	doACDC = true;
  }
  else if (pStr[11] & 0x20)
  {
	/* Resistance, including continuity test */
	unit = "Ohm";
	special = "OH";
  }
  else if (pStr[11] & 0x08)
  {
	/* Frequency */
	unit = "Hz";
	special = "HZ";
  }
  else if (pStr[11] & 0x04)
  {
	/* Capacitance */
	unit = "F";
	special = "CA";
  }
  else if (pStr[10] & 0x02)
  {
	/* Duty cycle */
	unit = "%";
	special = "PC";
	doUnits = false;
  }

  if (doACDC)
  {
	if (pStr[8] & 0x08)
	  special = "AC";
	else
	  special = "DC";
  }

  if (doUnits)
  {
	if (pStr[9] & 0x02)
	{
	  d_val /= 1e9;
	  unit.prepend ('n');
	}
	else if (pStr[10] & 0x80)
	{
	  d_val /= 1e6;
	  unit.prepend ('u');
	}
	else if (pStr[10] & 0x40)
	{
	  d_val /= 1e3;
	  unit.prepend ('m');
	}
	else if (pStr[10] & 0x20)
	{
	  d_val *= 1e3;
	  unit.prepend ('k');
	}
	else if (pStr[10] & 0x10)
	{
	  d_val *= 1e6;
	  unit.prepend ('M');
	}
  }

  Q_EMIT value( d_val, val, unit, special, "", false, showBar, id );
  m_error = tr( "Connected %1" ).arg(m_device);
}


void DMM::readM9803RContinuous( const QByteArray & data, int id, ReadEvent::DataFormat /*df*/ )
{
  QString val;
  QString special;
  QString unit;

  if (data[0] & 0x01)
	val = "  0L";
  else
  {
	val = data[0] == 0x08 ? " -" : "  ";
	val += QChar( '0'+data[4] );
	val += QChar( '0'+data[3] );
	val += QChar( '0'+data[2] );
	val += QChar( '0'+data[1] );
  }

  double d_val = val.toDouble();

  bool showBar = true;

  switch (data[5])
  {
	case 0x00:
	  switch (data[6])
	  {
		case 0x00:
		  unit = "mV";
		  val = insertComma( val, 3 );
		  d_val /= 10000.;
		  break;
		case 0x01:
		  unit = "V";
		  val = insertComma( val, 1 );
		  d_val /= 1000.;
		  break;
		case 0x02:
		  unit = "V";
		  val = insertComma( val, 2 );
		  d_val /= 100.;
		  break;
		case 0x03:
		  unit = "V";
		  val = insertComma( val, 3 );
		  d_val /= 10.;
		  break;
		case 0x04:
		  unit = "V";
		  break;
	  }
	  special = "DC";
	  break;
	case 0x01:
	  switch (data[6])
	  {
		case 0x00:
		  unit = "mV";
		  val = insertComma( val, 3 );
		  d_val /= 10000.;
		  break;
		case 0x01:
		  unit = "V";
		  val = insertComma( val, 1 );
		  d_val /= 1000.;
		  break;
		case 0x02:
		  unit = "V";
		  val = insertComma( val, 2 );
		  d_val /= 100.;
		  break;
		case 0x03:
		  unit = "V";
		  val = insertComma( val, 3 );
		  d_val /= 10.;
		  break;
		case 0x04:
		  unit = "V";
		  break;
	  }
	  special = "AC";
	  break;
	case 0x02:
	  switch (data[6])
	  {
		case 0x00:
		  unit = "mA";
		  val = insertComma( val, 1 );
		  d_val /= 1000.;
		  break;
		case 0x01:
		  unit = "mA";
		  val = insertComma( val, 2 );
		  d_val /= 100.;
		  break;
		case 0x02:
		  unit = "mA";
		  val = insertComma( val, 3 );
		  d_val /= 10.;
		  break;
	  }
	  special = "DC";
	  break;
	case 0x03:
	  switch (data[6])
	  {
		case 0x00:
		  unit = "mA";
		  val = insertComma( val, 1 );
		  d_val /= 1000.;
		  break;
		case 0x01:
		  unit = "mA";
		  val = insertComma( val, 2 );
		  d_val /= 100.;
		  break;
		case 0x02:
		  unit = "mA";
		  val = insertComma( val, 3 );
		  d_val /= 10.;
		  break;
	  }
	  special = "AC";
	  break;
	case 0x04:
	  switch (data[6])
	  {
		case 0x00:
		  unit = "Ohm";
		  val = insertComma( val, 1 );
		  d_val /= 10.;
		  break;
		case 0x01:
		  unit = "kOhm";
		  val = insertComma( val, 1 );
		  d_val /= 1000.;
		  break;
		case 0x02:
		  unit = "kOhm";
		  val = insertComma( val, 2 );
		  d_val /= 100.;
		  break;
		case 0x03:
		  unit = "kOhm";
		  val = insertComma( val, 3 );
		  d_val /= 10.;
		  break;
		case 0x04:
		  unit = "kOhm";
		  break;
		case 0x05:
		  unit = "MOhm";
		  val = insertComma( val, 2 );
		  d_val /= 100.;
		  break;
	  }
	  special = "OH";
	  break;
	case 0x05:
	  switch (data[6])
	  {
		case 0x00:
		  unit = "Ohm";
		  val = insertComma( val, 3 );
		  d_val /= 10.;
		  break;
	  }
	  special = "OH";
	  break;
	case 0x06:
	  unit = "V";
	  val = insertComma( val, 1 );
	  d_val /= 1000.;
	  special = "DI";
	  break;
	case 0x08:
	  unit = "A";
	  val = insertComma( val, 2 );
	  d_val /= 100.;
	  special = "DC";
	  break;
	case 0x09:
	  unit = "A";
	  val = insertComma( val, 2 );
	  d_val /= 100.;
	  special = "AC";
	  break;
	case 0x0A:
	  showBar = false;
	  switch (data[6])
	  {
		case 0x00:
		  unit = "kHz";
		  val = insertComma( val, 1 );
			//d_val /= 1000.;
		  break;
		case 0x01:
		  unit = "kHz";
		  val = insertComma( val, 2 );
		  d_val *= 10.;
		  break;
		case 0x05:
		  unit = "Hz";
		  val = insertComma( val, 2 );
		  d_val /= 100.;
		  break;
		case 0x06:
		  unit = "Hz";
		  val = insertComma( val, 3 );
		  d_val /= 10.;
		  break;
	  }
	  special = "HZ";
	  break;
	case 0x0C:
	  switch (data[6])
	  {
		case 0x00:
		  unit = "nF";
		  val = insertComma( val, 1 );
		  d_val /= 1e12;
		  break;
		case 0x01:
		  unit = "nF";
		  val = insertComma( val, 2 );
		  d_val /= 1e11;
		  break;
		case 0x02:
		  unit = "nF";
		  val = insertComma( val, 3 );
		  d_val /= 1e10;
		  break;
		case 0x03:
		  unit = "uF";
		  val = insertComma( val, 1 );
		  d_val /= 1e9;
		  break;
		case 0x04:
		  unit = "uF";
		  val = insertComma( val, 2 );
		  d_val /= 1e8;
		  break;
	  }
	  special = "CA";
	  break;
  }

  Q_EMIT value( d_val, val, unit, special, "", false, showBar, id );
  m_error = tr( "Connected %1" ).arg(m_device);
}

void DMM::readIsoTechContinuous(const QByteArray & data, int id, ReadEvent::DataFormat /*df*/)
{
  QString tmp(data);
  QString val = tmp.mid(11,4);
  QString unit, special;
  const char* tmpstring = data.data();

  double d_val = val.toDouble();

  // Type of measurement
  int typecode = tmpstring[15] & 0xf;
  int rangecode = tmpstring[10] & 0xf;
  int acdccode = tmpstring[18] & 0xc;
  //  printf("ISO: value %s, type: %d, range: %d, ac/dc: %c\n",
  //      re->string(), typecode, rangecode, acdccode == 0x8 ? 'D': 'A' );

  double multiplier = 1.0;

  switch(typecode)
  {
	case 0xb:
	  // voltage
	  if (rangecode < 4)
		rangecode += 5;
	  rangecode -= 4;
	  multiplier = pow(10, rangecode - 4);
	  if (acdccode == 0x8)
		special = "DC";
	  else
		 special = "AC";
	  switch (rangecode)
	  {
		case 0:
		  val = insertCommaIT(val, 3);
		  unit = "mV";
		  break;
		case 1:
		  val = insertCommaIT(val, 1);
		  unit = "V";
		  break;
		case 2:
		  val = insertCommaIT(val, 2);
		  unit = "V";
		  break;
		case 3:
		  val = insertCommaIT(val, 3);
		  unit = "V";
		  break;
		case 4:
		  unit = "V";
		  break;
		default:
		  // error
			  ;
	  }
	  break;
	case 0x3:
	  // resistance
	  multiplier = pow(10,rangecode - 1);
	  special = "OH";
	  switch (rangecode)
	  {
		case 0:
		  val = insertCommaIT(val, 3);
		  unit = "Ohm";
		  break;
		case 1:
		  val = insertCommaIT(val, 1);
		  unit = "kOhm";
		  break;
		case 2:
		  val = insertCommaIT(val, 2);
		  unit = "kOhm";
		  break;
		case 3:
		  val = insertCommaIT(val, 3);
		  unit = "kOhm";
		  break;
		case 4:
		  val = insertCommaIT(val, 1);
		  unit = "MOhm";
		  break;
		case 5:
		  val = insertCommaIT(val, 2);
		  unit = "MOhm";
		  break;
		default:
		  // error
		  ;
	  }
	  break;
   case 0x1:
	  // diode
	  multiplier = 0.001;
	  val = insertCommaIT(val,1);
	  unit = "V";
	  break;
   case 0xd:
	  // current micro
	  multiplier = pow(10,rangecode - 7);
	  special = "DC";
	  if (rangecode == 0)
	  {
		val = insertCommaIT(val,3);
		unit = "uA";
	  }
	  else
	  {
		val = insertCommaIT(val,1);
		unit = "mA";
	  }
	  break;
   case 0x0:
	  // current
	  multiplier = pow(10,rangecode - 3);
	  unit = "A";
	  val = insertCommaIT(val,rangecode + 1);
	  if (acdccode == 0x8)
		special = "DC";
	  else
		special = "AC";
	  break;
   case 0x2:
	  // frequency
	  special = "HZ";
	  multiplier = pow(10,rangecode);
	  switch (rangecode)
	  {
		case 0x0:
		  val = insertCommaIT(val, 1);
		  unit = "kHz";
		  break;
		case 0x1:
		  val = insertCommaIT(val, 2);
		  unit = "kHz";
		  break;
		case 0x2:
		  val = insertCommaIT(val, 3);
		  unit = "kHz";
		  break;
		case 0x3:
		  val = insertCommaIT(val, 1);
		  unit = "MHz";
		  break;
		case 0x4:
		  val = insertCommaIT(val, 2);
		  unit = "MHz";
		  break;
		default:
		  // error
		  ;

	  }
	  break;
   case 0x6:
	  // capacity
	  special = "CA";
	  multiplier = pow(10, rangecode - 12);
	  switch (rangecode)
	  {
		case 0x0:
		  val = insertCommaIT(val, 1);
		  unit = "nF";
		  break;
		case 0x1:
		  val = insertCommaIT(val, 2);
		  unit = "nF";
		  break;
		case 0x2:
		  val = insertCommaIT(val, 3);
		  unit = "nF";
		  break;
		case 0x3:
		  val = insertCommaIT(val, 1);
		  unit = "uF";
		  break;
		case 0x4:
		  val = insertCommaIT(val, 2);
		  unit = "uF";
		  break;
		case 0x5:
		  val = insertCommaIT(val, 3);
		  unit = "uF";
		  break;
		case 0x6:
		  val = insertCommaIT(val, 1);
		  unit = "mF";
		  break;
		default:
		  // error
		  ;
	  }
	  break;
   default:
	  // error - unknown type of measurement
	  ;
   }
  d_val *= multiplier;

  if (tmpstring[16] & 0x01)
	val = "  0L";
  if (tmpstring[16] & 0x4)
  {
	d_val = - d_val;
	val = " -" + val;
  }
  else
	val = "  " + val;

 // printf ("DVAL: %f\n", d_val);

  Q_EMIT value( d_val, val, unit, special, "", false, true, id );
  m_error = tr( "Connected %1" ).arg(m_device);
}

void DMM::readVC820Continuous( const QByteArray & data, int id, ReadEvent::DataFormat /*df*/ )
{
  QString val;
  QString special;
  QString unit;

  const char *in = data.data();

  // check for overload else find sign and fill in digits
  //
  if (((in[3] & 0x07) == 0x07) &&
	  ((in[4] & 0x0f) == 0x0d) &&
	  ((in[5] & 0x07) == 0x06) &&
	  ((in[6] & 0x0f) == 0x08))
  {
	val = "  0L";
  }
  else
  {
	if(in[1] & 0x08)
		val = " -";   // negative;
	else
		val = "  ";
	// create string;
	//
	for (int i=0; i<4; ++i)
		val += vc820Digit( ((in[1+2*i] << 4 ) & 0xf0) | (in[2+2*i] & 0x0f) );
  }

  // find comma position
  //
  if (in[3] & 0x08)
	val = insertComma( val, 1 );
  else if (in[5] & 0x08)
	val = insertComma( val, 2 );
  else if(in[7] & 0x08)
	val = insertComma( val, 3 );

  double d_val = val.toDouble();

  // try to find some special modes
  //
  if (in[9] & 0x01)
	special = "DI";
  if (in[0] & 0x08)
	special = "AC";
  else
	special = "DC";

  // try to find mode
  //
  if (in[11] & 0x08)
  {
	unit    = "F";
	special = "CA";
  }
  else if (in[11] & 0x04)
  {
	unit    = "Ohm";
	special = "OH";
  }
  else if (in[12] & 0x08)
	 unit = "A";
  else if (in[12] & 0x02)
  {
	unit    = "Hz";
	special = "HZ";
  }
  else if (in[12] & 0x04)
	unit = "V";
  else if (in[10] & 0x04)
  {
	unit    = "%";
	special = "PC";
  }
  else if (in[13] & 0x01)
  {
	unit    = "C";
	special = "TE";
  }
  else
	std::cerr << "Unknown unit!" << std::endl;

  // try to find prefix
  //
  if (in[9] & 0x04)
  {
	d_val /= 1e9;
	unit.prepend( "n" );
  }
  else if (in[9] & 0x08)
  {
	d_val /= 1e6;
	unit.prepend( "u" );
  }
  else if (in[10] & 0x08)
  {
	d_val /= 1e3;
	unit.prepend( "m" );
  }
  else if (in[9] & 0x02)
  {
	d_val *= 1e3;
	unit.prepend( "k" );
  }
  else if (in[10] & 0x02)
  {
	d_val *= 1e6;
	unit.prepend( "M" );
  }

  Q_EMIT value( d_val, val, unit, special, "", false, true, id );

  m_error = tr( "Connected %1" ).arg(m_device);
}

const char *DMM::vc820Digit( int byte )
{
  int           digit[10] = { 0x7d, 0x05, 0x5b, 0x1f, 0x27, 0x3e, 0x7e, 0x15, 0x7f, 0x3f };
  const char *c_digit[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

  byte &= 0x7f;

  for (int n=0; n<10; n++)
  {
	if (byte == digit[n])
		return c_digit[n];
  }
  return 0;
}

void DMM::readVC870Continuous( const QByteArray & data, int /*id*/, ReadEvent::DataFormat /*df*/ ) {
    // Support for the Voltcraft VC870 digital multimeter was contributed by Florian Evers, florian-evers@gmx.de, under the "GPLv3 or later" licence
    QString val1;
    QString val2;
    QString special;
    QString unit1;
    QString unit2;
    const char *in = data.data();

    // Obtain units and range
    const uint8_t l_FunctionCode = in[0];
    const uint8_t l_FunctionSelectCode = in[1];
    const uint8_t l_FactorIndex = (in[2] & 0x0f); // Main Display Range
    double l_Factor  = 0.0;
    double l_Factor2 = 0.0;
    int    l_DotPos  = 0;
    int    l_DotPos2 = 0;
    bool   l_bParserError = false;
    bool   l_bShowBar = true;
    if (l_FunctionCode == 0x30) {
        // Measurement mode: DCV / ACV
        unit1 = "V";
        unit2 = "V";
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: DCV
            special = "DC";
            switch (l_FactorIndex) {
                case 0: // 4 V
                    l_Factor = 1e-4;
                    l_DotPos = 2;
                    break;
                case 1:
                    // 40 V
                    l_Factor = 1e-3;
                    l_DotPos = 3;
                    break;
                case 2:
                    // 400 V
                    l_Factor = 1e-2;
                    l_DotPos = 4;
                    break;
                case 3:
                    // 4000 V (but 1000 V is the maximum)
                    l_Factor = 1e-1;
                    l_DotPos = 5;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else if (l_FunctionSelectCode == 0x31) {
            // Measurement mode: ACV
            special = "AC";
            switch (l_FactorIndex) {
                case 0:
                    // 4 V
                    l_Factor = 1e-3;
                    l_DotPos = 3;
                    break;
                case 1:
                    // 40 V
                    l_Factor = 1e-2;
                    l_DotPos = 4;
                    break;
                case 2:
                    // 400 V
                    l_Factor = 1e-1;
                    l_DotPos = 5;
                    break;
                case 3:
                    // 4000 V (but 1000 V is the maximum)
                    l_Factor = 1;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else if (l_FunctionCode == 0x31) {
        // Measurement mode: DCmV / Celsius
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: DCmV
            special    = "DC";
            unit1      = "mV";
            unit2      = "mV";
            switch (l_FactorIndex) {
                case 0:
                    // 400 mV
                    l_Factor = 1e-5;
                    l_DotPos = 4;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else if (l_FunctionSelectCode == 0x31) {
            // Measurement mode: degrees celsius and fahrenheit
            special    = "TE";
            unit1      = "C";
            unit2      = "F";
            l_bShowBar = false;
            switch (l_FactorIndex) {
                case 0:
                    // 4000 degrees C
                    l_Factor = 1e-1;
                    l_DotPos = 5;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else if (l_FunctionCode == 0x32) {
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: Resistance OHM
            special = "OH";
            switch (l_FactorIndex) {
                case 0:
                    // 400 Ohm
                    l_Factor = 1e-2;
                    l_DotPos = 4;
                    unit1    = "Ohm";
                    unit2    = "Ohm";
                    break;
                case 1:
                    // 4 kOhm
                    l_Factor = 1e-1;
                    l_DotPos = 2;
                    unit1    = "kOhm";
                    unit2    = "kOhm";
                    break;
                case 2:
                    // 40 kOhm
                    l_Factor = 1e1;
                    l_DotPos = 4;
                    unit1    = "kOhm";
                    unit2    = "kOhm";
                    break;
                case 3:
                    // 400 kOhm
                    l_Factor = 1e2;
                    l_DotPos = 5;
                    unit1    = "kOhm";
                    unit2    = "kOhm";
                    break;
                case 4:
                    // 4 MOhm
                    l_Factor = 1e3;
                    l_DotPos = 3;
                    unit1    = "MOhm";
                    unit2    = "MOhm";
                    break;
                case 5:
                    // 40 MOhm
                    l_Factor = 1e4;
                    l_DotPos = 4;
                    unit1    = "MOhm";
                    unit2    = "MOhm";
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else if (l_FunctionSelectCode == 0x31) {
            // Measurement mode: Short-circuit test CTN
            unit1   = "Ohm";
            unit2   = "Ohm";
            special = "OH";
            l_bShowBar = false;
            switch (l_FactorIndex) {
                case 0:
                    // 400 Ohm
                    l_Factor = 1e-2;
                    l_DotPos = 4;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else if (l_FunctionCode == 0x33) {
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: Capacitance CAP
            special = "CA";
            l_bShowBar = false;
            switch (l_FactorIndex) {
                case 0:
                    // 40 nF
                    l_Factor = 1e-12;
                    l_DotPos = 3;
                    unit1    = "nF";
                    unit2    = "nF";
                    break;
                case 1:
                    // 400 nF
                    l_Factor = 1e-11;
                    l_DotPos = 4;
                    unit1    = "nF";
                    unit2    = "nF";
                    break;
                case 2:
                    // 4 uF
                    l_Factor = 1e-10;
                    l_DotPos = 2;
                    unit1    = "uF";
                    unit2    = "uF";
                    break;
                case 3:
                    // 40 uF
                    l_Factor = 1e-9;
                    l_DotPos = 3;
                    unit1    = "uF";
                    unit2    = "uF";
                    break;
                case 4:
                    // 400 uF
                    l_Factor = 1e-8;
                    l_DotPos = 4;
                    unit1    = "uF";
                    unit2    = "uF";
                    break;
                case 5:
                    // 4000 uF
                    l_Factor = 1e-7;
                    l_DotPos = 5;
                    unit1    = "uF";
                    unit2    = "uF";
                    break;
                case 6:
                    // 40 mF
                    l_Factor = 1e-6;
                    l_DotPos = 3;
                    unit1    = "mF";
                    unit2    = "mF";
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else if (l_FunctionCode == 0x34) {
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: Diode DIO
            unit1   = "V";
            unit2   = "V";
            special = "DI";
            l_bShowBar = false;
            switch (l_FactorIndex) {
                case 0:
                    // 4 V
                    l_Factor = 1e-4;
                    l_DotPos = 2;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else if (l_FunctionCode == 0x35) {
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: Frequency
            special = "FR";
            l_bShowBar = false;
            
            // The frequency mode was tested using a "Fluke and Philips" PM5193 programmable synthesizer / function generator (50 MHz)
            switch (l_FactorIndex) {
                case 0:
                    // Range 40 Hz: 20 Hz is displayed as "20.000 Hz"
                    l_Factor = 1e-3; // is Hz
                    l_DotPos = 3;
                    unit1    = "Hz";
                    unit2    = "Hz";
                    break;
                case 1:
                    // Range 400 Hz: 200 Hz is displayed as "200.00 Hz"
                    l_Factor = 1e-2; // is Hz
                    l_DotPos = 4;
                    unit1    = "Hz";
                    unit2    = "Hz";
                    break;
                case 2:
                    // Range 4 kHz: 2 kHz is displayed as "2.0000 kHz"
                    l_Factor = 1e-1; // is Hz
                    l_DotPos = 2;
                    unit1    = "kHz";
                    unit2    = "kHz";
                    break;
                case 3:
                    // Range 40 kHz: 20 kHz is displayed as "20.000 kHz"
                    l_Factor = 1; // is Hz
                    l_DotPos = 3;
                    unit1    = "kHz";
                    unit2    = "kHz";
                    break;
                case 4:
                    // Range 400 kHz: 200 kHz is displayed as "200.00 kHz"
                    l_Factor = 1e1; // is Hz
                    l_DotPos = 4;
                    unit1    = "kHz";
                    unit2    = "kHz";
                    break;
                case 5:
                    // Range 4 Mhz: 2 MHz is displayed as "2.0000 MHz"
                    l_Factor = 1e2; // is Hz
                    l_DotPos = 2;
                    unit1    = "MHz";
                    unit2    = "MHz";
                    break;
                case 6:
                    // Range 40 Mhz: 20 MHz is displayed as "20.000 MHz"
                    l_Factor = 1e3; // is Hz
                    l_DotPos = 3;
                    unit1    = "MHz";
                    unit2    = "MHz";
                    break;
                case 7:
                    // Range 400 Mhz: 50 MHz is displayed as "050.00 MHz"
                    l_Factor = 1e4; // is Hz
                    l_DotPos = 4;
                    unit1    = "MHz";
                    unit2    = "MHz";
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else if (l_FunctionSelectCode == 0x31) {
            // Measurement mode: (4~20mA DC)%
            unit1      = "%";
            unit2      = "%";
            special    = "";
            switch (l_FactorIndex) {
                case 0:
                    // 400 % (but 100 % is the maximum)
                    l_Factor = 1e-2;
                    l_DotPos = 4;
                    l_bShowBar = false;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else if (l_FunctionCode == 0x36) {
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: DCuA
            unit1   = "uA";
            unit2   = "uA";
            special = "DC";
            switch (l_FactorIndex) {
                case 0:
                    // 400 uA
                    l_Factor = 1e-8;
                    l_DotPos = 4;
                    break;
                case 1:
                    // 4000 uA
                    l_Factor = 1e-7;
                    l_DotPos = 5;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else if (l_FunctionSelectCode == 0x31) {
            // Measurement mode: ACuA
            unit1   = "uA";
            unit2   = "uA";
            special = "AC";
            switch (l_FactorIndex) {
                case 0:
                    // 400 uA
                    l_Factor = 1e-7;
                    l_DotPos = 5;
                    break;
                case 1:
                    // 4000 uA
                    l_Factor = 1e-6;
                    l_DotPos = 0;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else if (l_FunctionCode == 0x37) {
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: DCmA
            unit1   = "mA";
            unit2   = "mA";
            special = "DC";
            switch (l_FactorIndex) {
                case 0:
                    // 40 mA
                    l_Factor = 1e-6;
                    l_DotPos = 3;
                    break;
                case 1:
                    // 400 mA
                    l_Factor = 1e-5;
                    l_DotPos = 4;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else if (l_FunctionSelectCode == 0x31) {
            // Measurement mode: ACmA
            unit1   = "mA";
            unit2   = "mA";
            special = "AC";
            switch (l_FactorIndex) {
                case 0:
                    // 40 mA
                    l_Factor = 1e-5;
                    l_DotPos = 4;
                    break;
                case 1:
                    // 400 mA
                    l_Factor = 1e-4;
                    l_DotPos = 5;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else if (l_FunctionCode == 0x38) {
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: DCA
            unit1   = "A";
            unit2   = "A";
            special = "DC";
            switch (l_FactorIndex) {
                case 0:
                    // 40 A (but 10 A is the maximum)
                    l_Factor = 1e-3;
                    l_DotPos = 3;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else if (l_FunctionSelectCode == 0x31) {
            // Measurement mode: ACA
            unit1   = "A";
            unit2   = "A";
            special = "AC";
            switch (l_FactorIndex) {
                case 0:
                    // 400 A (but 10 A is the maximum)
                    l_Factor = 1e-2;
                    l_DotPos = 4;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else if (l_FunctionCode == 0x39) {
        if (l_FunctionSelectCode == 0x30) {
            // Measurement mode: Active power + Apparent power
            unit1   = "W";
            unit2   = "VA";
            special = "AC";
            switch (l_FactorIndex) {
                case 0:
                    // 4000 W / 4000 VA
                    l_Factor = 1e-1;
                    l_DotPos = 5;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else if (l_FunctionSelectCode == 0x31) {
            // Measurement mode: Power factor + Frequency
            unit1   = "cosphi";
            unit2   = "Hz";
            special = "AC";
            switch (l_FactorIndex) {
                case 0:
                    // 40 PF
                    l_Factor  = 1e-3;
                    l_DotPos  = 3;

                    // 4000 Hz
                    l_Factor2 = 1e-1;
                    l_DotPos2 = 5;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else if (l_FunctionSelectCode == 0x32) {
            // Measurement mode: Voltage effective value + current effective value
            unit1   = "V";
            unit2   = "A";
            special = "AC";
            switch (l_FactorIndex) {
                case 0:
                    // 4000 V / 4000 A
                    l_Factor = 1e-3;
                    l_DotPos = 5;
                    break;
                default:
                    l_bParserError = true;
                    break;
            } // switch
        } else {
            // Unknown function select code
            l_bParserError = true;
        } // else
    } else {
        // Unknown function code
        l_bParserError = true;
    } // else

    // Obtain value of the primary display
    double d_val1;
    if ((in[15] & 0x01) || (in[19] & 0x01)) { // vc870_ol1 flag or open_flag
        val1   = "0L";
        d_val1 = 0;
    } else if (in[19] & 0x04) { // lo_flag
        val1   = "L 0";
        d_val1 = 0;
    } else if (in[19] & 0x02) { // hi_flag
        val1   = "H 1";
        d_val1 = 0;
    } else {
        if (in[15] & 0x04) { // Sign1_flag
            val1 = " -";
        } else {
            val1 = "  ";
        } // else

        val1 += QString(in[3]) + QString(in[4]) + QString(in[5]) + QString(in[6]) + QString(in[7]);
        d_val1 = (l_Factor * val1.toDouble());
        if (l_DotPos) {
            val1 = insertCommaIT(val1, l_DotPos); // 2 ... 5
        } // if
    } // else

    // Obtain value of the secondary display
    double d_val2;
    if ((in[17] & 0x08) || (in[19] & 0x01)) { // vc870_ol2 flag or open_flag
        val2   = "0L";
        d_val2 = 0;
    } else if (in[19] & 0x04) { // lo_flag
        val2   = "L 0";
        d_val2 = 0;
    } else if (in[19] & 0x02) { // hi_flag
        val2   = "H 1";
        d_val2 = 0;
    } else {
        if (in[15] & 0x08) { // Sign2_flag
            val2 = " -";
        } else {
            val2 = "  ";
        } // else

        val2 += QString(in[8]) + QString(in[9]) + QString(in[10]) + QString(in[11]) + QString(in[12]);
        if (l_Factor2 == 0.0) {
            d_val2 = (l_Factor * val2.toDouble());
        } else {
            d_val2 = (l_Factor2 * val2.toDouble());
        } // else

        if (!l_DotPos2) {
            val2 = insertCommaIT(val2, l_DotPos);  // 2 ... 5
        } else {
            val2 = insertCommaIT(val2, l_DotPos2); // 2 ... 5
        } // else
    } // else

    if (!l_bParserError) {
        // Always update the primary display (ID 0)
        Q_EMIT value(d_val1, val1, unit1, special, "", false, l_bShowBar, 0);
        if (in[20] == 0x31) { // LCD need Dual display
            // Enable or update the secondary display (ID 1)
            Q_EMIT value(d_val2, val2, unit2, special, "", false, l_bShowBar, 1); // Keep the l_bShowBar flag!
        } else {
            // Disable the secondary display (ID 1) as it is currently unused
            Q_EMIT value(0, "", "", "", "", false, l_bShowBar, 1); // Keep the l_bShowBar flag!
        } // else

        m_error = tr( "Connected %1" ).arg(m_device);
    } else {
        // Parser error
        m_error = tr( "Parser errors on %1" ).arg(m_device);
    } // else
}

void DMM::readDO3122Continuous( const QByteArray & data, int id, ReadEvent::DataFormat df )
{
    Q_UNUSED(id);
    Q_UNUSED(df);

    QString val = "";
    QString special = "";
    QString unit;
    double d_val;
    int idx;
    bool convOk = true;
    bool showbar = true;

    if (m_consoleLogging)
    {
       for (int i=0; i<22; ++i)
          fprintf( stdout, "%02X ", data[i] & 0x0ff );
       fprintf( stdout, "\r\n" );
    }

    // Check for overload
    if ( (static_cast<uint8_t>(data[6u]) == 0x80u)
         && (static_cast<uint8_t>(data[7u]) == 0x58u)
         && (static_cast<uint8_t>(data[8u]) == 0x5fu)
         && (static_cast<uint8_t>(data[9u]) == 0x00u)
         )
    {
        val = "OL";
        d_val = 0.0;
    }
    else
    {
        if (0u != (static_cast<uint8_t>(data[10u]) & 0x08u))
            val = "-";

        for (idx = 9; idx > 5; idx--)
        {
            // Check for blank
            if (data[idx] == 0u)
                continue;

            // Check for .dp
            if (data[idx] & 0x80u)
                val += '.';

            val += DO3122Digit(data[idx], &convOk);

            if (false == convOk)
            {
                m_error = tr( "Digit parse fail on %1" ).arg(m_device);
                return;
            }
        }

        d_val = val.toDouble(&convOk);
    }

    if (convOk)
    {
        switch (data[10u] & 0x07u)
        {
        case 0x00u: special = ""; break;
        case 0x01u: special = "Diode"; break;
        case 0x02u: special = "AC"; break;
        case 0x04u: special = "DC"; break;
        default: convOk = false; break;
        }

        if ( (data[20u] != 0u) && (data[21u] == 0u) )
        {
            switch (static_cast<uint8_t>(data[20u] & 0xffu))
            {
            case 0x01u: unit = "C"; break;
            case 0x02u: unit = "F"; break;
            case 0x90u: unit = "mF"; d_val *= 0.001; break;
            case 0xA0u: unit = "uF"; d_val *= 0.000001; break;
            case 0xC0u: unit = "nF"; d_val *= 0.000000001; break;
            default: convOk = false; break;
            }
        }
        else if (data[21u] != 0u)
        {
            switch (static_cast<uint8_t>(data[21u] & 0x33u))
            {
            case 0x00u: unit = ""; break;
            case 0x01u: d_val *= 0.000001; unit = 'u'; break;
            case 0x02u: d_val *= 0.001; unit = 'm'; break;
            case 0x10u: d_val *= 1000000; unit = 'M'; break;
            case 0x20u: d_val *= 1000; unit = 'K'; break;
            default: convOk = false; break;
            }

            switch (static_cast<uint8_t>(data[21u] & 0xCCu))
            {
            case 0x04u: unit += "A"; break;
            case 0x08u: unit += "V"; break;
            case 0x40u: unit += "Ohm"; break;
            case 0x80u: unit += "Hz"; break;
            default: convOk = false; break;
            }
        }
        else
            convOk = false;

        if (true == convOk)
        {
            Q_EMIT value(d_val, val, unit, special, "", false, showbar, 0);
            m_error = tr( "Connected %1" ).arg(m_device);
        }
        else
            m_error = tr( "Parser errors on %1" ).arg(m_device);
    }
    else
        m_error = tr( "Parser errors on %1" ).arg(m_device);
}

const char *DMM::DO3122Digit( int byte, bool *convOk )
{
  int           digit[10] = { 0x5f, 0x06, 0x6b, 0x2f, 0x36, 0x3d, 0x7d, 0x07, 0x7f, 0x3f };
  const char *c_digit[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

  byte &= 0x07f;

  *convOk = true;

  for (int n=0; n<10; n++)
  {
    if (byte == digit[n])
        return c_digit[n];
  }

  *convOk = false;
  return 0;
}
