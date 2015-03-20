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

#include <q3mainwindow.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <QLabel>

class MainWid;
class Q3Action;
class Q3ToolBar;
class QLabel;
class DisplayWid;

// mt: changed all QAction into Q3Action
class MainWin : public Q3MainWindow
{
  Q_OBJECT
public:
  MainWin( QWidget *parent=0, const char *name=0 );
  virtual ~MainWin();

  void setConsoleLogging( bool );
  
protected slots:
  void runningSLOT( bool );
  void connectSLOT( bool );
  void versionSLOT();
  void setConnectSLOT( bool );
  void toolbarVisibilitySLOT( bool, bool, bool, bool, bool );
  void setToolbarVisibilitySLOT();
  
protected:
  MainWid     *m_wid;  
  Q3ToolBar    *m_dmmTB;
  Q3ToolBar    *m_graphTB;
  Q3ToolBar    *m_fileTB;
  Q3ToolBar    *m_helpTB;
  Q3ToolBar    *m_displayTB;
  DisplayWid  *m_display;
  Q3Action     *m_connectAction;
  Q3Action     *m_resetAction;
  Q3Action     *m_startAction;
  Q3Action     *m_stopAction;
  Q3Action     *m_clearAction;
  Q3Action     *m_printAction;
  Q3Action     *m_exportAction;
  Q3Action     *m_importAction;
  Q3Action     *m_configAction;
  Q3Action     *m_configDmmAction;
  Q3Action     *m_configRecorderAction;
  Q3Action     *m_quitAction;
  Q3Action     *m_helpAction;
  Q3Action     *m_showTipsAction;
  Q3Action     *m_versionAction;
  bool         m_running;
  QLabel      *m_error;
  QLabel      *m_info;
  
  void createToolBars();
  void createActions();
  void createMenu();
  void closeEvent( QCloseEvent * );
};

#endif // MAINWIN_HH
