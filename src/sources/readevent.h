//======================================================================
// File:		readevent.h
// Author:	Matthias Toussaint
// Created:	Sat Apr 14 13:01:28 CEST 2001
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

#ifndef READEVENT_HH
#define READEVENT_HH

#include <QtCore>

//Added by qt3to4:
//#include <QCustomEvent>

// mt: it's just the enum now
// I've removed the custom event stuff which was needed
// with Qt<4 as Qthread couldn't emit signals
class ReadEvent //: public QCustomEvent
{
	public:
	  enum DataFormat
	  {
		Metex14 = 0,
		PeakTech10,
		Voltcraft14Continuous,
		Voltcraft15Continuous,
		M9803RContinuous,
		VC820Continuous,
		IsoTech,
		VC940Continuous,
		QM1537Continuous,
		RS22812Continuous
	  };
/*
  ReadEvent( char *str, int len, int id, DataFormat df );
  ~ReadEvent();

  const char *string() const { return m_str; }
  DataFormat format() const { return m_format; }
  int id() const { return m_id; }
  int length() const { return m_length; }

private:
  char       m_str[23];
  int        m_length;
  DataFormat m_format;
  int        m_id;
  */
};
#endif // READEVENT_HH
