//======================================================================
// File:		mainwid.cpp
// Author:	Matthias Toussaint
// Created:	Tue Apr 10 17:29:01 CEST 2001
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

#include <mainwid.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <q3whatsthis.h>
//Added by qt3to4:
#include <QTimerEvent>
#include <dmmgraph.h>
#include <qpixmap.h>
#include <qmessagebox.h>
#include <configdlg.h>
#include <q3process.h>
#include <printdlg.h>
#include <dmm.h>
#include <displaywid.h>
#include <tipdlg.h>

#include <icon.xpm>

#include <iostream>

MainWid::MainWid( QWidget *parent, const char *name ) :
  UIMainWid( parent, name ),
  m_display( 0 ),
  m_tipDlg( 0 )
{
  setIcon( QPixmap((const char **)icon_xpm ) );
  
  m_dmm = new DMM( this );
  m_external = new Q3Process( this );
  
  m_configDlg = new ConfigDlg( this );
  m_configDlg->hide();
  
  m_configDlg->readPrinter( &m_printer );
  
  m_printDlg = new PrintDlg( this );
  m_printDlg->hide();
  
  connect( m_dmm, SIGNAL( value( double, const QString &, 
                                 const QString &, const QString &, 
                                 bool, int )),
           this,  SLOT( valueSLOT( double, const QString &, 
                                   const QString &, const QString &, 
                                   bool, int )));
  connect( m_dmm, SIGNAL( error( const QString & ) ),
           this, SIGNAL( error( const QString & )));
  connect( ui_graph, SIGNAL( info( const QString & ) ),
           this, SIGNAL( info( const QString & ) ));
  connect( ui_graph, SIGNAL( error( const QString & ) ),
           this, SIGNAL( error( const QString & ) ));
  connect( ui_graph, SIGNAL( running( bool ) ),
           this, SLOT( runningSLOT( bool ) ));
  connect( m_configDlg, SIGNAL( accepted() ),
           this, SLOT( applySLOT() ));
  connect( m_configDlg, SIGNAL( zoomed() ),
           this, SLOT( zoomedSLOT() ));
  connect( ui_graph, SIGNAL( sampleTime( int ) ),
           m_configDlg, SLOT( setSampleTimeSLOT( int ) ));
  connect( ui_graph, SIGNAL( graphSize( int,int ) ),
           m_configDlg, SLOT( setGraphSizeSLOT( int,int ) ));
  connect( ui_graph, SIGNAL( externalTriggered() ),
           this, SLOT( startExternalSLOT() ));
  connect( m_external, SIGNAL( processExited() ),
           this, SLOT( exitedSLOT() ));  
  connect( ui_graph, SIGNAL( configure() ),
           this, SLOT( configSLOT() ));
  connect( ui_graph, SIGNAL( exportData() ),
           this, SLOT( exportSLOT() ));
  connect( ui_graph, SIGNAL( importData() ),
           this, SLOT( importSLOT() ));
  
  connect( ui_graph, SIGNAL( connectDMM( bool ) ),
           this, SIGNAL( connectDMM( bool ) ));
  
  connect( ui_graph, SIGNAL( zoomOut( double ) ),
           m_configDlg, SLOT( zoomOutSLOT( double ) ));
  connect( ui_graph, SIGNAL( zoomIn( double ) ),
           m_configDlg, SLOT( zoomInSLOT( double ) ));
  connect( ui_graph, SIGNAL( thresholdChanged( DMMGraph::CursorMode, double ) ),
           m_configDlg, SLOT( thresholdChangedSLOT( DMMGraph::CursorMode, double ) ));
  
  //resetSLOT();
  
  startTimer( 100 );
  
  if (m_configDlg->showTip())
  {
    showTipsSLOT();
  }
}

MainWid::~MainWid()
{
}

QString MainWid::deviceListText() const
{
  return m_configDlg->deviceListText();
}

void
MainWid::setConsoleLogging( bool on )
{
  m_dmm->setConsoleLogging( on );
}

void
MainWid::setDisplay( DisplayWid *display )
{
  m_display = display;
}

