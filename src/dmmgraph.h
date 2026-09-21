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

/// The recorder: samples the reading, keeps the recorded curve and plots it.
///
/// MainWid calls addValue() ten times a second with the current reading;
/// while recording the values are averaged over the sample time and stored
/// in a ring of @c m_length samples, of which a window of @c m_size is shown
/// (scrollable). Recording starts manually, at a clock time or when the
/// reading crosses a threshold (SampleMode), and can trigger the external
/// application the same way (setExternal()). An integration curve (running
/// sum above a threshold) is kept alongside. Rendering uses Qt Charts; the
/// cursor and threshold lines are QGraphicsLineItems on top of the chart.
/// Data can be exported/imported as CSV and printed.
///
/// Times are in tenths of a second internally: a sample time of 5 means
/// one stored sample per 0.5 s.
class DMMGraph : public QWidget
{
  Q_OBJECT
public:
  /// How recording is started.
  enum SampleMode
  {
    Manual = 0,   ///< Start button
    Time,         ///< at setStartTime()
    Raising,      ///< when the reading rises through the raising threshold
    Falling       ///< when the reading falls through the falling threshold
  };

  /// Marker drawn at each sample.
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

  /// Line style between samples.
  enum LineMode
  {
    NoLine = 0,
    Solid,
    Dot
  };

  /// Which horizontal threshold line the mouse is dragging.
  enum CursorMode
  {
    NoCursor = 0,
    Trigger,      ///< recording start threshold
    External,     ///< external application threshold
    Integration   ///< integration threshold
  };

  /// Entries of the context menu.
  enum PopupID
  {
    IDConnect = 1,
    IDDisconnect,
    IDStopRecorder,
    IDStartRecorder,
    IDClearGraph,
    IDConfigure,
    IDExportData,
    IDImportData,
    IDCopyImage
  };

  DMMGraph(QWidget *parent, Settings *settings);
  DMMGraph(QWidget *parent = Q_NULLPTR);
  ~DMMGraph();
  /// Visible window and total recording length, both in seconds.
  void             setGraphSize(int size, int length);
  /// The current reading; called every 100 ms. Handles the start triggers
  /// and, while recording, averaging and storing.
  void             addValue(double);
  /// Unit of the recorded quantity for the axis label; the SI prefix is
  /// stripped because values arrive in base units (see DmmDecoder::DmmResponse).
  void             setUnit(const QString &);
  /// Sample time in tenths of a second.
  void             setSampleTime(int v) { m_sampleTime = v; }
  /// Recording duration in tenths of a second after which recording stops
  /// on its own (0 = until stopped).
  void             setSampleLength(int v) { m_sampleLength = v; }
  /// Clock time for SampleMode::Time.
  void             setStartTime(const QTime &time) { m_startTime = time; }
  void             setMode(DMMGraph::SampleMode mode);
  /// Prints the curve with title and comment.
  void             print(QPrinter *prt, const QString &, const QString &);
  /// Thresholds for the Raising/Falling start modes.
  void             setThresholds(double falling, double raising);
  /// Y axis: automatic (optionally always including zero) or fixed min/max.
  void             setScale(bool autoScale, bool includeZero, double min, double max);
  void             setColors(const QColor &bg, const QColor &grid,
                             const QColor &data, const QColor &cursor,
                             const QColor &start, const QColor &external,
                             const QColor &integration, const QColor &intThreshold);
  /// Line widths of the data and the integration curve.
  void             setLine(int d, int i);
  /// External application trigger: fire externalTriggered() once per
  /// recording when the reading crosses @p threshold in the given direction.
  void             setExternal(bool on, bool falling = false, double threshold = 0);
  /// Unsaved recorded data in memory.
  bool             dirty() const { return m_dirty; }
  void             setAlertUnsaved(bool on) { m_alertUnsaved = on; }
  void             setCrosshair(bool on) { m_crosshair = on; }
  /// LineMode and PointMode for the data and the integration curve.
  void             setLineStyle(int, int, int, int);
  /// Integration curve: shown, scale factor, threshold (values at or below
  /// it reset the sum) and offset.
  void             setIntegration(bool, double, double, double);
  void             setSettings(Settings *settings) { m_cfg = settings; }

Q_SIGNALS:
  /// Status bar text: sample time, window and remaining length.
  void             info(const QString &);
  void             error(const QString &);
  /// Recording started/stopped.
  void             running(bool);
  /// Window/total size changed by zooming (seconds).
  void             graphSize(int, int);
  /// Sample time changed by a CSV import (tenths of a second).
  void             sampleTime(int);
  /// The external application threshold was crossed.
  void             externalTriggered();
  void             zoomIn(double);
  void             zoomOut(double);
  /// Show the whole recording (key 0).
  void             zoomFit();
  /// A threshold line was dragged with the mouse.
  void             thresholdChanged(DMMGraph::CursorMode, double);
  /// @name Context menu requests, handled by MainWid
  /// @{
  void             connectDMM(bool);
  void             configure();
  void             exportData();
  void             importData();
  /// @}

public Q_SLOTS:
  /// Discards the recorded data.
  void             clearSLOT();
  /// @name Keyboard zoom/pan, also reachable from MainWin's shortcuts
  /// @{
  void             zoomInSLOT()  { Q_EMIT zoomIn(1.25); }
  void             zoomOutSLOT() { Q_EMIT zoomOut(1.25); }
  void             zoomFitSLOT() { Q_EMIT zoomFit(); }
  /// Shifts the visible window by a fraction of its width (negative = back).
  void             pan(double fraction);
  void             scrollToStart();
  void             scrollToEnd();
  /// Puts a picture of the graph on the clipboard.
  void             copyImageSLOT();
  /// @}
  void             startSLOT();
  void             stopSLOT();
  /// Export with a file dialog; returns false when cancelled or failed.
  bool             exportDataSLOT();
  void             importDataSLOT();
  void             connectSLOT(bool on) { m_connected = on; }

  /// File-path-driven, non-interactive halves of export/importDataSLOT (no QFileDialog),
  /// split out so the CSV parsing/writing logic can be exercised from tests.
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
  /// Returns true when the key was used.
  bool             handleChartKey(QKeyEvent *);

  void             emitInfo();
  bool             computeMinMax(double);
  void             rebuildSeries();
  void             updateXAxisRange();
  void             updateSeriesAppearance();
  void             updateThresholdLinesVisibility();
  void             updateThresholdLinePositions();
  QString          formatEngineeringValue(double value, QString *unit = Q_NULLPTR) const;

private:
  Qt::PenStyle     penStyle(LineMode);
  Settings        *m_cfg;

};

