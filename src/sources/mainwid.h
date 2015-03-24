//======================================================================
// File:		mainwid.h
// Author:	Matthias Toussaint
// Created:	Tue Apr 10 17:25:07 CEST 2001
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

#ifndef MAINWID_HH
#define MAINWID_HH


#include <QtGui>
#include <QtPrintSupport>

#include "ui_uimainwid.h"

class DMM;
class PrintDlg;
class QProcess;
class ConfigDlg;
class DisplayWid;
class TipDlg;

class MainWid : public QFrame, private Ui::UIMainWid
{
  Q_OBJECT
	public:
	  MainWid(QWidget *parent=0);
	  bool			closeWin();
	  QRect			winRect() const;
	  bool			saveWindowPosition() const;
	  bool			saveWindowSize() const;
	  void			setDisplay( DisplayWid * );
	  void			setConsoleLogging( bool );
	  void			setToolbarVisibility( bool, bool, bool, bool, bool );

	  QString		deviceListText() const;

	Q_SIGNALS:
	  void			running( bool );
	  void			info( const QString & );
	  void			error( const QString & );
	  void			useTextLabel( bool );
	  void			winGeometry( const QRect & );
	  void			setConnect( bool );
	  void			toolbarVisibility( bool, bool, bool, bool, bool );
	  void			connectDMM( bool );

	public Q_SLOTS:
	  void			valueSLOT( double, const QString &, const QString &, const QString &, bool, int );
	  void			resetSLOT();
	  void			connectSLOT( bool );
	  void			quitSLOT();
	  void			helpSLOT();
	  void			clearSLOT();
	  void			startSLOT();
	  void			stopSLOT();
	  void			configSLOT();
	  void			configDmmSLOT();
	  void			configRecorderSLOT();
	  void			printSLOT();
	  void			exportSLOT();
	  void			importSLOT();
	  void			runningSLOT( bool );
	  void			applySLOT();
	  void			showTipsSLOT();

	protected:
	  DMM			*m_dmm;
	  double		m_min;
	  double		m_max;
	  QString		m_lastUnit;
	  ConfigDlg		*m_configDlg;
	  PrintDlg		*m_printDlg;
	  QPrinter		m_printer;
	  QProcess		*m_external;
	  DisplayWid	*m_display;
	  double		m_dval;
	  TipDlg		*m_tipDlg;

	  void			readConfig();
	  QRect			parentRect() const;
	  void			timerEvent( QTimerEvent * );

	protected Q_SLOTS:
	  void			startExternalSLOT();
	  void			exitedSLOT();
	  void			zoomedSLOT();

};

#endif // MAINWID_HH
