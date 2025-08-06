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
class QPrinter;
class RecorderPrefs;
class ScalePrefs;
class DmmPrefs;
class GuiPrefs;
class GraphPrefs;
class IntegrationPrefs;
class ExecutePrefs;
class PortsPrefs;

class ConfigDlg : public QDialog, private Ui::UIConfigDlg
{
  Q_OBJECT
public:
  enum PageType
  {
    Recorder = 0,
    Scale,
    DMM,
    Ports,
    GUI,
    Graph,
    Integration,
    External,
    NumItems,
  };

  ConfigDlg(QWidget *parent = Q_NULLPTR);
  Settings             *getSettings() { return m_settings;  }
  QString               device() const;
  int                   speed() const;
  int                   windowSeconds() const;
  int                   totalSeconds() const;
  QTime                 startTime() const;
  DMMGraph::SampleMode  sampleMode() const;
  int                   sampleStep() const;
  int                   sampleLength() const;
  double                raisingThreshold() const;
  double                fallingThreshold() const;
  double                scaleMin() const;
  double                scaleMax() const;
  bool                  automaticScale() const;
  bool                  includeZero() const;
  bool                  showMinMax() const;
  bool                  showBar() const;
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

  QColor                bgColor() const;
  QColor                gridColor() const;
  QColor                dataColor() const;
  QColor                cursorColor() const;
  QColor                displayBgColor() const;
  QColor                displayTextColor() const;
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
  void                  writePrinter(QPrinter *);
  void                  readPrinter(QPrinter *);
  void                  setWinRect(const QRect &);
  QRect                 winRect() const;
  bool                  saveWindowPosition() const;
  bool                  saveWindowSize() const;
  bool                  alertUnsavedData() const;
  bool                  useTextLabel() const;
  bool                  startExternal() const;
  bool                  externalFalling() const;
  double                externalThreshold() const;
  QString               externalCommand() const;
  bool                  disconnectExternal() const;
  bool                  crosshair() const;
  double                intScale() const;
  double                intThreshold() const;
  double                intOffset() const;
  bool                  showIntegration() const;
  bool                  showTip() const;
  int                   currentTipId() const;
  QString               dmmName() const;
  void                  showPage(PageType);
  bool                  showDmmToolbar() const;
  bool                  showGraphToolbar() const;
  bool                  showFileToolbar() const;
  bool                  showDisplay() const;
  void                  setToolbarVisibility(bool, bool, bool, bool);

  QString               deviceListText() const;

public Q_SLOTS:
  void                  connectSLOT(bool);
  void                  on_ui_buttonBox_accepted();
  void                  on_ui_buttonBox_rejected();
  void                  setSampleTimeSLOT(int);
  void                  setGraphSizeSLOT(int, int);
  void                  setShowTipsSLOT(bool);
  void                  setCurrentTipSLOT(int);
  void                  zoomInSLOT(double);
  void                  zoomOutSLOT(double);
  void                  thresholdChangedSLOT(DMMGraph::CursorMode, double);

Q_SIGNALS:
  void                  accepted();
  void                  rejected();
  void                  showTips(bool);
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

