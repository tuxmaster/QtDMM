//======================================================================
// File:		executeprefs.cpp
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:26:42 CEST 2002
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
#include "executeprefs.h"
#include "settings.h"


ExecutePrefs::ExecutePrefs( QWidget *parent) : PrefWidget( parent )
{
  setupUi(this);
  m_label = tr( "External application" );
  m_description = tr( "<b>Here you can configure if an external"
					  " command is to be started and when.</b>" );
  m_pixmap = new QPixmap(":/Symbols/execute.xpm");

  EngNumberValidator *validator = new EngNumberValidator( this );

  ui_execRaisingThreshold->setValidator( validator );
  ui_execFallingThreshold->setValidator( validator );

}
ExecutePrefs::~ExecutePrefs()
{
	delete m_pixmap;
}

void ExecutePrefs::defaultsSLOT()
{
  ui_executeCommand->setChecked( m_cfg->getBool( "External/exec"));

  ui_execRaising->setChecked( m_cfg->getBool( "External/raising", true ));
  ui_execRaisingThreshold->setText( m_cfg->getString( "External/raising-threshold"));
  ui_execFallingThreshold->setText( m_cfg->getString( "External/falling-threshold"));

  ui_disconnectExec->setChecked( m_cfg->getBool( "External/disconnect"));
  ui_commandExec->setText( m_cfg->getString( "External/command"));
}

void ExecutePrefs::factoryDefaultsSLOT()
{
  ui_executeCommand->setChecked( false );

  ui_execRaising->setChecked( true );
  ui_execRaisingThreshold->setText( "0" );
  ui_execFallingThreshold->setText( "0.1" );

  ui_disconnectExec->setChecked( false );
  ui_commandExec->setText( "" );
}

void ExecutePrefs::applySLOT()
{
  m_cfg->setBool( "External/exec", startExternal() );
  m_cfg->setBool( "External/raising", !externalFalling() );
  m_cfg->setString( "External/raising-threshold", ui_execRaisingThreshold->text() );
  m_cfg->setString( "External/falling-threshold", ui_execFallingThreshold->text() );
  m_cfg->setBool( "External/disconnect", disconnectExternal() );
  m_cfg->setString( "External/command", externalCommand() );
}

bool ExecutePrefs::startExternal() const
{
  return ui_executeCommand->isChecked();
}

bool ExecutePrefs::externalFalling() const
{
  return ui_execFalling->isChecked();
}

double ExecutePrefs::externalThreshold() const
{
  if (ui_execFalling->isChecked())
	return EngNumberValidator::value( ui_execFallingThreshold->text() == "" ? "0" : ui_execFallingThreshold->text());
  return EngNumberValidator::value( ui_execRaisingThreshold->text() == "" ? "0" : ui_execRaisingThreshold->text() );
}

void ExecutePrefs::setThreshold( double value )
{
  if (ui_execFalling->isChecked())
	ui_execFallingThreshold->setText( EngNumberValidator::engValue( value ) );
  else
	ui_execRaisingThreshold->setText( EngNumberValidator::engValue( value ) );
}

bool ExecutePrefs::disconnectExternal() const
{
  return ui_disconnectExec->isChecked();
}

QString ExecutePrefs::externalCommand() const
{
  return ui_commandExec->text();
}

void ExecutePrefs::on_ui_browseExec_clicked()
{
  QString filename = QFileDialog::getOpenFileName(this, QString(),QString(),"*");

  if (!filename.isEmpty())
  {
	ui_commandExec->setText( filename );
  }
}
