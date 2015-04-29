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

#include <QtGui>
#include <QtWidgets>

#include <iostream>

#include "mainwin.h"
#include "mainwid.h"
#include "displaywid.h"

#define VERSION_STRING "0.9.5"


MainWin::MainWin( QWidget *parent) : QMainWindow( parent),
  m_running( false )
{
  setWindowIcon( QPixmap(":/Symbols/icon.xpm") );

  m_wid = new MainWid( this );
  setCentralWidget( m_wid );

  createActions();
  createMenu();
  createToolBars();

  m_wid->setDisplay( m_display );

  setMinimumSize( 500, 450 );

  QString ver = "QtDMM ";
  ver += VERSION_STRING;

  setWindowTitle(ver );

  connect( m_wid, SIGNAL( running(bool) ),this, SLOT( runningSLOT(bool) ));

  connectSLOT( false );

  m_error = new QLabel( statusBar() );
  m_error->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statusBar()->addWidget( m_error);
  m_error->setLineWidth( 1 );

  m_info = new QLabel( statusBar() );
  m_info->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  statusBar()->addWidget( m_info );
  m_info->setLineWidth( 1 );

  connect( m_wid, SIGNAL( error( const QString & ) ), m_error, SLOT( setText( const QString & ) ));
  connect( m_wid, SIGNAL( info( const QString & ) ), m_info, SLOT( setText( const QString & ) ));
  connect( m_wid, SIGNAL( useTextLabel( bool ) ), this, SLOT( setUsesTextLabel( bool ) ));
  connect( m_wid, SIGNAL( setConnect( bool ) ), this, SLOT( setConnectSLOT( bool ) ));
  connect( m_wid, SIGNAL( toolbarVisibility( bool, bool, bool, bool, bool )),
		   this, SLOT( toolbarVisibilitySLOT( bool, bool, bool, bool, bool ) ));

  connect( m_wid, SIGNAL( connectDMM( bool ) ),m_connectAction, SLOT( setOn( bool ) ));

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
		resize( 640, 480 );
  }
}

void MainWin::setConsoleLogging( bool on )
{
  m_wid->setConsoleLogging( on );
}

