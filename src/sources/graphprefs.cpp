//======================================================================
// File:		graphprefs.cpp
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:27:46 CEST 2002
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
#include "graphprefs.h"
#include "simplecfg.h"


GraphPrefs::GraphPrefs( QWidget *parent) : PrefWidget( parent )
{
  setupUi(this);
  m_label = tr( "Graph settings" );
  m_description = tr( "<b>Here you can configure the colors and"
					  " drawing style for the graph.</b>" );
  m_pixmap = new QPixmap(":/Symbols/graph.xpm" );
}
GraphPrefs::~GraphPrefs()
{
	delete m_pixmap;
}

void GraphPrefs::defaultsSLOT()
{
  // mt: removed .rgb()
  ui_bgColor->setColor( QColor( m_cfg->getRGB( "Graph", "background", Qt::white )));
  ui_gridColor->setColor( QColor( m_cfg->getRGB( "Graph", "grid", Qt::gray )));
  ui_dataColor->setColor( QColor( m_cfg->getRGB( "Graph", "data", Qt::blue )));
  ui_cursorColor->setColor( QColor( m_cfg->getRGB( "Graph", "cursor", Qt::black )));
  ui_startColor->setColor( QColor( m_cfg->getRGB( "Graph", "start-trigger", Qt::magenta )));
  ui_extColor->setColor( QColor( m_cfg->getRGB( "Graph", "external-trigger", Qt::cyan )));

  ui_lineMode->setCurrentIndex( m_cfg->getInt( "Graph", "line-mode", 1 ));
  ui_pointMode->setCurrentIndex( m_cfg->getInt( "Graph", "point-mode", 0 ));
  ui_crosshair->setChecked( m_cfg->getBool( "Graph", "crosshair-cursor", true ));

  ui_lineWidth->setValue( m_cfg->getInt( "Graph", "line-width", 2 ) );

}

void GraphPrefs::factoryDefaultsSLOT()
{
  ui_bgColor->setColor( Qt::white );
  ui_gridColor->setColor( Qt::gray );
  ui_dataColor->setColor( Qt::blue );
  ui_cursorColor->setColor( Qt::black );
  ui_startColor->setColor( Qt::magenta ); // mt: removed .rgb()
  ui_extColor->setColor( Qt::cyan ); // mt: removed .rgb()
  ui_lineMode->setCurrentIndex( 1 );
  ui_pointMode->setCurrentIndex( 0 );
  ui_lineWidth->setValue( 2 );
  ui_crosshair->setChecked( true );

}

void GraphPrefs::applySLOT()
{
  m_cfg->setRGB( "Graph", "background", ui_bgColor->color().rgb() );
  m_cfg->setRGB( "Graph", "grid", ui_gridColor->color().rgb() );
  m_cfg->setRGB( "Graph", "data", ui_dataColor->color().rgb() );
  m_cfg->setRGB( "Graph", "cursor", ui_cursorColor->color().rgb() );
  m_cfg->setRGB( "Graph", "start-trigger", ui_startColor->color().rgb() );
  m_cfg->setRGB( "Graph", "external-trigger", ui_extColor->color().rgb() );
  m_cfg->setInt( "Graph", "line-width", ui_lineWidth->value() );
  m_cfg->setInt( "Graph", "line-mode", ui_lineMode->currentIndex() );
  m_cfg->setInt( "Graph", "point-mode", ui_pointMode->currentIndex() );
  m_cfg->setBool( "Graph", "crosshair-cursor", ui_crosshair->isChecked() );
}

QColor GraphPrefs::bgColor() const
{
  return ui_bgColor->color();
}

QColor GraphPrefs::gridColor() const
{
  return ui_gridColor->color();
}

QColor GraphPrefs::dataColor() const
{
  return ui_dataColor->color();
}

QColor GraphPrefs::startColor() const
{
  return ui_startColor->color();
}

QColor GraphPrefs::externalColor() const
{
  return ui_extColor->color();
}

QColor GraphPrefs::cursorColor() const
{
  return ui_cursorColor->color();
}

int GraphPrefs::lineWidth() const
{
  return ui_lineWidth->value();
}

int GraphPrefs::lineMode() const
{
  return ui_lineMode->currentIndex();
}

int GraphPrefs::pointMode() const
{
  return ui_pointMode->currentIndex();
}

bool GraphPrefs::crosshair() const
{
  return ui_crosshair->isChecked();
}
