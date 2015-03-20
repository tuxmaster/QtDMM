//======================================================================
// File:		mainwin.cpp
// Author:	Matthias Toussaint
// Created:	Sun Sep  2 12:15:28 CEST 2001
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

#include <mainwin.h>
#include <mainwid.h>
#include <q3toolbar.h>
#include <qicon.h>
#include <qtoolbutton.h>
#include <q3popupmenu.h>
#include <qmenubar.h>
#include <q3action.h> // mt: changed to q3action
#include <qmessagebox.h>
#include <qlabel.h>
#include <qstatusbar.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3Frame>
#include <QPixmap>
#include <displaywid.h>

#include <connect_on.xpm>
#include <reset.xpm>
#include <start.xpm>
#include <stop.xpm>
#include <clear.xpm>
#include <print.xpm>
#include <export.xpm>
#include <import.xpm>
#include <config.xpm>
#include <quit.xpm>
#include <help.xpm>
#include <icon.xpm>

#define VERSION_STRING "0.9.0"

#include <iostream>

MainWin::MainWin( QWidget *parent, const char *name ) :
  Q3MainWindow( parent, name ),
  m_running( false )
{
  setIcon( QPixmap((const char **)icon_xpm) );
  
  m_wid = new MainWid( this );
  setCentralWidget( m_wid );
  
  createActions();
  createMenu();
  createToolBars();
  
  m_wid->setDisplay( m_display );
  
  setMinimumSize( 500, 450 );
  
  QString ver = "QtDMM ";
  ver += VERSION_STRING;
  
  setCaption( ver );
  
  connect( m_wid, SIGNAL( running(bool) ),
           this, SLOT( runningSLOT(bool) ));
  
  connectSLOT( false );
  
  m_error = new QLabel( statusBar() );
  m_error->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken );
  statusBar()->addWidget( m_error, 2, true );
  m_error->setLineWidth( 1 );  
  
  m_info = new QLabel( statusBar() );
  m_info->setFrameStyle( Q3Frame::Panel | Q3Frame::Sunken );
  statusBar()->addWidget( m_info, 1, true );
  m_info->setLineWidth( 1 );
  
  connect( m_wid, SIGNAL( error( const QString & ) ),
           m_error, SLOT( setText( const QString & ) ));
  connect( m_wid, SIGNAL( info( const QString & ) ),
           m_info, SLOT( setText( const QString & ) ));
  connect( m_wid, SIGNAL( useTextLabel( bool ) ),
           this, SLOT( setUsesTextLabel( bool ) ));
  connect( m_wid, SIGNAL( setConnect( bool ) ),
           this, SLOT( setConnectSLOT( bool ) ));
  connect( m_wid, SIGNAL( toolbarVisibility( bool, bool, bool, bool, bool )),
           this, SLOT( toolbarVisibilitySLOT( bool, bool, bool, bool, bool ) ));
  
  connect( m_wid, SIGNAL( connectDMM( bool ) ),
           m_connectAction, SLOT( setOn( bool ) ));
  
  QRect winRect = m_wid->winRect();
  m_wid->applySLOT();  
    
  //std::cerr << "WR: " << winRect.x() << " " << winRect.y() 
  //    << " " << winRect.width() << " " << winRect.height() << std::endl;
  //adjustSize();
  
  if (!winRect.isEmpty())
  {
    //std::cerr << "ISNT EMPTY" << std::endl;
    if (m_wid->saveWindowPosition()) 
    {
      move( winRect.x(), winRect.y() );
      //std::cerr << "MOVE: " << winRect.x() << " " <<  winRect.y() << std::endl;
    }
    if (m_wid->saveWindowSize()) 
    {
      //std::cerr << "RESIZE: " << winRect.width() << " " <<  winRect.height() << std::endl;
      resize( winRect.width(), winRect.height() ); 
    }
    else
    {
      resize( 640, 480 );
    }
  }
}

MainWin::~MainWin()
{
}
 
void
MainWin::setConsoleLogging( bool on )
{
  m_wid->setConsoleLogging( on );
}
      
