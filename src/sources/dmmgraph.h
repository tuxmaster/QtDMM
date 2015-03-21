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

#ifndef DMMGRAPH_HH
#define DMMGRAPH_HH

#include <QtGui>
#include <QtWidgets>

class QPrinter;

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
		IDConnect=1,
		IDDisconnect,
		IDStopRecorder,
		IDStartRecorder,
		IDClearGraph,
		IDConfigure,
		IDExportData,
		IDImportData
	  };

	  DMMGraph( QWidget *parent=0, const char *name=0 );
	  virtual					~DMMGraph();

	  void						setGraphSize( int size, int length );
	  void						addValue( double );
	  void						setUnit( const QString & );
	  void						setSampleTime( int v ) { m_sampleTime = v; }
	  void						setSampleLength( int v ) { m_sampleLength = v; }
	  void						setStartTime( const QTime & time ) { m_startTime = time; }
	  void						setMode( DMMGraph::SampleMode mode ) { m_mode = mode; }
	  void						print( QPrinter *prt, const QString &, const QString & );
	  void						setThresholds( double falling, double raising );
	  void						setScale( bool autoScale, bool includeZero, double min, double max );
	  void						setColors( const QColor & bg, const QColor & grid,
										   const QColor & data, const QColor & cursor,
										   const QColor & start, const QColor & external,
										   const QColor & integration, const QColor & intThreshold );
	  void						setLine( int d, int i );
	  void						setExternal( bool on, bool falling=false, double threshold=0 );
	  bool						dirty() const { return m_dirty; }
	  void						setAlertUnsaved( bool on ) { m_alertUnsaved = on; }
	  void						setCrosshair( bool on ) { m_crosshair = on; }
	  void						setLineStyle( int, int, int, int );
	  void						setIntegration( bool, double, double, double );

	Q_SIGNALS:
	  void						info( const QString & );
	  void						error( const QString & );
	  void						running( bool );
	  void						graphSize( int, int );
	  void						sampleTime( int );
	  void						externalTriggered();
	  void						zoomIn( double );
	  void						zoomOut( double );
	  void						thresholdChanged( DMMGraph::CursorMode, double );
	  void						connectDMM( bool );
	  void						configure();
	  void						exportData();
	  void						importData();

	public Q_SLOTS:
	  void						clearSLOT();
	  void						startSLOT();
	  void						stopSLOT();
	  bool						exportDataSLOT();
	  void						importDataSLOT();
	  void						connectSLOT( bool on ) { m_connected = on; }

	protected Q_SLOTS:
	  void						popupSLOT( int );

	protected:
	  QScrollBar                *scrollbar;
	  int                       m_size;
	  int                       m_length;
	  double                    m_scaleMin;
	  double                    m_scaleMax;
	  bool                      m_autoScale;
	  QVector<double>           *m_array;	// mt: changed from QArray to QVector
	  QVector<double>           *m_arrayInt;	// mt: changed from QArray to QVector
	  int                       m_pointer;
	  double                    m_yfactor;
	  double                    m_xfactor;
	  double                    m_ystep;
	  QString                   m_unit;
	  QString                   m_hUnit;
	  double                    m_xstep;
	  double                    m_hUnitFact;
	  double                    m_maxUnit;
	  int                       m_sampleTime;
	  int                       m_sampleLength;
	  bool                      m_running;
	  bool                      m_connected;
	  int                       m_sampleCounter;
	  int                       m_remainingLength;
	  SampleMode                m_mode;
	  QTime                     m_startTime;
	  QDateTime                 m_graphStartDateTime;
	  double                    m_sum;
	  bool                      m_first;
	  QPoint                    m_mpos;
	  QLabel                    *m_infoBox;
	  bool                      m_mouseDown;
	  double                    m_raisingThreshold;
	  double                    m_fallingThreshold;
	  double                    m_lastVal;
	  bool                      m_lastValValid;
	  QColor                    m_bgColor;
	  QColor                    m_gridColor;
	  QColor                    m_dataColor;
	  QColor                    m_cursorColor;
	  QColor                    m_startColor;
	  QColor                    m_externalColor;
	  QColor                    m_intColor;
	  QColor                    m_intThresholdColor;
	  int                       m_lineWidth;
	  int                       m_intLineWidth;
	  bool                      m_dirty;
	  bool                      m_alertUnsaved;
	  bool                      m_startExternal;
	  bool                      m_externalFalling;
	  double                    m_externalThreshold;
	  bool                      m_externalStarted;
	  bool                      m_crosshair;
	  PointMode                 m_pointMode;
	  PointMode                 m_intPointMode;
	  LineMode                  m_lineMode;
	  LineMode                  m_intLineMode;
	  QPointArray               m_drawArray;
	  double                    m_integrationScale;
	  double                    m_integrationThreshold;
	  double                    m_integrationOffset;
	  bool                      m_showIntegration;
	  double                    m_factor;
	  QString                   m_prefix;
	  QRect                     m_graphRect;
	  int                       m_fontHeight;
	  int                       m_triggerThresholdY;
	  int                       m_integrationThresholdY;
	  int                       m_externalThresholdY;
	  CursorMode                m_cursorMode;
	  bool                      m_includeZero;
	  QPopupMenu                *m_popup;
	  bool                      m_mousePan;

	  void						paintEvent( QPaintEvent * )Q_DECL_OVERRIDE;
	  void						resizeEvent( QResizeEvent * )Q_DECL_OVERRIDE;
	  void						mousePressEvent( QMouseEvent * )Q_DECL_OVERRIDE;
	  void						mouseMoveEvent( QMouseEvent * )Q_DECL_OVERRIDE;
	  void						mouseReleaseEvent( QMouseEvent * )Q_DECL_OVERRIDE;
	  void						wheelEvent( QWheelEvent * )Q_DECL_OVERRIDE;
	  void						timerEvent( QTimerEvent * )Q_DECL_OVERRIDE;

	  double					createYScale( int h, double & ystep );
	  double					createTimeScale( int w, double & xstep, double & hUnitFact,
												 double & maxUnit, QString & hUnit );
	  void						emitInfo();
	  void						paint( QPainter *p, int w, int h, double xfactor, double xstep,
									   double yfactor, double ystep, double maxUnit, double hUnitFact,
									   const QString & hUnit, bool color, bool printer );
	  void						paintHorizontalGrid( QPainter *p, double yfactor, double ystep, bool color );
	  void						paintVerticalGrid( QPainter *p, double xfactor, double xstep,
												   double maxUnit, double hUnitFact,
												   const QString & hUnit, bool color );
	  void						paintData( QPainter *p, double xfactor,
										   double yfactor, bool color, bool printer );
	  void						paintThresholds( QPainter *p, double xfactor,
												 double yfactor, bool color, bool printer );
	  void						drawCursor( const QPoint & );
	  void						fillInfoBox( const QPoint & );
	  void						drawPoint( PointMode, QPainter *, int, int );
	  void						computeUnitFactor();
	  bool						computeMinMax( double );

	private:
	  Qt::PenStyle				penStyle( LineMode );

};

#endif // DMMGRAPH_HH