void MainWin::createActions()
{
  // mt: changed all QAction into Q3Action

  m_connectAction = new QAction(QPixmap(":/Symbols/connect_on.xpm"),tr("Connect"),this);
  m_connectAction->setShortcut( Qt::CTRL+Qt::Key_C);
  m_connectAction->setWhatsThis( tr("<b>Connect to the Multimeter</b><p>This will establish"
	  " the serial connection to the dmm. If not connected the serial port is free"
	  " and can be used by other software." ));
  m_resetAction = new QAction(QPixmap(":/Symbols/reset.xpm"),tr("Reset"),this);
  m_resetAction->setShortcut(Qt::CTRL+Qt::Key_R);
  m_resetAction->setWhatsThis( tr("<b>Reset min/max values</b><p>The min/max values in the"
	  " display will be reset. You can activate this option at any time." ));
  m_startAction= new QAction(QPixmap(":/Symbols/start.xpm"), tr("Start"),this);
  m_startAction->setShortcut(Qt::CTRL+Qt::Key_S);
  m_startAction->setWhatsThis( tr("<b>Start the recorder</b><p>If you are in manual mode"
	  " this will start the recorder. Press F2 to set the recorder options" ));
  m_stopAction= new QAction(QPixmap(":/Symbols/stop.xpm"),tr("Stop"),this);
  m_stopAction->setShortcut(Qt::CTRL+Qt::Key_X);
  m_stopAction->setWhatsThis( tr("<b>Stop the recorder</b><p>The recorder will be stopped."
	  " This is independent from the start mode of the recorder" ));
  m_clearAction= new QAction(QPixmap(":/Symbols/clear.xpm"),tr("Clear"),this);
  m_clearAction->setShortcut(Qt::Key_Delete);
  m_clearAction->setWhatsThis( tr("<b>Clear the recorder graph</b><p>If the recorder is already"
	  " started it will clear the graph and continue recording." ));
  m_printAction= new QAction(QPixmap(":/Symbols/print.xpm"),tr("Print"),this);
  m_printAction->setShortcut(Qt::CTRL+Qt::Key_P);
  m_printAction->setWhatsThis( tr("<b>Print recorder graph</b><p>A dialog will open where you can"
	  " define a title and a comment for your printout. The printer itself can also be configured here."
	  " To be able to print you need at least one working postscript printer configured in your"
	  " system. Printing into a file is also supported." ));
  m_exportAction= new QAction(QPixmap(":/Symbols/export.xpm"),tr("Export"),this);
  m_exportAction->setShortcut(Qt::CTRL+Qt::Key_E);
  m_exportAction->setWhatsThis( tr("<b>Export recorder graph</b><p>Here you can export the recorded"
	  " data as tab separated list. Each line contains the following values (separated by a tab "
	  "character): date (dd.mm.yyyy) time (hh:mm:ss) value (float) unit." ));
  m_importAction= new QAction(QPixmap(":/Symbols/import.xpm"),tr("Import"),this);
  m_importAction->setShortcut(Qt::CTRL+Qt::Key_I);
  m_importAction->setWhatsThis( tr("<b>Import data into recorder</b><p>Here you can import previously"
	  " exported data files. QtDMM tries to do an educated guess if the file format is correct and"
	  " rejects import of files which to not match." ));
  m_configAction= new QAction(QPixmap(":/Symbols/config.xpm"),tr("Configure"),this);
  m_configAction->setShortcut(Qt::Key_F2);
  m_configDmmAction= new QAction(QPixmap(":/Symbols/config.xpm"),tr("Configure"),this);
  m_configDmmAction->setShortcut(Qt::SHIFT+Qt::Key_F2);
  m_configRecorderAction= new QAction(QPixmap(":/Symbols/config.xpm"),tr("Configure"),this);
  m_configRecorderAction->setShortcut(Qt::CTRL+Qt::Key_F2);
  m_configAction->setWhatsThis( tr("<b>Configure QtDMM</b><p>This will open QtDMM's configuration"
	  " dialog. Here you can configure it's visual appearance and all options regarding the "
	  "multimeter hardware and the recorder." ));
  m_configDmmAction->setWhatsThis( tr("<b>Configure QtDMM</b><p>This will open QtDMM's configuration"
	  " dialog. Here you can configure it's visual appearance and all options regarding the "
	  "multimeter hardware and the recorder." ));
  m_configRecorderAction->setWhatsThis( tr("<b>Configure QtDMM</b><p>This will open QtDMM's configuration"
	  " dialog. Here you can configure it's visual appearance and all options regarding the "
	  "multimeter hardware and the recorder." ));
  m_quitAction= new QAction(QPixmap(":/Symbols/quit.xpm"), tr("Quit"),this);
  m_quitAction->setShortcut(Qt::CTRL+Qt::Key_Q);
  m_quitAction->setWhatsThis( tr("<b>Quit QtDMM</b><p>If the recorder contains unsaved data QtDMM"
	  " will give you the option to savve your data first." ));
  m_helpAction= new QAction(QPixmap(":/Symbols/help.xpm"),tr("Direct Help"),this);
  m_helpAction->setShortcut(Qt::SHIFT+Qt::Key_F1);
  m_helpAction->setWhatsThis( tr("<b>Direct Help</b><p>Enter the direct help mode. You have done this"
	  " already when reading this text :)" ));
  m_showTipsAction  = new QAction ( this );
  m_showTipsAction->setText(  tr("Tip of the day") );
  m_showTipsAction->setWhatsThis(tr("Show tip of the day"));
  m_versionAction  = new QAction ( this );
  m_versionAction->setText( tr("On version") );
  m_versionAction->setWhatsThis( tr("<b>Copyright information</b><p>Show copyright information and some"
	  " blurb about QtDMM." ));

  connect( m_connectAction, SIGNAL( toggled(bool) ),m_wid, SLOT( connectSLOT(bool) ));
  connect( m_connectAction, SIGNAL( toggled(bool) ),this, SLOT( connectSLOT(bool) ));
  connect( m_resetAction, SIGNAL( triggered() ), m_wid, SLOT( resetSLOT() ));
  connect( m_startAction, SIGNAL( triggered() ), m_wid, SLOT( startSLOT() ));
  connect( m_stopAction, SIGNAL( triggered() ), m_wid, SLOT( stopSLOT() ));
  connect( m_clearAction, SIGNAL( triggered() ),m_wid, SLOT( clearSLOT() ));
  connect( m_printAction, SIGNAL( triggered() ), m_wid, SLOT( printSLOT() ));
  connect( m_importAction, SIGNAL( triggered() ),m_wid, SLOT( importSLOT() ));
  connect( m_exportAction, SIGNAL( triggered() ),m_wid, SLOT( exportSLOT() ));
  connect( m_configAction, SIGNAL( triggered() ),m_wid, SLOT( configSLOT() ));
  connect( m_configDmmAction, SIGNAL( triggered() ),m_wid, SLOT( configDmmSLOT() ));
  connect( m_configRecorderAction, SIGNAL( triggered() ), m_wid, SLOT( configRecorderSLOT() ));
  connect( m_quitAction, SIGNAL( triggered() ),this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_quitAction, SIGNAL( triggered() ), m_wid, SLOT( quitSLOT() ));
  connect( m_helpAction, SIGNAL( triggered() ),m_wid, SLOT( helpSLOT() ));
  connect( m_showTipsAction, SIGNAL( triggered() ),m_wid, SLOT( showTipsSLOT() ));
  connect( m_versionAction, SIGNAL( triggered() ), this, SLOT( versionSLOT() ));

}