bool
MainWid::closeWin()
{
  m_dmm->close();
  m_configDlg->setWinRect( parentRect() );
  m_configDlg->applySLOT();
  
  emit setConnect( false );
  
  if (ui_graph->dirty() && m_configDlg->alertUnsavedData())
  {
    QMessageBox question( tr("QtDMM: Unsaved data" ),
                          tr("<font size=+2><b>Unsaved data</b></font><p>"
                             "You still have unsaved measured data in memory."
                             " If you quit now it will be lost."
                             "<p>Do you want to export your unsaved data first?" ),
                             QMessageBox::Information,
                             QMessageBox::Yes | QMessageBox::Default,
                             QMessageBox::No,
                             QMessageBox::Cancel | QMessageBox::Escape );
    
    question.setButtonText( QMessageBox::Yes, tr("Export data first") );
    question.setButtonText( QMessageBox::No, tr("Quit without saving") );
    question.setIconPixmap( QPixmap((const char **)icon_xpm ) );
    
    switch (question.exec())
    {
    case QMessageBox::Yes:
      return ui_graph->exportDataSLOT();
      
    case QMessageBox::No:
      break;
      
    case QMessageBox::Cancel:
      return false;
    }
  }
  
  return true;
}

QRect
MainWid::winRect() const
{ 
  return m_configDlg->winRect();
}

bool
MainWid::saveWindowPosition() const
{
  return m_configDlg->saveWindowPosition();
}

bool
MainWid::saveWindowSize() const
{
  return m_configDlg->saveWindowSize();
}

QRect
MainWid::parentRect() const
{
  QRect fRect = parentWidget()->frameGeometry();
  QRect rect  = parentWidget()->rect();
  
  return QRect( fRect.x(), fRect.y(), rect.width(), rect.height() ); 
}

void
MainWid::timerEvent( QTimerEvent * )
{
  ui_graph->addValue( m_dval );
}

void
MainWid::valueSLOT( double dval,
                    const QString & val, 
                    const QString & u, 
                    const QString & s,
                    bool showBar,
                    int id )
{
/*  cerr << "valueSLOT " << dval
       << " val=" << val.latin1()
       << " u=" << u.latin1() 
       << " s=" << s.latin1()
       << " showBar=" << showBar
       << " id=" << id << endl;  */
  
  m_display->setShowBar( showBar );
  m_display->setValue( id, val );
  
  m_display->setMode( id, s );

  QString tmpUnit = u;

  m_display->setUnit( id, tmpUnit );
  
  if (0 == id)
  {
    if (m_lastUnit != s)
    {
      resetSLOT();
      ui_graph->setUnit( u );
    }
    m_lastUnit = s;
  
    if (dval > m_max)
    {
      m_max = dval;  
      m_display->setMaxUnit( u );
      m_display->setMaxValue( val );
    }

    if (dval < m_min)
    {
      m_min = dval;  
      m_display->setMinUnit( u );
      m_display->setMinValue( val );
    }
    
    m_dval = dval;
  }
  
  m_display->update();
}

void
MainWid::resetSLOT()
{
  m_min =  1.0E20;
  m_max = -1.0E20;
  
  m_display->setMinValue( "" );
  m_display->setMaxValue( "" );
  m_display->setMinUnit( "" );
  m_display->setMaxUnit( "" );
}

void
MainWid::connectSLOT( bool on )
{
  if (on)
  {
    m_dmm->open();
    ui_graph->clearSLOT();
  }
  else
  {
    m_dmm->close();
    ui_graph->stopSLOT();
  }
  
  m_configDlg->connectSLOT( on );
  
  ui_graph->connectSLOT( on );
}

void
MainWid::quitSLOT()
{
  if (closeWin()) qApp->quit();
}

void
MainWid::helpSLOT()
{
  Q3WhatsThis::enterWhatsThisMode();
}

void
MainWid::configSLOT()
{
  m_configDlg->show();
  m_configDlg->raise();
}

void
MainWid::configDmmSLOT()
{
  m_configDlg->show();
  m_configDlg->raise();
  
  m_configDlg->showPage( ConfigDlg::DMM );
}

void
MainWid::configRecorderSLOT()
{
  m_configDlg->show();
  m_configDlg->raise();
  
  m_configDlg->showPage( ConfigDlg::Recorder );
}

void
MainWid::applySLOT()
{
  readConfig();
  
  ui_graph->setAlertUnsaved( m_configDlg->alertUnsavedData() );
  m_dmm->setName( m_configDlg->dmmName() );
}

