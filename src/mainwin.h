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
#include <QMenu>
#include <QCommandLineParser>

#include "ui_uimainwin.h"
#include "sharedstatemanager.h"

class MainWid;
class DisplayWid;
class HelpDlg;
class MeterWid;
class QDockWidget;

/// The application window: menus, toolbars, status bar and the two dock
/// panels (LCD display, analog meter) around a MainWid.
///
/// Also the place where several QtDMM instances talk to each other: the
/// SharedStateManager's state changes ("RECORD", "STOP", "RAISE_<id>") are
/// turned into actions here, and a second instance with the same id is
/// refused.
class MainWin : public QMainWindow, private Ui::UIMainWin
{
  Q_OBJECT
public:
  /// @param parser the processed command line (--debug, --config-dir, --config-id)
  /// @param parent parent widget, normally none
  MainWin(QCommandLineParser &parser, QWidget *parent = Q_NULLPTR);
  /// --debug: frame dump and the qtdmm.hid logging category.
  void      setConsoleLogging(bool);

protected Q_SLOTS:
  /// Recording state changed; enables/disables Start/Stop.
  void      runningSLOT(bool);
  /// The Connect action was toggled.
  void      connectSLOT(bool);
  /// Start action: records locally and tells the other instances.
  void      startSLOT();
  void      stopSLOT();
  /// Writes a state string for the other instances.
  void      sendStateSLOT(const QString &);
  void      on_action_About_triggered();
  void      on_action_Help_triggered();
  /// Shows the popup menu (the window has no menu bar).
  void      on_action_Menu_triggered();
  /// Checks the Connect action without triggering it.
  void      setConnectSLOT(bool);
  /// Applies toolbar visibility from the settings.
  void      toolbarVisibilitySLOT(bool, bool, bool, bool);
  /// A toolbar was shown/hidden by the user; stores the new state.
  void      setToolbarVisibilitySLOT();
  /// Toolbar button style: icons only or icons with text.
  void      setUseTextLabel(bool on);
  /// Title = app name, instance id and the configured meter.
  void      updateWindowTitle();

protected:
  MainWid    *m_wid;
  DisplayWid *m_display;
  MeterWid   *m_meter;
  QDockWidget *m_meterDock;
  QDockWidget *m_displayDock;
  QAction    *m_lockPanels;
  /// Locked panels have no title bar and cannot be moved or floated.
  void        setPanelsLocked(bool locked);
  bool        m_running;
  QLabel     *m_error;
  QLabel     *m_info;
  QMenu      *m_menu;
  HelpDlg    *m_helpDlg;
  SharedStateManager* m_stateMgr;
  QString     m_config_id;
  bool        m_localRecord;

  void        setupIcons();
  void        createActions();
  /// Saves window/dock state; vetoed by MainWid::closeWin() on unsaved data.
  void        closeEvent(QCloseEvent *)Q_DECL_OVERRIDE;
  /// Raises this window when another instance asks for it ("RAISE_<id>").
  void        bringMainWindowToFront();
};

