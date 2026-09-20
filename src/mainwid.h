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

#pragma once


#include <QtGui>
#include <QtPrintSupport>

#include "ui_uimainwid.h"

#include "printdlg.h"

class DMM;
class QProcess;
class ConfigDlg;
class DisplayWid;
class TipDlg;
class Settings;
class InstancesDlg;
class MeterWid;
class SharedStateManager;

/// The central widget: owns the DMM connection, the settings dialog and the
/// graph, and routes readings to the display, the meter and the recorder.
///
/// MainWin provides the frame (menus, toolbars, docks, status bar) and hooks
/// its actions up to the *SLOT members here. Readings arrive in valueSLOT();
/// the sampled value is fed to the graph from timerEvent() at the recorder's
/// sample rate. External-application triggers, min/max memory and the
/// dialogs (settings, print, tips, instances) live here as well.
class MainWid : public QFrame, private Ui::UIMainWid
{
  Q_OBJECT
public:
  /// @param instance_id  name of this instance for multi-instance setups (--config-id)
  /// @param config_path  directory of the settings file (--config-dir), or empty
  /// @param parent       the MainWin
  MainWid(QString instance_id, QString config_path, QWidget *parent = Q_NULLPTR);
  /// Disconnects, saves the settings and asks about unsaved data. Returns
  /// false when the user cancels; MainWin then ignores the close event.
  bool        closeWin();
  /// Window geometry stored in the settings.
  QRect       winRect() const;
  bool        saveWindowPosition() const;
  bool        saveWindowSize() const;
  /// The LCD panel to feed; created and docked by MainWin.
  void        setDisplay(DisplayWid *);
  /// The analog meter to feed; created and docked by MainWin.
  void        setMeter(MeterWid *);
  /// The instance coordinator; readings are published through it.
  void        setStateManager(SharedStateManager *);
  /// --debug: pass on to DMM::setConsoleLogging().
  void        setConsoleLogging(bool);
  /// Stores the toolbar visibility (display, dmm, graph, file) in the settings.
  void        setToolbarVisibility(bool, bool, bool, bool);
  Settings   *settings() const { return m_settings; }

Q_SIGNALS:
  /// Recording started/stopped (graph state).
  void        running(bool);
  /// Message for the status bar's info field.
  void        info(const QString &);
  /// Message for the status bar's connection field (from DMM::error()).
  void        error(const QString &);
  /// The "icons with text" preference changed.
  void        useTextLabel(bool);
  /// Asks MainWin to connect/disconnect (drives the Connect action).
  void        setConnect(bool);
  /// Toolbar visibility read from the settings, for MainWin to apply.
  void        toolbarVisibility(bool, bool, bool, bool);
  /// The connection state changed; MainWin checks the Connect action.
  void        connectDMM(bool);
  /// A state string for the other instances (SharedStateManager).
  void        sendState(const QString&);

public Q_SLOTS:
  /// A reading from DMM::value(). Updates display, meter, min/max, the
  /// external-application thresholds and remembers dval for the sampler.
  void        valueSLOT(double, const QString &, const QString &, const QString &, const QString &, bool, bool, int);
  /// Clears min/max memory and the meter's peak/auto-bipolar latch.
  void        resetSLOT();
  /// Connect (true) or disconnect (false) the meter.
  void        connectSLOT(bool);
  void        quitSLOT();
  void        helpSLOT();
  /// Clears the graph.
  void        clearSLOT();
  /// Starts recording (also triggered remotely via the shared state).
  void        startSLOT();
  void        stopSLOT();
  /// Opens the settings dialog on its first page.
  void        configSLOT();
  /// Opens the settings dialog on the multimeter page.
  void        configDmmSLOT();
  /// Opens the settings dialog on the recording page.
  void        configRecorderSLOT();
  void        printSLOT();
  void        exportSLOT();
  void        importSLOT();
  /// Graph started/stopped recording.
  void        runningSLOT(bool);
  /// Settings dialog OK/Apply: re-reads the configuration (readConfig()).
  void        applySLOT();
  /// Settings dialog Cancel.
  void        rejectSLOT();
  void        showTipsSLOT();
  /// Shows the instances dialog.
  void        instancesSLOT();
  /// The set of running instances changed (from SharedStateManager).
  void        instancesChangedSlot(QStringList&);

protected:
  DMM        *m_dmm;
  double      m_min;
  double      m_max;
  QString     m_lastUnit;
  ConfigDlg  *m_configDlg;
  qtdmm::PrintDlg *m_printDlg;
  QPrinter    m_printer;
  QProcess   *m_external;
  DisplayWid *m_display;
  MeterWid   *m_meter;
  SharedStateManager *m_stateMgr;
  double      m_dval;
  TipDlg     *m_tipDlg;
  InstancesDlg *m_instancesDlg;
  Settings    *m_settings;

  /// Applies the settings to DMM, graph, display and meter.
  void        readConfig();
  /// Derives full scale, coupling label, overload and peak for the meter.
  void        feedMeter(const QString &val, const QString &unit, const QString &special, bool hold);
  QRect       parentRect() const;
  /// Sample timer: hands the current value to the graph.
  void        timerEvent(QTimerEvent *);

protected Q_SLOTS:
  /// Launches the configured external application (threshold trigger).
  void        startExternalSLOT();
  /// The external application exited.
  void        exitedSLOT();
  /// Graph zoom changed; re-applies the window/total size.
  void        zoomedSLOT();
};

