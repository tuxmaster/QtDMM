//======================================================================
// File:		dmmgraph.h
// Author:	Matthias Toussaint
// Created:	Tue Apr 10 17:43:46 CEST 2001
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
#include <QPrinter>
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QGraphicsLineItem>

class Settings;

class DMMGraph : public QWidget
{
  Q_OBJECT
public:
  enum SampleMode
  {
    Manual = 0,
    Time,
    Raising,
    Falling
  };

  enum PointMode
  {
    NoPoint = 0,
    Circle,
    Square,
    Diamond,
    X,
    LargeCircle,
    LargeSquare,
    LargeDiamond,
    LargeX
  };

  enum LineMode
  {
    NoLine = 0,
    Solid,
    Dot
  };

  enum CursorMode
  {
    NoCursor = 0,
    Trigger,
    External,
    Integration
  };

  enum PopupID
  {
    IDConnect = 1,
    IDDisconnect,
    IDStopRecorder,
    IDStartRecorder,
    IDClearGraph,
    IDConfigure,
    IDExportData,
    IDImportData
  };

  DMMGraph(QWidget *parent, Settings *settings);
  DMMGraph(QWidget *parent = Q_NULLPTR);
  ~DMMGraph();
  void             setGraphSize(int size, int length);
  void             addValue(double);
  void             setUnit(const QString &);
  void             setSampleTime(int v) { m_sampleTime = v; }
  void             setSampleLength(int v) { m_sampleLength = v; }
  void             setStartTime(const QTime &time) { m_startTime = time; }
  void             setMode(DMMGraph::SampleMode mode);
  void             print(QPrinter *prt, const QString &, const QString &);
  void             setThresholds(double falling, double raising);
  void             setScale(bool autoScale, bool includeZero, double min, double max);
  void             setColors(const QColor &bg, const QColor &grid,
                             const QColor &data, const QColor &cursor,
                             const QColor &start, const QColor &external,
                             const QColor &integration, const QColor &intThreshold);
  void             setLine(int d, int i);
  void             setExternal(bool on, bool falling = false, double threshold = 0);
  bool             dirty() const { return m_dirty; }
  void             setAlertUnsaved(bool on) { m_alertUnsaved = on; }
  void             setCrosshair(bool on) { m_crosshair = on; }
  void             setLineStyle(int, int, int, int);
  void             setIntegration(bool, double, double, double);
  void             setSettings(Settings *settings) { m_cfg = settings; }

Q_SIGNALS:
  void             info(const QString &);
  void             error(const QString &);
  void             running(bool);
  void             graphSize(int, int);
  void             sampleTime(int);
  void             externalTriggered();
  void             zoomIn(double);
  void             zoomOut(double);
  void             thresholdChanged(DMMGraph::CursorMode, double);
  void             connectDMM(bool);
  void             configure();
  void             exportData();
  void             importData();

public Q_SLOTS:
  void             clearSLOT();
  void             startSLOT();
  void             stopSLOT();
  bool             exportDataSLOT();
  void             importDataSLOT();
  void             connectSLOT(bool on) { m_connected = on; }

  // File-path-driven, non-interactive halves of export/importDataSLOT (no QFileDialog),
  // split out so the CSV parsing/writing logic can be exercised from tests.
  bool             exportCsvFile(const QString &fileName);
  bool             importCsvFile(const QString &fileName);

protected Q_SLOTS:
  void             popupSLOT(QAction *action);

protected:
  QScrollBar      *scrollbar;
  int              m_size;
  int              m_length;
  double           m_scaleMin;
  double           m_scaleMax;
  bool             m_autoScale;
  QVector<double> *m_array;	// mt: changed from QArray to QVector
  QVector<double> *m_arrayInt;	// mt: changed from QArray to QVector
  int              m_pointer;
  QString          m_unit;
  double           m_sampleTime;
  int              m_sampleLength;
  bool             m_running;
  bool             m_connected;
  int              m_sampleCounter;
  int              m_remainingLength;
  SampleMode       m_mode;
  QTime            m_startTime;
  QDateTime        m_graphStartDateTime;
  double           m_sum;
  bool             m_first;
  QPoint           m_mpos;
  bool             m_mouseDown;
  bool             m_mousePan;
  CursorMode       m_cursorMode;
  double           m_raisingThreshold;
  double           m_fallingThreshold;
  double           m_lastVal;
  bool             m_lastValValid;
  QColor           m_bgColor;
  QColor           m_gridColor;
  QColor           m_dataColor;
  QColor           m_cursorColor;
  QColor           m_startColor;
  QColor           m_externalColor;
  QColor           m_intColor;
  QColor           m_intThresholdColor;
  int              m_lineWidth;
  int              m_intLineWidth;
  bool             m_dirty;
  bool             m_alertUnsaved;
  bool             m_startExternal;
  bool             m_externalFalling;
  double           m_externalThreshold;
  bool             m_externalStarted;
  bool             m_crosshair;
  PointMode        m_pointMode;
  PointMode        m_intPointMode;
  LineMode         m_lineMode;
  LineMode         m_intLineMode;
  double           m_integrationScale;
  double           m_integrationThreshold;
  double           m_integrationOffset;
  bool             m_showIntegration;
  double           m_factor;
  QString          m_prefix;
  bool             m_includeZero;
  QMenu           *m_popup;

  // Qt Charts based rendering (core curve + axes, plus the integration curve
  // and the cursor/threshold overlays reintroduced as QGraphicsLineItems on
  // top of the chart scene).
  QChartView      *m_chartView;
  QChart          *m_chart;
  QLineSeries     *m_dataSeries;
  QScatterSeries  *m_dataPoints;
  QLineSeries     *m_intSeries;
  QScatterSeries  *m_intPoints;
  QValueAxis      *m_xAxis;
  QValueAxis      *m_yAxis;
  QGraphicsLineItem *m_crosshairVLine;
  QGraphicsLineItem *m_crosshairHLine;
  QGraphicsLineItem *m_triggerLine;
  QGraphicsLineItem *m_externalLine;
  QGraphicsLineItem *m_integrationLine;

  void             resizeEvent(QResizeEvent *)Q_DECL_OVERRIDE;
  bool             eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

  void             handleChartMousePress(QMouseEvent *);
  void             handleChartMouseMove(QMouseEvent *);
  void             handleChartMouseRelease(QMouseEvent *);
  void             handleChartWheel(QWheelEvent *);

  void             emitInfo();
  void             computeUnitFactor();
  bool             computeMinMax(double);
  void             rebuildSeries();
  void             updateXAxisRange();
  void             updateSeriesAppearance();
  void             updateThresholdLinesVisibility();
  void             updateThresholdLinePositions();
  QString          formatEngineeringValue(double) const;

private:
  Qt::PenStyle     penStyle(LineMode);
  Settings        *m_cfg;

};

