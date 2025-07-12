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

#pragma once

#include <QtGui>
#include <QtWidgets>

#include "ui_uimainwin.h"

class MainWid;
class DisplayWid;

// mt: changed all QAction into Q3Action
class MainWin : public QMainWindow, private Ui::UIMainWin
{
  Q_OBJECT
public:
  MainWin(QWidget *parent = Q_NULLPTR);
  void			setConsoleLogging(bool);

protected Q_SLOTS:
  void			runningSLOT(bool);
  void			connectSLOT(bool);
  void			on_action_On_version_triggered();
  void			setConnectSLOT(bool);
  void			toolbarVisibilitySLOT(bool, bool, bool, bool, bool);
  void			setToolbarVisibilitySLOT();
  void			setUseTextLabel(bool on);

protected:
  MainWid       *m_wid;
  DisplayWid	*m_display;
  bool			m_running;
  QLabel		*m_error;
  QLabel		*m_info;

  void			setupIcons();
  void			createToolBars();
  void			createActions();
  void			closeEvent(QCloseEvent *)Q_DECL_OVERRIDE;
};

