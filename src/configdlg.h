//======================================================================
// File:		configdlg.h
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 14:53:06 CEST 2002
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
// Copyright (c) 2002 Matthias Toussaint
//======================================================================

#pragma once

#include <QtSerialPort>

#include "ui_uiconfigdlg.h"
#include "dmmgraph.h"
#include "readevent.h"
#include "dmmdecoder.h"

class Settings;
class SharedStateManager;
class QPrinter;
class RecorderPrefs;
class ScalePrefs;
class DmmPrefs;
class GuiPrefs;
class GraphPrefs;
class IntegrationPrefs;
class ExecutePrefs;
class PortsPrefs;

/// The settings dialog: a category list beside a stack of PrefWidget pages.
///
/// Besides hosting the pages it is the read-only facade the rest of the
/// application uses for configuration values - MainWid, the graph and the
/// meter ask the accessors here rather than reading Settings keys
/// themselves. OK/Apply run applySLOT() on every page, save the Settings and
/// emit applied()/accepted(); Cancel restores the pages from the stored
/// values.
class ConfigDlg : public QDialog, private Ui::UIConfigDlg
{
  Q_OBJECT
public:
  /// The ids double as the page index in ui_stack, so this is also the
  /// order of the category list.
  enum PageType
  {
    DMM = 0,
    GUI,
    Graph,
    Scale,
    Integration,
    Recorder,
    Ports,
    External,
    NumItems,
  };

  ConfigDlg(Settings* settings, QWidget *parent = Q_NULLPTR);

  /// @name Multimeter page
  /// @{
  QString               device() const;
  int                   speed() const;
  ReadEvent::DataFormat format() const;
  QSerialPort::Parity   parity() const;
  bool                  externalSetup() const;
  int                   display() const;
  int                   bits() const;
  int                   stopBits() const;
  int                   numValues() const;
  bool                  rts() const;
  bool                  dtr() const;
  DmmDecoder::DMMInfo    dmmInfo() const;
  QString               dmmName() const;
  /// @}

  /// @name Graph, scale and integration pages
  /// @{
  int                   windowSeconds() const;
  int                   totalSeconds() const;
  double                scaleMin() const;
  double                scaleMax() const;
  bool                  automaticScale() const;
  bool                  includeZero() const;
  QColor                bgColor() const;
  QColor                gridColor() const;
  QColor                dataColor() const;
  QColor                cursorColor() const;
  QColor                startColor() const;
  QColor                externalColor() const;
  QColor                intColor() const;
  QColor                intThresholdColor() const;
  int                   lineWidth() const;
  int                   intLineWidth() const;
  int                   lineMode() const;
  int                   pointMode() const;
  int                   intLineMode() const;
  int                   intPointMode() const;
  bool                  crosshair() const;
  double                intScale() const;
  double                intThreshold() const;
  double                intOffset() const;
  bool                  showIntegration() const;
  /// @}

  /// @name Recording page
  /// @{
  QTime                 startTime() const;
  DMMGraph::SampleMode  sampleMode() const;
  int                   sampleStep() const;
  int                   sampleLength() const;
  double                raisingThreshold() const;
  double                fallingThreshold() const;
  /// @}

  /// @name Appearance page (window, toolbars, LCD display, analog meter, tips)
  /// @{
  void                  setWinRect(const QRect &);
  QRect                 winRect() const;
  bool                  saveWindowPosition() const;
  bool                  saveWindowSize() const;
  bool                  alertUnsavedData() const;
  bool                  useTextLabel() const;
  bool                  showTip() const;
  int                   currentTipId() const;
  bool                  showDmmToolbar() const;
  bool                  showGraphToolbar() const;
  bool                  showFileToolbar() const;
  bool                  showDisplay() const;
  QColor                displayBgColor() const;
  bool                  showMinMax() const;
  bool                  showBar() const;
  /// MeterWid::ScaleMode as int.
  int                   meterScaleMode() const;
  /// 0 dark studio, 1 classic ivory (MeterStyle::dark()/ivory()).
  int                   meterStyle() const;
  bool                  meterBallistics() const;
  /// Start of the red zone in percent of full scale.
  int                   meterRedZone() const;
  void                  setToolbarVisibility(bool, bool, bool, bool);
  /// @}

  /// @name External application page
  /// @{
  bool                  startExternal() const;
  bool                  externalFalling() const;
  double                externalThreshold() const;
  QString               externalCommand() const;
  bool                  disconnectExternal() const;
  /// @}

  /// Printer settings are kept in the Settings too.
  void                  writePrinter(QPrinter *);
  void                  readPrinter(QPrinter *);
  /// Raises the dialog on the given page.
  void                  showPage(PageType);
  /// Instance coordinator for the multimeter page's formula hint.
  void                  setStateManager(SharedStateManager *);


public Q_SLOTS:
  /// Connected state: the multimeter page is disabled while connected.
  void                  connectSLOT(bool);
  /// OK or Apply: applies all pages and saves.
  void                  on_ui_buttonBox_accepted();
  void                  on_ui_buttonBox_rejected();
  /// The graph changed the sample time (recorder page follows).
  void                  setSampleTimeSLOT(int);
  void                  setGraphSizeSLOT(int, int);
  void                  setShowTipsSLOT(bool);
  void                  setCurrentTipSLOT(int);
  void                  zoomInSLOT(double);
  void                  zoomOutSLOT(double);
  /// A threshold cursor was dragged in the graph; updates the spin box.
  void                  thresholdChangedSLOT(DMMGraph::CursorMode, double);

Q_SIGNALS:
  /// Apply pressed: settings saved, dialog stays open.
  void applied();
  /// OK pressed: settings saved, dialog closes.
  void                  accepted();
  void                  rejected();
  void                  showTips(bool);
  /// Window/total size changed on the graph page.
  void                  zoomed();

protected:
  QPrinter             *m_printer;
  QRect                 m_winRect;
  RecorderPrefs        *m_recorder;
  ScalePrefs           *m_scale;
  PortsPrefs           *m_ports;
  DmmPrefs             *m_dmm;
  GuiPrefs             *m_gui;
  GraphPrefs           *m_graph;
  IntegrationPrefs     *m_integration;
  ExecutePrefs         *m_execute;
  bool                  m_buttonBox_OK;

  void                  reloadSettings();

protected Q_SLOTS:
  void                  on_ui_list_currentItemChanged(QListWidgetItem *current, QListWidgetItem *);
  void                  on_ui_factoryDefaults_clicked();
  void                  on_ui_buttonBox_clicked(QAbstractButton *button);

private:
  Settings             *m_settings;

};

