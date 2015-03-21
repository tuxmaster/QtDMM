//======================================================================
// File:		printdlg.h
// Author:	Matthias Toussaint
// Created:	Wed Apr 11 16:53:46 CEST 2001
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

#ifndef PRINTDLG_HH
#define PRINTDLG_HH

#include <QtGui>
#include "uiprintdlg.h"


class QPrinter;
class PrintDlg : public UIPrintDlg
{
  Q_OBJECT
	public:
	  PrintDlg( QWidget *parent=0, const char *name=0 );
	  void		setPrinter( QPrinter * prt );
	  QString	title() const { return printTitle->text(); }
	  QString	comment() const { return printComment->text(); }

	protected:
	  QPrinter	*m_printer;

	protected Q_SLOTS:
	  void		configSLOT();
	  void		helpSLOT();
	  void		createPrinterString();

};

#endif // PRINTDLG_HH
