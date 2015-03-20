//======================================================================
// File:		configdlg.cpp
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 14:54:29 CEST 2002
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

#include <qcolordialog.h>
#include <qdir.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <QtPrintSupport>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QPixmap>

#include "configdlg.h"
#include "configitem.h"
#include "dmmprefs.h"
#include "executeprefs.h"
#include "graphprefs.h"
#include "guiprefs.h"
#include "integrationprefs.h"
#include "recorderprefs.h"
#include "scaleprefs.h"
#include "simplecfg.h"

#include <iostream>

#include <xpm/icon.xpm>

ConfigDlg::ConfigDlg( QWidget *parent, const char *name ) :  UIConfigDlg( parent, name )
{
  setIcon(QPixmap((const char **)icon_xpm));

  ui_list->header()->hide();
  ui_list->setItemMargin( 4 );

  // PREPARE CONFIGURATION FILE
  //
  QString path = QDir::homeDirPath();
  path += "/.qtdmmrc";

  m_cfg = new SimpleCfg( path );
  m_cfg->setComment( "#####################################################################\n"
					 "# This file was automagically created by QtDMM a simple DMM readout #\n"
					 "# software. QtDMM is copyright  by Matthias Toussaint. Don't change #\n"
					 "# this file unless you know what you are doing.                     #\n"
					 "#                                                                   #\n"
					 "# Contact: qtdmm@mtoussaint.de                                      #\n"
					 "#####################################################################\n\n" );

  // Check if configuration file exists. If not welcome user
  //
  QFile cfg( path );

  if (!cfg.exists())
  {
	QMessageBox welcome( tr("QtDMM: Welcome!" ),
						 tr("<font size=+2><b>Welcome!</b></font><p>"
							"This seems to be your first invocation of QtDMM "
							"(Or you have deleted it's configuration file).<p>QtDMM"
							" has created the file ~/.qtdmmrc in your home directory"
							" to save its settings."),
						 QMessageBox::Information,
						 QMessageBox::Yes | QMessageBox::Default,
						 Qt::NoButton,
						 Qt::NoButton );

	welcome.setButtonText( QMessageBox::Yes, tr("Continue") );
	welcome.setIconPixmap( QPixmap((const char **)icon_xpm ) );
	welcome.exec();

	m_cfg->save();
  }
  else
  {
	m_cfg->load();

	int version  = m_cfg->getInt( "QtDMM", "version", 0 );
	int revision = m_cfg->getInt( "QtDMM", "revision", 0 );

	if ((version <= 0 && revision < 84) || version >= 7)
	{
	  QMessageBox welcome( tr("QtDMM: Welcome!" ),
						   tr("<font size=+2><b>Welcome!</b></font><p>"
							  "You seem to have upgraded <b>QtDMM</b> from a version prior to 0.8.4"
							  " Please check your configuration. There are some new parameter to be"
							  " configured."
							  "<p>Thank you for choosing <b>QtDMM</b>.<p><i>Matthias Toussaint</i>"),
							QMessageBox::Information,
							QMessageBox::Yes | QMessageBox::Default,
							Qt::NoButton,
							Qt::NoButton );

	  welcome.setButtonText( QMessageBox::Yes, tr("Continue") );
	  welcome.setIconPixmap( QPixmap((const char **)icon_xpm ) );
	  welcome.exec();
	}
  }

  // CREATE PAGES (Top page last)
  //
  ui_list->setSorting( -1 );

  m_execute = new ExecutePrefs( ui_stack );
  m_execute->setId( ConfigDlg::External );
  new ConfigItem( m_execute->id(),
				  m_execute->pixmap(),
				  m_execute->label(),
				  ui_list );
  m_execute->setCfg( m_cfg );
  ui_stack->addWidget( m_execute, m_execute->id() );

  m_integration = new IntegrationPrefs( ui_stack );
  m_integration->setId( ConfigDlg::Integration );
  new ConfigItem( m_integration->id(),
				  m_integration->pixmap(),
				  m_integration->label(),
				  ui_list );
  m_integration->setCfg( m_cfg );
  ui_stack->addWidget( m_integration, m_integration->id() );

  m_graph = new GraphPrefs( ui_stack );
  m_graph->setId( ConfigDlg::Graph );
  new ConfigItem( m_graph->id(),
				  m_graph->pixmap(),
				  m_graph->label(),
				  ui_list );
  m_graph->setCfg( m_cfg );
  ui_stack->addWidget( m_graph, m_graph->id() );

  m_gui = new GuiPrefs( ui_stack );
  m_gui->setId( ConfigDlg::GUI );
  new ConfigItem( m_gui->id(),
				  m_gui->pixmap(),
				  m_gui->label(),
				  ui_list );
  m_gui->setCfg( m_cfg );
  ui_stack->addWidget( m_gui, m_gui->id() );

  m_dmm = new DmmPrefs( ui_stack );
  m_dmm->setId( ConfigDlg::DMM );
  new ConfigItem( m_dmm->id(),
				  m_dmm->pixmap(),
				  m_dmm->label(),
				  ui_list );
  m_dmm->setCfg( m_cfg );
  ui_stack->addWidget( m_dmm, m_dmm->id() );

  m_scale = new ScalePrefs( ui_stack );
  m_scale->setId( ConfigDlg::Scale );
  new ConfigItem( m_scale->id(),
				  m_scale->pixmap(),
				  m_scale->label(),
				  ui_list );
  m_scale->setCfg( m_cfg );
  ui_stack->addWidget( m_scale, m_scale->id() );

  m_recorder = new RecorderPrefs( ui_stack );
  m_recorder->setId( ConfigDlg::Recorder );
  new ConfigItem( m_recorder->id(),
				  m_recorder->pixmap(),
				  m_recorder->label(),
				  ui_list );
  m_recorder->setCfg( m_cfg );
  ui_stack->addWidget( m_recorder, m_recorder->id() );

  // Connections
  //
  connect( ui_ok, SIGNAL( clicked() ), this, SLOT( applySLOT() ));
  connect( ui_apply, SIGNAL( clicked() ), this, SLOT( applySLOT() ));
  connect( ui_cancel, SIGNAL( clicked() ), this, SLOT( cancelSLOT() ));
  connect( ui_factoryDefaults, SIGNAL( clicked() ),
		   this, SLOT( factoryDefaultsSLOT() ));

  connect( ui_list, SIGNAL( selectionChanged( Q3ListViewItem * ) ),
		   this, SLOT( pageSelectedSLOT( Q3ListViewItem * )));

  // init stuff
  //
  cancelSLOT();
  showPage( DMM );
  ui_undo->hide();
  adjustSize();
}