void
MainWid::zoomedSLOT()
{
  ui_graph->setGraphSize( m_configDlg->windowSeconds(),
                          m_configDlg->totalSeconds() );  
}

void
MainWid::exportSLOT()
{
  ui_graph->exportDataSLOT();
}

void
MainWid::importSLOT()
{
  ui_graph->importDataSLOT();
}

void
MainWid::printSLOT()
{
  m_printDlg->setPrinter( &m_printer );
  
  if (m_printDlg->exec())
  {
    m_configDlg->writePrinter( &m_printer );
    ui_graph->print( &m_printer, m_printDlg->title(), m_printDlg->comment() );    
  }
}

void
MainWid::clearSLOT()
{
  ui_graph->clearSLOT();
}

void
MainWid::startSLOT()
{
  ui_graph->startSLOT();
}

void
MainWid::stopSLOT()
{
  ui_graph->stopSLOT();
}

void
MainWid::readConfig()
{
  bool reopen = false;
  
  if (m_dmm->isOpen())
  {
    m_dmm->close();
    reopen = true;
  }
  
  m_dmm->setDevice( m_configDlg->device() );
  m_dmm->setSpeed( m_configDlg->speed() );
  m_dmm->setFormat( m_configDlg->format() );
  m_dmm->setPortSettings( m_configDlg->bits(), m_configDlg->stopBits(), 
                          m_configDlg->parity(), m_configDlg->externalSetup(),
                          m_configDlg->rts(), m_configDlg->cts(), 
                          m_configDlg->dsr(), m_configDlg->dtr() 
                        );
  
  ui_graph->setGraphSize( m_configDlg->windowSeconds(),
                          m_configDlg->totalSeconds() );
  ui_graph->setStartTime( m_configDlg->startTime() );
  ui_graph->setMode( m_configDlg->sampleMode() );
  
  ui_graph->setSampleTime( m_configDlg->sampleStep() );
  ui_graph->setSampleLength( m_configDlg->sampleLength() );
  
  ui_graph->setCrosshair( m_configDlg->crosshair() );
  
  ui_graph->setThresholds( m_configDlg->fallingThreshold(), 
                           m_configDlg->raisingThreshold() );
  
  ui_graph->setScale( m_configDlg->automaticScale(), 
                      m_configDlg->includeZero(),
                      m_configDlg->scaleMin(),
                      m_configDlg->scaleMax() );
  
  ui_graph->setColors( m_configDlg->bgColor(), 
                       m_configDlg->gridColor(),
                       m_configDlg->dataColor(),
                       m_configDlg->cursorColor(),
                       m_configDlg->startColor(),
                       m_configDlg->externalColor(),
                       m_configDlg->intColor(),
                       m_configDlg->intThresholdColor() );
  
  ui_graph->setExternal( m_configDlg->startExternal(),
                         m_configDlg->externalFalling(),
                         m_configDlg->externalThreshold() );
  
  ui_graph->setLineStyle( m_configDlg->lineMode(),
                          m_configDlg->pointMode(),
                          m_configDlg->intLineMode(),
                          m_configDlg->intPointMode() );
      
  QColorGroup cg = colorGroup();
  cg.setColor( QColorGroup::Background, m_configDlg->displayBgColor() );
  cg.setColor( QColorGroup::Foreground, m_configDlg->displayTextColor() );
  
  m_display->setPalette( QPalette( cg, cg, cg ) );
  m_display->setDisplayMode( m_configDlg->display(), m_configDlg->showMinMax(),
                             m_configDlg->showBar(), m_configDlg->numValues() );      
  m_dmm->setNumValues( m_configDlg->numValues() );
  
  ui_graph->setLine( m_configDlg->lineWidth(), m_configDlg->intLineWidth() );
  
  ui_graph->setIntegration( m_configDlg->showIntegration(),
                            m_configDlg->intScale(),
                            m_configDlg->intThreshold(),
                            m_configDlg->intOffset() );
   
  if (m_configDlg->sampleMode() == DMMGraph::Time)
  {
    QString txt;
    txt.sprintf( "%s %s", tr( "Automatic start at" ).latin1(),
        m_configDlg->startTime().toString().latin1() );
    
    emit info( txt );
  }
  else if (m_configDlg->sampleMode() == DMMGraph::Raising)
  {
    QString txt;
    txt.sprintf( "%s %g", tr( "Raising threshold" ).latin1(),
        m_configDlg->raisingThreshold() );
    
    emit info( txt );
  }
  else if (m_configDlg->sampleMode() == DMMGraph::Falling)
  {
    QString txt;
    txt.sprintf( "%s %g", tr( "Falling threshold" ).latin1(),
        m_configDlg->fallingThreshold() );
    
    emit info( txt );
  }
  
  emit useTextLabel( m_configDlg->useTextLabel() );
  emit toolbarVisibility( m_configDlg->showDisplay(),
                          m_configDlg->showDmmToolbar(),
                          m_configDlg->showGraphToolbar(),
                          m_configDlg->showFileToolbar(),
                          m_configDlg->showHelpToolbar() );
                          
  if (reopen)
  {
    m_dmm->open();
  }
}

