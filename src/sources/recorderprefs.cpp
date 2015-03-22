//======================================================================
// File:		recorderprefs.cpp
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 14:59:20 CEST 2002
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
#include "recorderprefs.h"
#include  "simplecfg.h"

#define MINUTE_SECS   60
#define HOUR_SECS     60*60
#define DAY_SECS      60*60*24

RecorderPrefs::RecorderPrefs( QWidget *parent, const char *name ) : UIRecorderPrefs( parent, name )
{
  m_label = tr( "Recorder settings" );
  m_description = tr( "<b>Here you can configure the sampling"
					  " frequency and start options for the"
					  " recorder.</b>" );
  m_pixmap = new QPixmap(":/Symbols/recorder.xpm" );

  EngNumberValidator *validator = new EngNumberValidator( this );

  ui_raisingThreshold->setValidator( validator );
  ui_fallingThreshold->setValidator( validator );
}

void RecorderPrefs::defaultsSLOT()
{
  sampleEvery->setValue( m_cfg->getInt( "Sample", "rate", 1 ));
  ui_sampleUnit->setCurrentItem( m_cfg->getInt( "Sample", "rate-unit", 1 ));
  sampleTime->setValue( m_cfg->getInt( "Sample", "time", 500 ));
  timeUnit->setCurrentItem( m_cfg->getInt( "Sample", "time-unit", 0 ));

  DMMGraph::SampleMode mode = (DMMGraph::SampleMode)m_cfg->getInt( "Start", "mode", 0 );
  if (mode == DMMGraph::Manual)
	manualBut->setChecked( true );
  else if (mode == DMMGraph::Time)
	predefinedBut->setChecked( true );
  if (mode == DMMGraph::Raising)
  {
	triggerBut->setChecked( true );
	raisingBut->setChecked( true );
  }
  if (mode == DMMGraph::Falling)
  {
	triggerBut->setChecked( true );
	fallingBut->setChecked( true );
  }

  hour->setValue( m_cfg->getInt( "Start", "hour", 0 ));
  minute->setValue( m_cfg->getInt( "Start", "minute", 0 ));
  second->setValue( m_cfg->getInt( "Start", "second", 0 ));
  ui_raisingThreshold->setText( m_cfg->getString( "Start", "raising-threshold", "0.0" ));
  ui_fallingThreshold->setText( m_cfg->getString( "Start", "falling-threshold", "0.0" ));
}

void RecorderPrefs::factoryDefaultsSLOT()
{
  sampleEvery->setValue( 1 );
  ui_sampleUnit->setCurrentItem( 1 );
  sampleTime->setValue( 500 );
  timeUnit->setCurrentItem( 0 );

  manualBut->setChecked( true );

  hour->setValue( 0 );
  minute->setValue( 0 );
  second->setValue( 0 );
  ui_raisingThreshold->setText( "0.0" );
  ui_fallingThreshold->setText( "0.0" );
}

void RecorderPrefs::applySLOT()
{
  m_cfg->setInt( "Sample", "rate", sampleEvery->value() );
  m_cfg->setInt( "Sample", "rate-unit", ui_sampleUnit->currentItem() );
  m_cfg->setInt( "Sample", "time", sampleTime->value() );
  m_cfg->setInt( "Sample", "time-unit", timeUnit->currentItem() );

  m_cfg->setInt( "Start", "mode", sampleMode() );
  m_cfg->setInt( "Start", "hour", hour->value() );
  m_cfg->setInt( "Start", "minute", minute->value() );
  m_cfg->setInt( "Start", "second", second->value() );
  m_cfg->setString( "Start", "raising-threshold", ui_raisingThreshold->text() );
  m_cfg->setString( "Start", "falling-threshold", ui_fallingThreshold->text() );
}

DMMGraph::SampleMode RecorderPrefs::sampleMode() const
{
  if (predefinedBut->isChecked())
	return DMMGraph::Time;
  if (triggerBut->isChecked())
  {
	if (raisingBut->isChecked())
		return DMMGraph::Raising;
	return DMMGraph::Falling;
  }
  return DMMGraph::Manual;
}

int RecorderPrefs::sampleStep() const
{
  int thenthOfSec = sampleEvery->text().toInt();

  switch (ui_sampleUnit->currentItem())
  {
	  case 1:
		thenthOfSec *= 10;
		break;
	  case 2:
		thenthOfSec *= MINUTE_SECS * 10;
		break;
	  case 3:
		thenthOfSec *= HOUR_SECS * 10;
		break;
	  case 4:
		thenthOfSec *= DAY_SECS * 10;
		break;
  }
  return thenthOfSec;
}

int RecorderPrefs::sampleLength() const
{
  int thenthOfSec = sampleTime->text().toInt();

  switch (timeUnit->currentItem())
  {
	  case 0:
		thenthOfSec *= 10;
		break;
	  case 1:
		thenthOfSec *= MINUTE_SECS*10;
		break;
	  case 2:
		thenthOfSec *= HOUR_SECS*10;
		break;
	  case 3:
		thenthOfSec *= DAY_SECS*10;
		break;
  }
  return thenthOfSec;
}

double RecorderPrefs::fallingThreshold() const
{
  return EngNumberValidator::value( ui_fallingThreshold->text() );
}

double RecorderPrefs::raisingThreshold() const
{
  return EngNumberValidator::value( ui_raisingThreshold->text() );
}

void RecorderPrefs::setThreshold( double value )
{
  if (raisingBut->isChecked())
	ui_raisingThreshold->setText( EngNumberValidator::engValue( value ) );
  else
	ui_fallingThreshold->setText( EngNumberValidator::engValue( value ) );
}

QTime RecorderPrefs::startTime() const
{
  QTime time( hour->value(), minute->value(), second->value() );

  return time;
}

void RecorderPrefs::setSampleTimeSLOT( int sampleTime )
{
  sampleEvery->setValue( sampleTime );
}