void
MainWin::createActions()
{
  // mt: changed all QAction into Q3Action
  m_connectAction = new Q3Action ( tr("Connect"), 
                                  QIcon(QPixmap((const char **)connect_on_xpm)), 
                                  tr("Connect"), 
                                  Qt::CTRL+Qt::Key_C, 
                                  this, 0, true );
  m_connectAction->setWhatsThis( tr("<b>Connect to the Multimeter</b><p>This will establish"
      " the serial connection to the dmm. If not connected the serial port is free"
      " and can be used by other software." ));
  m_resetAction   = new Q3Action ( tr("Reset"), 
                                  QIcon(QPixmap((const char **)reset_xpm)), 
                                  tr("Reset"), 
                                  Qt::CTRL+Qt::Key_R, 
                                  this );
  m_resetAction->setWhatsThis( tr("<b>Reset min/max values</b><p>The min/max values in the"
      " display will be reset. You can activate this option at any time." ));
  m_startAction   = new Q3Action ( tr("Start"), 
                                  QIcon(QPixmap((const char **)start_xpm)), 
                                  tr("Start"), 
                                  Qt::CTRL+Qt::Key_S, 
                                  this );
  m_startAction->setWhatsThis( tr("<b>Start the recorder</b><p>If you are in manual mode"
      " this will start the recorder. Press F2 to set the recorder options" ));
  m_stopAction    = new Q3Action ( tr("Stop"), 
                                  QIcon(QPixmap((const char **)stop_xpm)), 
                                  tr("Stop"), 
                                  Qt::CTRL+Qt::Key_X, 
                                  this );
  m_stopAction->setWhatsThis( tr("<b>Stop the recorder</b><p>The recorder will be stopped."
      " This is independent from the start mode of the recorder" ));
  m_clearAction   = new Q3Action ( tr("Clear"), 
                                  QIcon(QPixmap((const char **)clear_xpm)), 
                                  tr("Clear"), 
                                  Qt::Key_Delete, 
                                  this );
  m_clearAction->setWhatsThis( tr("<b>Clear the recorder graph</b><p>If the recorder is already"
      " started it will clear the graph and continue recording." ));
  m_printAction   = new Q3Action ( tr("Print"), 
                                  QIcon(QPixmap((const char **)print_xpm)), 
                                  tr("Print ..."), 
                                  Qt::CTRL+Qt::Key_P, 
                                  this );
  m_printAction->setWhatsThis( tr("<b>Print recorder graph</b><p>A dialog will open where you can"
      " define a title and a comment for your printout. The printer itself can also be configured here."
      " To be able to print you need at least one working postscript printer configured in your"
      " system. Printing into a file is also supported." ));
  m_exportAction   = new Q3Action ( tr("Export"), 
                                  QIcon(QPixmap((const char **)export_xpm)), 
                                  tr("Export ..."), 
                                  Qt::CTRL+Qt::Key_E, 
                                  this );
  m_exportAction->setWhatsThis( tr("<b>Export recorder graph</b><p>Here you can export the recorded"
      " data as tab separated list. Each line contains the following values (separated by a tab "
      "character): date (dd.mm.yyyy) time (hh:mm:ss) value (float) unit." ));
  m_importAction   = new Q3Action ( tr("Import"), 
                                  QIcon(QPixmap((const char **)import_xpm)), 
                                  tr("Import ..."), 
                                  Qt::CTRL+Qt::Key_I, 
                                  this );
  m_importAction->setWhatsThis( tr("<b>Import data into recorder</b><p>Here you can import previously"
      " exported data files. QtDMM tries to do an educated guess if the file format is correct and"
      " rejects import of files which to not match." ));
  m_configAction   = new Q3Action ( tr("Configure"), 
                                  QIcon(QPixmap((const char **)config_xpm)), 
                                  tr("Configure ..."), 
                                  Qt::Key_F2, 
                                  this );
  m_configDmmAction   = new Q3Action ( tr("Configure"), 
                                  QIcon(QPixmap((const char **)config_xpm)), 
                                  tr("Configure ..."), 
                                  Qt::SHIFT+Qt::Key_F2, 
                                  this );
  m_configRecorderAction   = new Q3Action ( tr("Configure"), 
                                  QIcon(QPixmap((const char **)config_xpm)), 
                                  tr("Configure ..."), 
                                  Qt::CTRL+Qt::Key_F2, 
                                  this );
  m_configAction->setWhatsThis( tr("<b>Configure QtDMM</b><p>This will open QtDMM's configuration"
      " dialog. Here you can configure it's visual appearance and all options regarding the "
      "multimeter hardware and the recorder." ));
  m_configDmmAction->setWhatsThis( tr("<b>Configure QtDMM</b><p>This will open QtDMM's configuration"
      " dialog. Here you can configure it's visual appearance and all options regarding the "
      "multimeter hardware and the recorder." ));
  m_configRecorderAction->setWhatsThis( tr("<b>Configure QtDMM</b><p>This will open QtDMM's configuration"
      " dialog. Here you can configure it's visual appearance and all options regarding the "
      "multimeter hardware and the recorder." ));
  m_quitAction     = new Q3Action ( tr("Quit"), 
                                  QIcon(QPixmap((const char **)quit_xpm)), 
                                  tr("Quit"), 
                                  Qt::CTRL+Qt::Key_Q, 
                                  this );
  m_quitAction->setWhatsThis( tr("<b>Quit QtDMM</b><p>If the recorder contains unsaved data QtDMM"
      " will give you the option to savve your data first." ));
  m_helpAction     = new Q3Action ( tr("Help"), 
                                  QIcon(QPixmap((const char **)help_xpm)), 
                                  tr("Direct Help"), 
                                  Qt::SHIFT+Qt::Key_F1, 
                                  this );
  m_helpAction->setWhatsThis( tr("<b>Direct Help</b><p>Enter the direct help mode. You have done this"
      " already when reading this text :)" ));
  m_showTipsAction  = new Q3Action ( this );
  m_showTipsAction->setText( tr("Show tip of the day") );
  m_showTipsAction->setMenuText( tr("Tip of the day...") );
  m_versionAction  = new Q3Action ( this );
  m_versionAction->setText( tr("On version") );
  m_versionAction->setMenuText( tr("On version...") );
  m_versionAction->setWhatsThis( tr("<b>Copyright information</b><p>Show copyright information and some"
      " blurb about QtDMM." ));
  
  connect( m_connectAction, SIGNAL( toggled(bool) ),
           m_wid, SLOT( connectSLOT(bool) ));
  connect( m_connectAction, SIGNAL( toggled(bool) ),
           this, SLOT( connectSLOT(bool) ));
  connect( m_resetAction, SIGNAL( activated() ),
           m_wid, SLOT( resetSLOT() ));
  connect( m_startAction, SIGNAL( activated() ),
           m_wid, SLOT( startSLOT() ));
  connect( m_stopAction, SIGNAL( activated() ),
           m_wid, SLOT( stopSLOT() ));
  connect( m_clearAction, SIGNAL( activated() ),
           m_wid, SLOT( clearSLOT() ));
  connect( m_printAction, SIGNAL( activated() ),
           m_wid, SLOT( printSLOT() ));
  connect( m_importAction, SIGNAL( activated() ),
           m_wid, SLOT( importSLOT() ));
  connect( m_exportAction, SIGNAL( activated() ),
           m_wid, SLOT( exportSLOT() ));
  connect( m_configAction, SIGNAL( activated() ),
           m_wid, SLOT( configSLOT() ));
  connect( m_configDmmAction, SIGNAL( activated() ),
           m_wid, SLOT( configDmmSLOT() ));
  connect( m_configRecorderAction, SIGNAL( activated() ),
           m_wid, SLOT( configRecorderSLOT() ));
  connect( m_quitAction, SIGNAL( activated() ),
           this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_quitAction, SIGNAL( activated() ),
           m_wid, SLOT( quitSLOT() ));
  connect( m_helpAction, SIGNAL( activated() ),
           m_wid, SLOT( helpSLOT() ));
  connect( m_showTipsAction, SIGNAL( activated() ),
           m_wid, SLOT( showTipsSLOT() ));
  connect( m_versionAction, SIGNAL( activated() ),
           this, SLOT( versionSLOT() ));
  
}

