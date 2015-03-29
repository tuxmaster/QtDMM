//======================================================================
// File:		mainwin.h
// Author:	Matthias Toussaint
// Created:	Sun Sep  2 12:14:07 CEST 2001
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

#ifndef MAINWIN_HH
#define MAINWIN_HH

#include <QtGui>
#include <QtWidgets>

class MainWid;
class DisplayWid;

// mt: changed all QAction into Q3Action
class MainWin : public QMainWindow
{
  Q_OBJECT
	public:
	  MainWin(QWidget *parent=0);
	  void setConsoleLogging( bool );

	protected Q_SLOTS:
	  void runningSLOT( bool );
	  void connectSLOT( bool );
	  void versionSLOT();
	  void setConnectSLOT( bool );
	  void toolbarVisibilitySLOT( bool, bool, bool, bool, bool );
	  void setToolbarVisibilitySLOT();

	protected:
	  MainWid       *m_wid;
	  QToolBar      *m_dmmTB;
	  QToolBar		*m_graphTB;
	  QToolBar		*m_fileTB;
	  QToolBar	    *m_helpTB;
	  QToolBar		*m_displayTB;
	  DisplayWid	*m_display;
	  QAction		*m_connectAction;
	  QAction		*m_resetAction;
	  QAction		*m_startAction;
	  QAction		*m_stopAction;
	  QAction		*m_clearAction;
	  QAction		*m_printAction;
	  QAction		*m_exportAction;
	  QAction		*m_importAction;
	  QAction		*m_configAction;
	  QAction		*m_configDmmAction;
	  QAction		*m_configRecorderAction;
	  QAction		*m_quitAction;
	  QAction		*m_helpAction;
	  QAction		*m_showTipsAction;
	  QAction		*m_versionAction;
	  bool			m_running;
	  QLabel		*m_error;
	  QLabel		*m_info;

	  void			createToolBars();
	  void			createActions();
	  void			createMenu();
	  void			closeEvent( QCloseEvent * )Q_DECL_OVERRIDE;
};

#endif // MAINWIN_HH