void MainWin::runningSLOT( bool on )
{
  m_running = on;

  m_startAction->setEnabled( !on );
  m_stopAction->setEnabled( on );
  m_printAction->setEnabled( !on );
  m_exportAction->setEnabled( !on );
  m_importAction->setEnabled( !on );
}

void MainWin::connectSLOT( bool on )
{
  m_startAction->setEnabled( on );
  m_stopAction->setEnabled( on );

  m_startAction->setEnabled( on );
  m_stopAction->setEnabled( on && m_running );
  m_printAction->setEnabled( !(on || m_running) );
  m_exportAction->setEnabled( !(on || m_running) );
  m_importAction->setEnabled( !(on || m_running) );

  if (!on)
	  m_running = false;
}

void MainWin::versionSLOT()
{
  QMessageBox::about(this,tr("QtDMM: Welcome!" ),tr("<h1>QtDMM %1</h1><hr>"\
													"<div align=right><i>A simple recorder for DMM's</i></div><p>"\
													"<div align=justify>A simple display software for a variety of digital multimeter. Currently confirmed are:"\
													"<table>%2</table>Other compatible models may work also.<p>"\
													"QtDMM features min/max memory and a configurable "\
													"recorder with import/export and printing function. Sampling may"\
													" be started manually, at a given time or triggered by a measured threshold. "\
													"Additionally an external program may be started when given thresholds are reached.</div>"\
													 "<div align=justify><b>QtDMM</b> uses the platform independent toolkit "\
													"<b>Qt</b> version %3 and is licensed under <b>GPL 3</b> (Versions prior to v0.9.0 where licensed under GPL 2)</div><br>"\
													"&copy; 2001-2014 Matthias Toussaint &nbsp;-&nbsp;&nbsp;<font color=blue><u><a href='mailto:qtdmm@mtoussaint.de'>qtdmm@mtoussaint.de</a></u></font>"\
													"<p><br>The icons (except the DMM icon) have been taken from the KDE project.<p>")
					 .arg(VERSION_STRING).arg(m_wid->deviceListText()).arg(qVersion()));
}