void
MainWin::runningSLOT( bool on )
{
  m_running = on;
  
  m_startAction->setEnabled( !on );
  m_stopAction->setEnabled( on );
  m_printAction->setEnabled( !on );
  m_exportAction->setEnabled( !on );
  m_importAction->setEnabled( !on );
}

void
MainWin::connectSLOT( bool on )
{
  m_startAction->setEnabled( on );
  m_stopAction->setEnabled( on );
   
  m_startAction->setEnabled( on );
  m_stopAction->setEnabled( on && m_running );
  m_printAction->setEnabled( !(on || m_running) );
  m_exportAction->setEnabled( !(on || m_running) );
  m_importAction->setEnabled( !(on || m_running) );
  
  if (!on) m_running = false;
}

void
MainWin::versionSLOT()
{
  QString ver = "QtDMM ";
  ver += VERSION_STRING;
  QString msg = "<h1>";
  msg += ver;
  msg += "</h1><hr>"
         "<div align=right><i>A simple recorder for DMM's</i></div><p>"
         "<div align=justify>A simple display software for a variety of digital multimeter. Currently confirmed are:";

  msg += "<table>";
  msg += m_wid->deviceListText();
  msg += "</table>";

  msg +=
         "Other compatible models may work also.<p>"
         "QtDMM features min/max memory and a configurable "
         "recorder with import/export and printing function. Sampling may"
         " be started manually, at a given time or triggered by a measured threshold. "
         "Additionally an external program may be started when given thresholds are reached.</div>"
         "<div align=justify><b>QtDMM</b> uses the platform independent toolkit "
         "<b>Qt</b> version ";
  msg += qVersion();
  msg += " and is licensed under <b>GPL 3</b> (Versions prior to v0.9.0 where licensed under GPL 2)</div><br>"
         "&copy; 2001-2014 Matthias Toussaint &nbsp;-&nbsp;&nbsp;<font color=blue><u>qtdmm@mtoussaint.de</u></font>"
         "<p><br>The icons (except the DMM icon) have been taken from the KDE project.<p>";
          
  QMessageBox version( tr("QtDMM: Welcome!" ),
                       tr( msg ),
                       QMessageBox::Information,
                       QMessageBox::Yes | QMessageBox::Default,
                       Qt::NoButton,
                       Qt::NoButton );

  version.adjustSize();
  version.setButtonText( QMessageBox::Yes, tr("Ok") );
  version.setIconPixmap( QPixmap((const char **)icon_xpm ) );
  version.exec();
}

