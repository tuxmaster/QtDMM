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

class ReadEvent
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
		RS22812Continuous,
		VC870Continuous,
        DO3122Continuous,
		CyrustekES51922
	  };
};
#endif // READEVENT_HH