QString ConfigDlg::deviceListText() const
{
  return m_dmm->deviceListText();
}

void ConfigDlg::showPage( ConfigDlg::PageType page )
{
  for (ConfigItem *item=dynamic_cast<ConfigItem *> (ui_list->firstChild());
	   item;
	   item=dynamic_cast<ConfigItem *> (item->nextSibling()))
  {
	if (item->id() == page)
	{
	  ui_list->setSelected( item, true );
	  break;
	}
  }

  ui_stack->raiseWidget( page );

  PrefWidget *wid = dynamic_cast<PrefWidget *> (ui_stack->widget( page ));

  if (wid)
  {
	ui_helpText->setText( wid->description() );
	ui_helpPixmap->setPixmap( wid->pixmap() );
  }
}

void ConfigDlg::factoryDefaultsSLOT()
{
  ((PrefWidget *)ui_stack->visibleWidget())->factoryDefaultsSLOT();
}

void ConfigDlg::zoomInSLOT( double fac )
{
  m_scale->zoomInSLOT( fac );

  Q_EMIT zoomed();
}

void ConfigDlg::zoomOutSLOT( double fac )
{
  m_scale->zoomOutSLOT( fac );

  Q_EMIT zoomed();
}

void ConfigDlg::setSampleTimeSLOT( int sampleTime )
{
  m_recorder->setSampleTimeSLOT( sampleTime );

  applySLOT();
}