void MainWin::createToolBars()
{
  m_dmmTB = new QToolBar(tr("DMM"), this );
  m_dmmTB->addAction(m_connectAction);
  m_dmmTB->addAction(m_resetAction);
  addToolBar( m_dmmTB);

  m_graphTB = new QToolBar( tr("Recorder"), this );
  m_graphTB->addAction(m_startAction);
  m_graphTB->addAction(m_stopAction);
  m_graphTB->addSeparator();
  m_graphTB->addAction(m_clearAction);
  addToolBar( m_graphTB);

  m_fileTB = new QToolBar( tr("File"), this );
  m_fileTB->addAction(m_printAction);
  m_fileTB->addAction(m_exportAction);
  m_fileTB->addAction(m_importAction);
  m_fileTB->addSeparator();
  m_fileTB->addAction(m_configAction);
  m_fileTB->addSeparator();
  m_fileTB->addAction(m_quitAction);
  addToolBar( m_fileTB );

  m_helpTB = new QToolBar(tr("Help"), this );
  m_helpTB->addAction(m_helpAction);
  addToolBar( m_helpTB );

  m_displayTB = new QToolBar(tr("Display"), this );
  m_display = new DisplayWid( m_displayTB );
  addToolBar( m_displayTB );

  connect( m_displayTB, SIGNAL( visibilityChanged( bool ) ), this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_helpTB, SIGNAL( visibilityChanged( bool ) ),  this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_fileTB, SIGNAL( visibilityChanged( bool ) ), this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_graphTB, SIGNAL( visibilityChanged( bool ) ), this, SLOT( setToolbarVisibilitySLOT() ));
  connect( m_dmmTB, SIGNAL( visibilityChanged( bool ) ), this, SLOT( setToolbarVisibilitySLOT() ));
}

void MainWin::createMenu()
{
  QMenuBar *menu = menuBar();

  QMenu *file = new QMenu(tr("File"), menu );
  file->addAction(m_exportAction);
  file->addAction(m_importAction);
  file->addSeparator();
  file->addAction(m_printAction);
  file->addSeparator();
  file->addAction(m_configAction);
  file->addSeparator();
  file->addAction(m_quitAction);
  menu->addMenu(file );

  QMenu *dmm = new QMenu(tr("DMM"), menu );
  dmm->addAction(m_connectAction);
  dmm->addAction(m_resetAction);
  dmm->addSeparator();
  dmm->addAction(m_configDmmAction);
  menu->addMenu(dmm);

  QMenu *recorder = new QMenu(tr("Recorder"), menu );
  recorder->addAction(m_startAction);
  recorder->addAction(m_stopAction);
  recorder->addSeparator();
  recorder->addAction(m_clearAction);
  recorder->addSeparator();
  recorder->addAction(m_configRecorderAction);
  menu->addMenu(  recorder );

  QMenu *help = new QMenu( tr("Help"),menu );
  help->addAction(m_versionAction);
  help->addAction(m_showTipsAction);
  help->addAction(m_helpAction);
  menu->addSeparator();
  menu->addMenu( help );

}

void MainWin::closeEvent( QCloseEvent *ev )
{
  setToolbarVisibilitySLOT();

  if (m_wid->closeWin())
	ev->accept();
  else
	ev->ignore();
}

void MainWin::setToolbarVisibilitySLOT()
{
  m_wid->setToolbarVisibility( m_displayTB->isVisible(),
							   m_dmmTB->isVisible(),
							   m_graphTB->isVisible(),
							   m_fileTB->isVisible(),
							   m_helpTB->isVisible() );
}

void MainWin::setConnectSLOT( bool on )
{
  m_connectAction->setChecked(on );
}

void MainWin::toolbarVisibilitySLOT( bool disp, bool dmm, bool graph, bool file, bool help )
{
  m_dmmTB->setVisible(dmm );
  m_graphTB->setVisible( graph );
  m_fileTB->setVisible( file );
  m_helpTB->setVisible( help );
  m_displayTB->setVisible( disp );
}