void
MainWid::runningSLOT( bool on )
{
  emit running( on );
}

void
MainWid::startExternalSLOT()
{
  if (m_external->isRunning())
  {
    QString msg;
    msg.sprintf( tr("<font size=+2><b>Launch error</b></font><p>"
                    "Application %s is still running!<p>"
                    "Do you want to kill it now"), 
                    m_configDlg->externalCommand().latin1() );
    
    QMessageBox question( tr("QtDMM: Launch error" ),
                          msg,
                             QMessageBox::Information,
                             QMessageBox::Yes | QMessageBox::Default,
                             QMessageBox::No,
                             Qt::NoButton );
    
    question.setButtonText( QMessageBox::Yes, tr("Yes, kill it!") );
    question.setButtonText( QMessageBox::Yes, tr("No, keep running") );
    question.setIconPixmap( QPixmap((const char **)icon_xpm ) );
    
    switch (question.exec())
    {
    case QMessageBox::Yes:
      m_external->kill();
      break;
    default:
      return;
    }
  }
  
  if (m_configDlg->disconnectExternal())
  {
    emit setConnect( false );
  }
  
  QStringList args;
  args.append( m_configDlg->externalCommand() );
  m_external->setArguments( args );
  
  // mt: call with empty string
  if (!m_external->launch(QString()))
  {
    QString msg;
    msg.sprintf( tr("<font size=+2><b>Launch error</b></font><p>"
                    "Couldn't launch %s"), 
                    m_configDlg->externalCommand().latin1() );
    
    QMessageBox question( tr("QtDMM: Launch error" ),
                          msg,
                             QMessageBox::Information,
                             QMessageBox::Yes | QMessageBox::Default,
                             Qt::NoButton,
                             Qt::NoButton );
    
    question.setButtonText( QMessageBox::Yes, tr("Bummer!") );
    question.setIconPixmap( QPixmap((const char **)icon_xpm ) );
    
    question.exec();
  }
  else
  {
    QString msg;
    msg.sprintf( tr("Launched %s"), m_configDlg->externalCommand().latin1() );
    emit error( msg );
  }
}

void
MainWid::exitedSLOT()
{
  QString msg;
  msg.sprintf( "%s terminated with exit code %d.",
      m_configDlg->externalCommand().latin1(), m_external->exitStatus() );
  
  emit error( msg );
}

void
MainWid::showTipsSLOT()
{
  if (!m_tipDlg)
  {
    m_tipDlg = new TipDlg( this );
    
    m_tipDlg->setShowTipsSLOT( m_configDlg->showTip() );
    m_tipDlg->setCurrentTip( m_configDlg->currentTipId() );
    
    connect( m_tipDlg, SIGNAL( showTips( bool ) ),
             m_configDlg, SLOT( setShowTipsSLOT( bool ) ));
    connect( m_configDlg, SIGNAL( showTips( bool ) ),
             m_tipDlg, SLOT( setShowTipsSLOT( bool ) ));
    connect( m_tipDlg, SIGNAL( currentTip( int ) ),
             m_configDlg, SLOT( setCurrentTipSLOT( int ) ));
    
  }
  
  m_tipDlg->show();
}

void MainWid::setToolbarVisibility( bool disp, bool dmm, bool graph,
                                    bool file, bool help )
{
  m_configDlg->setToolbarVisibility( disp, dmm, graph, file, help );
}
