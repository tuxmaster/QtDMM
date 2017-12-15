//======================================================================
// File:		scaleprefs.cpp
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:34:13 CEST 2002
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


#include <QtGui>
#include <QtWidgets>

#include "engnumbervalidator.h"
#include "scaleprefs.h"
#include "Settings.h"


#define MINUTE_SECS   60
#define HOUR_SECS     60*60
#define DAY_SECS      60*60*24

ScalePrefs::ScalePrefs( QWidget *parent ) : PrefWidget( parent )
{
  setupUi(this);
  m_label = tr( "Scale settings" );
  m_description = tr( "<b>Here you can configure the vertical scale"
					  " of the recorder and the length (in time)"
					  " of the window.</b>" );
  m_pixmap = new QPixmap(":/Symbols/scale.xpm" );

  EngNumberValidator *validator = new EngNumberValidator( this );

  ui_scaleMin->setValidator( validator );
  ui_scaleMax->setValidator( validator );
}
ScalePrefs::~ScalePrefs()
{
	delete m_pixmap;
}

void ScalePrefs::defaultsSLOT()
{
  bool autoScale = m_cfg->getBool( "Scale/automatic", true );
  if (autoScale)
	autoScaleBut->setChecked( true );
  else
	manualScaleBut->setChecked( true );

  bool includeZero = m_cfg->getBool( "Scale/automatic-include-zero", true );
  if (includeZero)
	ui_includeZero->setChecked( true );
  else
	ui_includeZero->setChecked( false );

  ui_scaleMin->setText( m_cfg->getString( "Scale/minimum", "-3.999" ));
  ui_scaleMax->setText( m_cfg->getString( "Scale/maximum", "3.999" ));

  ui_winSize->setValue( m_cfg->getInt( "Window/size", 600 ));
  sizeUnit->setCurrentIndex( m_cfg->getInt( "Window/size-unit"));
  winLength->setValue( m_cfg->getInt( "Window/length", 3600 ));
  lengthUnit->setCurrentIndex(m_cfg->getInt( "Window/length-unit"));
}

void ScalePrefs::factoryDefaultsSLOT()
{
  autoScaleBut->setChecked( true );
  ui_includeZero->setChecked( true );

  ui_scaleMin->setText( "-3.999" );
  ui_scaleMax->setText( "3.999" );

  ui_winSize->setValue( 600 );
  sizeUnit->setCurrentIndex( 0 );
  winLength->setValue( 3600 );
  lengthUnit->setCurrentIndex( 0 );
}

void ScalePrefs::applySLOT()
{
  m_cfg->setBool( "Scale/automatic", automaticScale() );
  m_cfg->setBool( "Scale/automatic-include-zero", includeZero() );
  m_cfg->setString( "Scale/minimum", ui_scaleMin->text() );
  m_cfg->setString( "Scale/maximum", ui_scaleMax->text() );
  m_cfg->setInt( "Window/size", ui_winSize->value() );
  m_cfg->setInt( "Window/size-unit", sizeUnit->currentIndex() );
  m_cfg->setInt( "Window/length", winLength->value() );
  m_cfg->setInt( "Window/length-unit", lengthUnit->currentIndex() );
}

bool ScalePrefs::includeZero() const
{
  return ui_includeZero->isChecked();
}

bool ScalePrefs::automaticScale() const
{
  return autoScaleBut->isChecked();
}

double ScalePrefs::scaleMin() const
{
  return EngNumberValidator::value( ui_scaleMin->text() );
}

double ScalePrefs::scaleMax() const
{
  return EngNumberValidator::value( ui_scaleMax->text() );
}

void ScalePrefs::setAutoScaleSLOT( bool autoScale )
{
  autoScaleBut->setChecked( autoScale );
}

void ScalePrefs::zoomInSLOT( double fac )
{
  double size = static_cast<double>(ui_winSize->value()) / fac;

  switch (sizeUnit->currentIndex())
  {
	case 0:
	  if (size < 10.0)
		size = 10.0;
	  break;
	case 1:
	  if (size < 1.0)
	  {
		size *= 60.0;
		sizeUnit->setCurrentIndex( 0 );
	  }
	  break;
	case 2:
	  if (size < 1.0)
	  {
		size *= 60.0;
		sizeUnit->setCurrentIndex( 1 );
	  }
	  break;
	case 3:
	  if (size < 1.0)
	  {
		size *= 24.0;
		sizeUnit->setCurrentIndex( 2 );
	  }
	  break;
  }

  ui_winSize->setValue( static_cast<int>(size ));

}

void ScalePrefs::zoomOutSLOT( double fac )
{
  double size = static_cast<double>(ui_winSize->value()) * fac;

  double winSec = static_cast<double>(windowSeconds()) * fac;
  double totalSec = static_cast<double>(totalSeconds());

  if (winSec <= totalSec)
  {
	switch (sizeUnit->currentIndex())
	{
	  case 0:
		if (size > 600)
		{
		  size /= 60.0;
		  sizeUnit->setCurrentIndex( 1 );
		}
		break;
	  case 1:
		if (size > 300)
		{
		  size /= 60.0;
		  sizeUnit->setCurrentIndex( 2 );
		}
		break;
	  case 2:
		if (size > 48)
		{
		  size /= 24.0;
		  sizeUnit->setCurrentIndex( 3 );
		}
		break;
	}

	ui_winSize->setValue( static_cast<int>(size));
  }
  else  // clamp to total length
  {
	ui_winSize->setValue( winLength->value() );
	sizeUnit->setCurrentIndex( lengthUnit->currentIndex() );
  }

}

int ScalePrefs::windowSeconds() const
{
  int sec = ui_winSize->text().toInt();

  switch (sizeUnit->currentIndex())
  {
	  case 1:
		sec *= MINUTE_SECS;
		break;
	  case 2:
		sec *= HOUR_SECS;
		break;
	  case 3:
		sec *= DAY_SECS;
		break;
  }
  return sec;
}

int ScalePrefs::totalSeconds() const
{
  int sec = winLength->text().toInt();

  switch (lengthUnit->currentIndex())
  {
	case 1:
	sec *= MINUTE_SECS;
	break;
	case 2:
	sec *= HOUR_SECS;
	break;
	case 3:
	sec *= DAY_SECS;
	break;
  }
  return sec;
}

void ScalePrefs::setGraphSizeSLOT( int size, int length )
{
  ui_winSize->setValue( size );
  winLength->setValue( length );
}