void ConfigDlg::setGraphSizeSLOT( int size, int length )
{
  m_scale->setGraphSizeSLOT( size, length );

  applySLOT();
}

void ConfigDlg::connectSLOT( bool /*connected*/ )
{
  //ui_ok->setDisabled( connected );
  //ui_apply->setDisabled( connected );
}

QRect ConfigDlg::winRect() const
{
  QRect rect( m_cfg->getInt( "Position", "x", 0 ),
			  m_cfg->getInt( "Position", "y", 0 ),
			  m_cfg->getInt( "Position", "width", 500 ),
			  m_cfg->getInt( "Position", "height", 350 ) );

  //std::cerr << "WRFC: " << rect.x() << " " << rect.y() << " " << rect.width()
  //    << " " << rect.height() << std::endl;
  return rect;
}

void ConfigDlg::setWinRect( const QRect & rect )
{
  m_winRect = rect;
}

void ConfigDlg::cancelSLOT()
{
  m_cfg->clear();
  m_cfg->load();

  int count = m_cfg->getInt( "Custom colors", "count", 0 );

  for (int i=0; i<count; i++)
  {
	QString color;
	color.sprintf( "color_%d", i );
	QColorDialog::setCustomColor( i, m_cfg->getRGB( "Custom colors", color, Qt::white ) );
  }

  for (int i=0; i<NumItems; ++i)
  {
	((PrefWidget *)ui_stack->widget( i ))->defaultsSLOT();
  }

  hide();
}

void ConfigDlg::setCurrentTipSLOT( int id )
{
  m_cfg->setInt( "QtDMM", "tip-id", id );
  m_cfg->save();
}

int ConfigDlg::currentTipId() const
{
  return m_cfg->getInt( "QtDMM", "tip-id", 0 );
}

void ConfigDlg::applySLOT()
{
  m_cfg->setInt( "Custom colors", "count", QColorDialog::customCount() );

  for (int i=0; i<QColorDialog::customCount(); i++)
  {
	QString color;
	color.sprintf( "color_%d", i );
	m_cfg->setRGB( "Custom colors", color, QColorDialog::customColor( i ) );
  }

  m_cfg->setInt( "Position", "x", m_winRect.x() );
  m_cfg->setInt( "Position", "y", m_winRect.y() );
  m_cfg->setInt( "Position", "width", m_winRect.width() );
  m_cfg->setInt( "Position", "height", m_winRect.height() );

  m_cfg->setInt( "Printer", "page-size", (int)m_printer->pageSize() );
  m_cfg->setInt( "Printer", "page-orientation", (int)m_printer->orientation() );
  m_cfg->setInt( "Printer", "color", (int)m_printer->colorMode() );
  m_cfg->setString( "Printer", "name", m_printer->printerName() );
  m_cfg->setString( "Printer", "filename", m_printer->outputFileName() );
  m_cfg->setBool( "Printer", "print-file", m_printer->outputToFile() );

  for (int i=0; i<NumItems; ++i)
  {
	((PrefWidget *)ui_stack->widget( i ))->applySLOT();
  }

  m_cfg->save();

  Q_EMIT accepted();

  if (sender() == ui_ok)
  {
	hide();
  }
}

void ConfigDlg::writePrinter( QPrinter * printer )
{
  m_printer = printer;

  applySLOT();
}