void
MainWin::createToolBars()
{
  m_dmmTB = new Q3ToolBar( this );
  m_connectAction->addTo( m_dmmTB );
  m_resetAction->addTo( m_dmmTB );
  addToolBar( m_dmmTB, tr("DMM"), Qt::DockTop, true );
  
  m_graphTB = new Q3ToolBar( this );
  m_startAction->addTo( m_graphTB );
  m_stopAction->addTo( m_graphTB );
  m_graphTB->addSeparator();
  m_clearAction->addTo( m_graphTB );
  addToolBar( m_graphTB, tr("Recorder") );
  
  m_fileTB = new Q3ToolBar( this );
  m_printAction->addTo( m_fileTB );
  m_exportAction->addTo( m_fileTB );
  m_importAction->addTo( m_fileTB );
  m_fileTB->addSeparator();
  m_configAction->addTo( m_fileTB );
  m_fileTB->addSeparator();
  m_quitAction->addTo( m_fileTB );
  addToolBar( m_fileTB, tr("File") );
  
  m_helpTB = new Q3ToolBar( this );
  m_helpAction->addTo( m_helpTB );
  addToolBar( m_helpTB, tr("Help") );
  
  m_displayTB = new Q3ToolBar( this );
  m_display = new DisplayWid( m_displayTB );
  addToolBar( m_displayTB, tr("Display"), Qt::DockTop, true );
  
  connect( m_displayTB, SIGNAL( visibilityChanged( bool ) ),
           this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_helpTB, SIGNAL( visibilityChanged( bool ) ),
           this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_fileTB, SIGNAL( visibilityChanged( bool ) ),
           this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_graphTB, SIGNAL( visibilityChanged( bool ) ),
           this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_dmmTB, SIGNAL( visibilityChanged( bool ) ),
           this, SLOT( setToolbarVisibilitySLOT() ));
}

void
MainWin::createMenu()
{
  QMenuBar *menu = menuBar();
  
  Q3PopupMenu *file = new Q3PopupMenu( menu );
  m_exportAction->addTo( file );
  m_importAction->addTo( file );
  file->insertSeparator();
  m_printAction->addTo( file );
  file->insertSeparator();
  m_configAction->addTo( file );
  file->insertSeparator();
  m_quitAction->addTo( file );
  
  menu->insertItem( tr("File"), file );
  
  Q3PopupMenu *dmm = new Q3PopupMenu( menu );
  m_connectAction->addTo( dmm );
  m_resetAction->addTo( dmm );
  dmm->insertSeparator();
  m_configDmmAction->addTo( dmm );
  
  menu->insertItem( tr("DMM"), dmm );
  
  Q3PopupMenu *recorder = new Q3PopupMenu( menu );
  m_startAction->addTo( recorder );
  m_stopAction->addTo( recorder );
  recorder->insertSeparator();
  m_clearAction->addTo( recorder );
  recorder->insertSeparator();
  m_configRecorderAction->addTo( recorder );
  
  menu->insertItem( tr("Recorder"), recorder );
  
  Q3PopupMenu *help = new Q3PopupMenu( menu );
  m_versionAction->addTo( help );
  m_showTipsAction->addTo( help );
  m_helpAction->addTo( help );
  
  menu->insertSeparator();
  menu->insertItem( tr("Help"), help );
  
}
     
void
MainWin::closeEvent( QCloseEvent *ev )
{
  setToolbarVisibilitySLOT();
                               
  if (m_wid->closeWin())
  {
    ev->accept();
  }
  else
  {
    ev->ignore();
  } 
}

void MainWin::setToolbarVisibilitySLOT()
{
  m_wid->setToolbarVisibility( m_displayTB->isVisible(),
                               m_dmmTB->isVisible(),
                               m_graphTB->isVisible(),
                               m_fileTB->isVisible(),
                               m_helpTB->isVisible() );
}

void
MainWin::setConnectSLOT( bool on )
{
  m_connectAction->setOn( on );
}

void
MainWin::toolbarVisibilitySLOT( bool disp, bool dmm, bool graph, 
                                bool file, bool help )
{
  m_dmmTB->setShown( dmm );
  m_graphTB->setShown( graph );
  m_fileTB->setShown( file );
  m_helpTB->setShown( help );
  m_displayTB->setShown( disp );
}

