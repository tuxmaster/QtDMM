//======================================================================
// File:		integrationprefs.cpp
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:30:12 CEST 2002
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

#include "colorbutton.h"
#include "engnumbervalidator.h"
#include "integrationprefs.h"
#include "simplecfg.h"

IntegrationPrefs::IntegrationPrefs( QWidget *parent) : PrefWidget( parent)
{
  setupUi(this);
  m_label = tr( "Integration" );
  m_description = tr( "<b>Here you can configure the parameter"
					  " for the integration curve.</b>" );
  m_pixmap = new QPixmap(":/Symbols/integration.xpm" );

  EngNumberValidator *validator = new EngNumberValidator( this );

  ui_intScale->setValidator( validator );
  ui_intThreshold->setValidator( validator );
  ui_intOffset->setValidator( validator );
}
IntegrationPrefs::~IntegrationPrefs()
{
	delete m_pixmap;
}

void IntegrationPrefs::defaultsSLOT()
{
  ui_intColor->setColor( QColor( m_cfg->getRGB( "Graph", "integration", Qt::darkBlue )));  // mt: removed .rgb()
  ui_intThresholdColor->setColor( QColor( m_cfg->getRGB( "Graph", "integration-threshold", Qt::darkBlue ))); // mt: removed .rgb()
  ui_intLineMode->setCurrentIndex( m_cfg->getInt( "Graph", "int-line-mode", 0 ));
  ui_intPointMode->setCurrentIndex( m_cfg->getInt( "Graph", "int-point-mode", 1 ));
  ui_showInt->setChecked( m_cfg->getBool( "Graph", "show-integration", false ));
  ui_intScale->setText( m_cfg->getString( "Graph", "int-scale", "1.0" ));
  ui_intOffset->setText( m_cfg->getString( "Graph", "int-offset", "0.0" ));
  ui_intThreshold->setText( m_cfg->getString( "Graph", "int-threshold", "0.0" ));
  ui_intLineWidth->setValue( m_cfg->getInt( "Graph", "int-line-width", 2 ) );
}

void IntegrationPrefs::factoryDefaultsSLOT()
{
  ui_intColor->setColor( Qt::darkBlue );
  ui_intThresholdColor->setColor( Qt::darkBlue );	// mt: removed .rgb()
  ui_intLineMode->setCurrentIndex( 0 );
  ui_intPointMode->setCurrentIndex( 1 );
  ui_intLineWidth->setValue( 1 );
  ui_showInt->setChecked( false );
  ui_intScale->setText( "0.1" );
  ui_intOffset->setText( "0.0" );
  ui_intThreshold->setText( "0.0" );
}

void IntegrationPrefs::applySLOT()
{
  m_cfg->setRGB( "Graph", "integration", ui_intColor->color().rgb() );
  m_cfg->setRGB( "Graph", "integration-threshold", ui_intThresholdColor->color().rgb() );
  m_cfg->setInt( "Graph", "int-line-width", ui_intLineWidth->value() );
  m_cfg->setInt( "Graph", "int-line-mode", ui_intLineMode->currentIndex() );
  m_cfg->setInt( "Graph", "int-point-mode", ui_intPointMode->currentIndex() );
  m_cfg->setBool( "Graph", "show-integration", ui_showInt->isChecked() );
  m_cfg->setString( "Graph", "int-scale", ui_intScale->text() );
  m_cfg->setString( "Graph", "int-offset", ui_intOffset->text() );
  m_cfg->setString( "Graph", "int-threshold", ui_intThreshold->text() );
}

double IntegrationPrefs::intScale() const
{
  return EngNumberValidator::value( ui_intScale->text() );
}

double IntegrationPrefs::intThreshold() const
{
  return EngNumberValidator::value( ui_intThreshold->text() );
}

double IntegrationPrefs::intOffset() const
{
  return EngNumberValidator::value( ui_intOffset->text() );
}

void IntegrationPrefs::setThreshold( double value )
{
  ui_intThreshold->setText( EngNumberValidator::engValue( value ) );
}

bool IntegrationPrefs::showIntegration() const
{
  return ui_showInt->isChecked();
}

QColor IntegrationPrefs::intColor() const
{
  return ui_intColor->color();
}

QColor IntegrationPrefs::intThresholdColor() const
{
  return ui_intThresholdColor->color();
}

int IntegrationPrefs::intLineWidth() const
{
  return ui_intLineWidth->value();
}

int IntegrationPrefs::intLineMode() const
{
  return ui_intLineMode->currentIndex();
}

int IntegrationPrefs::intPointMode() const
{
  return ui_intPointMode->currentIndex();
}