void ConfigDlg::readPrinter( QPrinter * printer )
{
  m_printer = printer;

  m_printer->setPageSize( (QPrinter::PageSize) m_cfg->getInt( "Printer", "page-size", 0 ) );
  m_printer->setOrientation( (QPrinter::Orientation) m_cfg->getInt( "Printer", "page-orientation", 0 ) );
  m_printer->setColorMode( (QPrinter::ColorMode) m_cfg->getInt( "Printer", "color", 1 ) );
  m_printer->setPrinterName( m_cfg->getString( "Printer", "name", "lp" ) );
  m_printer->setOutputFileName( m_cfg->getString( "Printer", "filename", "" ) );
  m_printer->setOutputToFile( m_cfg->getBool( "Printer", "print-file", false ) );
}

void ConfigDlg::pageSelectedSLOT(QListViewItem *item )
{
  int id = ((ConfigItem *)item)->id();
  PrefWidget *wid = (PrefWidget *)ui_stack->widget( id );

  ui_stack->raiseWidget( wid );

  ui_helpText->setText( wid->description() );
  ui_helpPixmap->setPixmap( wid->pixmap() );
}

void ConfigDlg::thresholdChangedSLOT( DMMGraph::CursorMode mode, double value )
{
  switch (mode)
  {
  case DMMGraph::Trigger:
	m_recorder->setThreshold( value );
	break;
  case DMMGraph::External:
	m_execute->setThreshold( value );
	break;
  case DMMGraph::Integration:
	m_integration->setThreshold( value );
	break;
  default:
	std::cerr << "Unexpected CursorMode in configdlg.cpp:418" << std::endl;
	break;
  }
}

/////////////////////////////////////////////////////////////////
// RECORDER
//
DMMGraph::SampleMode ConfigDlg::sampleMode() const
{
  return m_recorder->sampleMode();
}

int ConfigDlg::sampleStep() const
{
  return m_recorder->sampleStep();
}

int ConfigDlg::sampleLength() const
{
  return m_recorder->sampleLength();
}

double ConfigDlg::fallingThreshold() const
{
  return m_recorder->fallingThreshold();
}

double ConfigDlg::raisingThreshold() const
{
  return m_recorder->raisingThreshold();
}

QTime ConfigDlg::startTime() const
{
  return m_recorder->startTime();
}

/////////////////////////////////////////////////////////////////
// EXECUTE
//
bool ConfigDlg::startExternal() const
{
  return m_execute->startExternal();
}

bool ConfigDlg::externalFalling() const
{
  return m_execute->externalFalling();
}

double ConfigDlg::externalThreshold() const
{
  return m_execute->externalThreshold();
}

bool ConfigDlg::disconnectExternal() const
{
  return m_execute->disconnectExternal();
}

QString ConfigDlg::externalCommand() const
{
  return m_execute->externalCommand();
}

/////////////////////////////////////////////////////////////////
// GUI
//
bool ConfigDlg::showTip() const
{
  return m_gui->showTip();
}

bool ConfigDlg::showBar() const
{
  return m_gui->showBar();
}

bool ConfigDlg::showMinMax() const
{
  return m_gui->showMinMax();
}

bool ConfigDlg::alertUnsavedData() const
{
  return m_gui->alertUnsavedData();
}

bool ConfigDlg::useTextLabel() const
{
  return m_gui->useTextLabel();
}

QColor ConfigDlg::displayBgColor() const
{
  return m_gui->displayBgColor();
}

QColor ConfigDlg::displayTextColor() const
{
  return m_gui->displayTextColor();
}

bool ConfigDlg::saveWindowPosition() const
{
  return m_gui->saveWindowPosition();
}

bool ConfigDlg::saveWindowSize() const
{
  return m_gui->saveWindowSize();
}

void ConfigDlg::setShowTipsSLOT( bool on )
{
  m_gui->setShowTipsSLOT( on );
}

bool ConfigDlg::showDmmToolbar() const
{
  return m_gui->showDmmToolbar();
}

bool ConfigDlg::showGraphToolbar() const
{
  return m_gui->showGraphToolbar();
}

bool ConfigDlg::showFileToolbar() const
{
  return m_gui->showFileToolbar();
}

bool ConfigDlg::showHelpToolbar() const
{
  return m_gui->showHelpToolbar();
}

