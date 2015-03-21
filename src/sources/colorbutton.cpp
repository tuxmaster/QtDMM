//======================================================================
// File:		colorbutton.cpp
// Author:	Matthias Toussaint
// Created:	Sam Jan 27 23:30:28 CET 2001
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

#include "colorbutton.h"

ColorButton::ColorButton( QWidget *parent, const char *name ) : QPushButton( parent, name )
{
  m_color = QColor( 255, 255, 255 );
  setAutoDefault( false );

  connect( this, SIGNAL( clicked() ), this, SLOT( clickedSLOT() ));
}

QColor ColorButton::color() const
{
  return m_color;
}

void ColorButton::setColor( const QColor & c )
{
  m_color = c;

  QImage img( 16, 12, 32 );
  img.fill(m_color.rgb());
  for (int i=0; i<16; ++i)
  {
	((QRgb *)img.scanLine(0))[i] = 0;
	((QRgb *)img.scanLine(11))[i] = 0;
  }

  for (int i=0; i<12; ++i)
  {
	((QRgb *)img.scanLine(i))[0] = 0;
	((QRgb *)img.scanLine(i))[15] = 0;
  }

  QPixmap pix;
  pix.convertFromImage( img );
  setPixmap( pix );
}

void ColorButton::clickedSLOT()
{
  QColor c = QColorDialog::getColor( color(), this );

  if (c.isValid())
  {
	setColor( c );

	Q_EMIT valueChanged();
	Q_EMIT valueChanged( c );
  }
}
/*
void
ColorButton::drawButtonLabel( QPainter *p )
{
  p->setBrush( color() );
  p->drawRoundRect( 6, 6, width()-12, height()-12, 30, 30 );
}
*/