bool ConfigDlg::showDisplay() const
{
  return m_gui->showDisplay();
}

void ConfigDlg::setToolbarVisibility( bool disp, bool dmm, bool graph,
									  bool file, bool help )
{
  m_gui->setToolbarVisibility( disp, dmm, graph, file, help );
}

/////////////////////////////////////////////////////////////////
// SCALE
//
bool ConfigDlg::automaticScale() const
{
  return m_scale->automaticScale();
}

bool ConfigDlg::includeZero() const
{
  return m_scale->includeZero();
}

double ConfigDlg::scaleMin() const
{
  return m_scale->scaleMin();
}

double ConfigDlg::scaleMax() const
{
  return m_scale->scaleMax();
}

int ConfigDlg::windowSeconds() const
{
  return m_scale->windowSeconds();
}

int ConfigDlg::totalSeconds() const
{
  return m_scale->totalSeconds();
}

/////////////////////////////////////////////////////////////////
// INTEGRATION
//
double ConfigDlg::intScale() const
{
  return m_integration->intScale();
}

double ConfigDlg::intThreshold() const
{
  return m_integration->intThreshold();
}

double ConfigDlg::intOffset() const
{
  return m_integration->intOffset();
}

bool ConfigDlg::showIntegration() const
{
  return m_integration->showIntegration();
}

QColor ConfigDlg::intColor() const
{
  return m_integration->intColor();
}

QColor ConfigDlg::intThresholdColor() const
{
  return m_integration->intThresholdColor();
}

int ConfigDlg::intLineWidth() const
{
  return m_integration->intLineWidth();
}

int ConfigDlg::intLineMode() const
{
  return m_integration->intLineMode();
}

int ConfigDlg::intPointMode() const
{
  return m_integration->intPointMode();
}

/////////////////////////////////////////////////////////////////
// DMM
//
bool ConfigDlg::rts() const
{
  return m_dmm->rts();
}

bool ConfigDlg::cts() const
{
  return m_dmm->cts();
}

bool ConfigDlg::dsr() const
{
  return m_dmm->dsr();
}

bool ConfigDlg::dtr() const
{
  return m_dmm->dtr();
}

int ConfigDlg::parity() const
{
  return m_dmm->parity();
}

bool ConfigDlg::externalSetup() const
{
  return m_dmm->externalSetup();
}

int ConfigDlg::bits() const
{
  return m_dmm->bits();
}

int ConfigDlg::stopBits() const
{
  return m_dmm->stopBits();
}

int ConfigDlg::speed() const
{
  return m_dmm->speed();
}

int ConfigDlg::numValues() const
{
  return m_dmm->numValues();
}

ReadEvent::DataFormat ConfigDlg::format() const
{
  return m_dmm->format();
}

int ConfigDlg::display() const
{
  return m_dmm->display();
}

QString ConfigDlg::dmmName() const
{
  return m_dmm->dmmName();
}

QString ConfigDlg::device() const
{
  return m_dmm->device();
}

/////////////////////////////////////////////////////////////////
// GRAPH
//
bool ConfigDlg::crosshair() const
{
  return m_graph->crosshair();
}

QColor ConfigDlg::bgColor() const
{
  return m_graph->bgColor();
}

QColor ConfigDlg::gridColor() const
{
  return m_graph->gridColor();
}

QColor ConfigDlg::dataColor() const
{
  return m_graph->dataColor();
}

QColor ConfigDlg::startColor() const
{
  return m_graph->startColor();
}

QColor ConfigDlg::externalColor() const
{
  return m_graph->externalColor();
}

QColor ConfigDlg::cursorColor() const
{
  return m_graph->cursorColor();
}

int ConfigDlg::lineWidth() const
{
  return m_graph->lineWidth();
}

int ConfigDlg::lineMode() const
{
  return m_graph->lineMode();
}

int ConfigDlg::pointMode() const
{
  return m_graph->pointMode();
}
