//======================================================================
// File:		displaywid.cpp
// Author:	Matthias Toussaint
// Created:	Fri Nov 23 22:34:30 CET 2001
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

#include "displaywid.h"

#include <iostream>

DisplayWid::DisplayWid(QWidget *parent) : QWidget( parent),
  m_paintBar( true ),
  m_numValues( 1 )
{

  m_bigDigit = BitmapHelper(":/Symbols/numbers.xpm");
  m_bigSpecialChar =BitmapHelper(":/Symbols/specialchars.xpm");

  m_bigDecimal =  BitmapHelper(":/Symbols/decimal.xpm");
  m_bigMinus =  BitmapHelper(":/Symbols/minus.xpm");
  m_bigG = BitmapHelper(":/Symbols/G.xpm");
  m_bigM =  BitmapHelper(":/Symbols/M.xpm");
  m_bigk =  BitmapHelper(":/Symbols/k.xpm");
  m_bigm =  BitmapHelper(":/Symbols/m.xpm");
  m_bigu =  BitmapHelper(":/Symbols/µ.xpm");
  m_bign =  BitmapHelper(":/Symbols/n.xpm");
  m_bigp =  BitmapHelper(":/Symbols/p.xpm");
  m_bigHz =  BitmapHelper(":/Symbols/Hz.xpm");
  m_bigF =  BitmapHelper(":/Symbols/F.xpm");
  m_bigV =  BitmapHelper(":/Symbols/V.xpm");
  m_bigVA =  BitmapHelper(":/Symbols/VA.xpm");
  m_bigcosphi =  BitmapHelper(":/Symbols/cosphi.xpm");
  m_bigA =  BitmapHelper(":/Symbols/A.xpm");
  m_bigH =  BitmapHelper(":/Symbols/H.xpm");
  m_bigW =  BitmapHelper(":/Symbols/W.xpm");
  m_bigDBM =  BitmapHelper(":/Symbols/DBM.xpm");
  m_bigOhm =  BitmapHelper(":/Symbols/Ohm.xpm");
  m_bigDeg =  BitmapHelper(":/Symbols/deg.xpm");
  m_bigPercent =  BitmapHelper(":/Symbols/percent.xpm");

  m_smallDigit =  BitmapHelper(":/Symbols/numbers_small.xpm");
  m_smallSpecialChar =  BitmapHelper(":/Symbols/specialchars_small.xpm");

  m_smallDecimal =  BitmapHelper(":/Symbols/decimal_small.xpm");
  m_smallMinus =  BitmapHelper(":/Symbols/minus_small.xpm");
  m_smallG =  BitmapHelper(":/Symbols/G_small.xpm");
  m_smallM =  BitmapHelper(":/Symbols/M_small.xpm");
  m_smallk =  BitmapHelper(":/Symbols/k_small.xpm");
  m_smallm =  BitmapHelper(":/Symbols/m_small.xpm");
  m_smallu =  BitmapHelper(":/Symbols/µ_small.xpm");
  m_smalln =  BitmapHelper(":/Symbols/n_small.xpm");
  m_smallp =  BitmapHelper(":/Symbols/p_small.xpm");
  m_smallHz =  BitmapHelper(":/Symbols/Hz_small.xpm");
  m_smallF =  BitmapHelper(":/Symbols/F_small.xpm");
  m_smallV =  BitmapHelper(":/Symbols/V_small.xpm");
  m_smallVA =  BitmapHelper(":/Symbols/VA_small.xpm");
  m_smallcosphi =  BitmapHelper(":/Symbols/cosphi_small.xpm");
  m_smallA =  BitmapHelper(":/Symbols/A_small.xpm");
  m_smallH =  BitmapHelper(":/Symbols/H_small.xpm");
  m_smallW =  BitmapHelper(":/Symbols/W_small.xpm");
  m_smallDBM =  BitmapHelper(":/Symbols/DBM_small.xpm");
  m_smallOhm =  BitmapHelper(":/Symbols/Ohm_small.xpm");
  m_smallDeg =  BitmapHelper(":/Symbols/deg_small.xpm");
  m_smallPercent =  BitmapHelper(":/Symbols/percent_small.xpm");

  m_minStr =  BitmapHelper(":/Symbols/min_str.xpm");
  m_maxStr =  BitmapHelper(":/Symbols/max_str.xpm");

  m_diode =  BitmapHelper(":/Symbols/diode.xpm");
  m_ac =  BitmapHelper(":/Symbols/ac.xpm");
  m_dc =  BitmapHelper(":/Symbols/dc.xpm");

  m_bar[0] =  BitmapHelper(":/Symbols/null_bar.xpm");
  m_bar[1] =  BitmapHelper(":/Symbols/ten_bar.xpm");
  m_bar[2] =  BitmapHelper(":/Symbols/twenty_bar.xpm");
  m_bar[3] =  BitmapHelper(":/Symbols/thirty_bar.xpm");
  m_bar[4] =  BitmapHelper(":/Symbols/fourty_bar.xpm");
  m_bar[5] =  BitmapHelper(":/Symbols/fifty_bar.xpm");
  m_bar[6] =  BitmapHelper(":/Symbols/sixty_bar.xpm");

  setAttribute(Qt::WA_OpaquePaintEvent);

  setDisplayMode( 1, true, true, 1 );
}

DisplayWid::~DisplayWid()
{
	delete m_bigDigit;
	delete m_bigSpecialChar;
	delete m_bigDecimal;
	delete m_bigMinus;
	delete m_bigG;
	delete m_bigM;
	delete m_bigk;
	delete m_bigm;
	delete m_bigu;
	delete m_bign;
	delete m_bigp;
	delete m_bigHz;
	delete m_bigF;
	delete m_bigV;
	delete m_bigVA;
	delete m_bigcosphi;
	delete m_bigA;
	delete m_bigH;
	delete m_bigW;
	delete m_bigDBM;
	delete m_bigOhm;
	delete m_bigDeg;
	delete m_bigPercent;
	delete m_smallDigit;
	delete m_smallSpecialChar;
	delete m_smallDecimal;
	delete m_smallMinus;
	delete m_smallG;
	delete m_smallM;
	delete m_smallk;
	delete m_smallm;
	delete m_smallu;
	delete m_smalln;
	delete m_smallp;
	delete m_smallHz;
	delete m_smallF;
	delete m_smallV;
	delete m_smallVA;
	delete m_smallcosphi;
	delete m_smallA;
	delete m_smallH;
	delete m_smallW;
	delete m_smallDBM;
	delete m_smallOhm;
	delete m_smallDeg;
	delete m_smallPercent;
	delete m_minStr;
	delete m_maxStr;
	delete m_diode;
	delete m_ac;
	delete m_dc;
	delete m_bar[0];
	delete m_bar[1];
	delete m_bar[2];
	delete m_bar[3];
	delete m_bar[4];
	delete m_bar[5];
	delete m_bar[6];
}

void DisplayWid::setDisplayMode( int dm, bool minMax, bool bar, int numValues )
{
  m_displayMode = dm;
  m_showMinMax  = minMax;
  m_showBar     = bar;
  m_numValues   = numValues;

  int numDigits = calcNumDigits( m_displayMode );

  m_minMaxW = m_showMinMax ? (numDigits)*18+30+24 : 10;
  int digitsW = (numDigits)*49+30+30;
  m_extraH = m_numValues > 1 ? 24 : 0;
  m_extraW = (numDigits)*18+30+10;
  m_minW = m_extraW * (m_numValues-1);

  setFixedSize( qMax( m_minW, digitsW+m_minMaxW ), 76 + (m_showBar ? 26 : 2)+m_extraH );

  switch (m_displayMode)
  {
  case 0:
	m_range = 2000;
	break;

  case 1:
	m_range = 4000;
	break;

  case 2:
	m_range = 20000;
	break;

  case 3:
	m_range = 50000;
	break;

  case 4:
	m_range = 100000;
	break;

  case 5:
	m_range = 200000;
	break;

  case 6:
	m_range = 400000;
	break;

  case 7:
	m_range = 1000000;
	break;

  case 8:
	m_range = 6000;
	break;

  case 9:
	m_range = 40000;
	break;
  }

  update();
}

void DisplayWid::setShowBar( bool bar )
{
  m_paintBar = bar;
}

void DisplayWid::setValue( int id, const QString & value )
{
  m_value[id] = value;
}

void DisplayWid::setMinValue( const QString & value )
{
  m_minValue = value;
}

void DisplayWid::setMaxValue( const QString & value )
{
  m_maxValue = value;
}

void DisplayWid::setUnit( int id, const QString & value )
{
  m_unit[id] = value;
}

void DisplayWid::setMinUnit( const QString & value )
{
  m_minUnit = value;
}

void DisplayWid::setMaxUnit( const QString & value )
{
  m_maxUnit = value;
}

void DisplayWid::setMode( int id, const QString & value )
{
  m_mode[id] = value;
}

void DisplayWid::paintEvent( QPaintEvent * )
{
  QPixmap pix( width(), height() );
  pix.fill( palette().window().color() );
  QPainter p;

  int numDigits = calcNumDigits( m_displayMode );

  if (!m_value[0].isEmpty())
  {
	p.begin(&pix);

	p.setPen( palette().windowText().style() );

	if (m_showMinMax)
	{
	  p.save();
	  p.drawPixmap( 6, 16, *m_minStr );
	  p.translate( 28, 12 );
	  drawSmallNumber( &p, m_minValue );
	  p.translate( (numDigits)*18+12, 12 );
	  drawSmallUnit( &p, m_minUnit );
	  p.restore();

	  p.save();
	  p.drawPixmap( 6, 12+34, *m_maxStr );
	  p.translate( 28,8+34 );
	  drawSmallNumber( &p, m_maxValue );
	  p.translate( (numDigits)*18+12, 12 );
	  drawSmallUnit( &p, m_maxUnit );
	  p.restore();
	}

	p.save();
	if (m_showMinMax)
	{
	  p.translate( 6+16+(numDigits*18)+30, 8 );
	}
	else
	{
	  p.translate( 6, 8 );
	}

	drawBigNumber( &p, m_value[0] );
	p.translate( (numDigits)*49+24, 42 );
	drawBigUnit( &p, m_unit[0] );

	if (m_mode[0] == "DI")
	{
	  p.drawPixmap( 0, -36, *m_diode );
	}
	else if (m_mode[0] == "AC")
	{
	  p.drawPixmap( 0, -36, *m_ac );
	}
	else if (m_mode[0] == "DC")
	{
	  p.drawPixmap( 0, -36, *m_dc );
	}

	p.restore();

	if (m_showBar)
	{
	  QString val;

	  for (unsigned i=0; i<static_cast<unsigned>(m_value[0].length()); ++i)
	  {
		if (m_value[0][i].digitValue() != -1)
		{
		  val += m_value[0][i];
		}
	  }
	  double percent = val.toDouble() / static_cast<double>(m_range);

	  int step = width()-18;
	  int off = 0;
	  if (0 == m_displayMode || 2 == m_displayMode)
	  {
		step /= 20;
		off = (width()-20*step)/2-2;
	  }
	  else if (1 == m_displayMode || 3 == m_displayMode || 9 == m_displayMode)
	  {
		step /= 40;
		off = (width()-40*step)/2-2;
	  }
	  else if (8 == m_displayMode)
	  {
		step /= 60;
		off = (width()-60*step)/2-2;
	  }
	  else
	  {
		step /= 50;
		off = (width()-50*step)/2-2;
	  }

	  for(int i=off, c=0, n=0; i<=width()-off; i+=step, ++c)
	  {
		if (!(c%5))
		{
		  p.drawLine( i, height()-16, i, height()-12 );

		  if (!(c%10))
		  {
			p.drawPixmap( i-4, height()-16-9, *m_bar[n++] );
		  }
		}
		else
		{
		  p.drawLine( i, height()-14, i, height()-12 );
		}
	  }

	  if (m_paintBar)
	  {
		p.fillRect( off, height()-10,
					static_cast<int>(qRound(static_cast<double>((width()-2*off))*percent)), 5,
					palette().windowText().color() );
	  }
	}

	p.translate( 6, 76 );
	for (int i=1; i<m_numValues; ++i)
	{
	  drawSmallNumber( &p, m_value[i] );
	  p.save();
	  p.translate( (numDigits)*18+12, 12 );
	  drawSmallUnit( &p, m_unit[i] );
	  p.restore();
	  p.translate( m_extraW, 0 );
	}

	p.end();
  }

  p.begin(this);
  p.drawPixmap(0,0,pix);
  p.end();
}

void DisplayWid::drawSmallNumber( QPainter *p, const QString & num )
{
  int x = 0;
  int offset = 0;
  if(!num.isEmpty())
  {
	  while (num[offset] == ' ' && offset<num.length()-1)
		++offset;
	  if (num[offset] == '-')
	  {
		p->drawPixmap( 0, 9, *m_smallMinus );
		offset++;
	  }
  }
  x += 12;

  for (unsigned i=offset; i<static_cast<unsigned>(num.length()); i++)
  {
	if (num[i] == '.')
	{
	  p->drawPixmap( x-5, 0, *m_smallDecimal );
	}
	else if (num[i].toLower() == 'a')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*0, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'b')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*1, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'd')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*2, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'e')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*3, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'h')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*4, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'l')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*5, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'n')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*6, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'o')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*7, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'p')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*8, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'r')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*9, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 's')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*10, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 't')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*11, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].toLower() == 'y')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*12, 0, 12, 21 );
	  x += 18;
	}
	else
	{
	  int digit = num[i].toLatin1()-'0';
	  if (digit >= 0 && digit <= 9)
	  {
		p->drawPixmap( x, 0, *m_smallDigit, 12*digit, 0, 12, 21 );
	  }
	  x += 18;
	}
  }
}

void DisplayWid::drawBigUnit( QPainter *p, const QString & str )
{
  int index = 0;
  int x = 0;

  if(!str.isEmpty())
  {
	  if (str[0] == 'G')
	  {
		p->drawPixmap( x, 0, *m_bigG );
		x += m_bigG->width()+2;
		index++;
	  }
	  else if (str[0] == 'M')
	  {
		p->drawPixmap( x, 0, *m_bigM );
		x += m_bigM->width()+2;
		index++;
	  }
	  else if (str[0] == 'k')
	  {
		p->drawPixmap( x, 0, *m_bigk );
		x += m_bigk->width()+2;
		index++;
	  }
	  else if (str[0] == 'm')
	  {
		p->drawPixmap( x, 0, *m_bigm );
		x += m_bigm->width()+2;
		index++;
	  }
	  else if (str[0] == 'u')
	  {
		p->drawPixmap( x, 3, *m_bigu );
		x += m_bigu->width()+2;
		index++;
	  }
	  else if (str[0] == 'n')
	  {
		p->drawPixmap( x, 0, *m_bign );
		x += m_bign->width()+2;
		index++;
	  }

	  if (str.mid(index) == "Ohm")
		p->drawPixmap( x, 0, *m_bigOhm );
	  else if (str.mid(index) == "C")
		p->drawPixmap( x, 0, *m_bigDeg );
	  else if (str.mid(index) == "Hz")
		p->drawPixmap( x, 0, *m_bigHz );
	  else if (str.mid(index) == "F") // ignore Farenheit
		p->drawPixmap( x, 0, *m_bigF );
	  else if (str.mid(index) == "H")
		p->drawPixmap( x, 0, *m_bigH );
	  else if (str.mid(index) == "W")
		p->drawPixmap( x, 0, *m_bigW );
	  else if (str.mid(index) == "dBm")
		p->drawPixmap( x, 0, *m_bigDBM );
	  else if (str.mid(index) == "A")
		p->drawPixmap( x, 0, *m_bigA );
	  else if (str.mid(index) == "V")
		p->drawPixmap( x, 0, *m_bigV );
	  else if (str.mid(index) == "VA")
		p->drawPixmap( x, 0, *m_bigVA );
	  else if (str.mid(index) == "cosphi")
		p->drawPixmap( x, 0, *m_bigcosphi );
	  else if (str.mid(index) == "%")
		p->drawPixmap( x, 0, *m_bigPercent );
  }
}

void DisplayWid::drawSmallUnit( QPainter *p, const QString & str )
{
  int index = 0;
  int x = 0;

  if(!str.isEmpty())
  {
	  if (str[0] == 'G')
	  {
		p->drawPixmap( x, 0, *m_smallG );
		x += m_smallG->width()+1;
		index++;
	  }
	  else if (str[0] == 'M')
	  {
		p->drawPixmap( x, 0, *m_smallM );
		x += m_smallM->width()+1;
		index++;
	  }
	  else if (str[0] == 'k')
	  {
		p->drawPixmap( x, 0, *m_smallk );
		x += m_smallk->width()+1;
		index++;
	  }
	  else if (str[0] == 'm')
	  {
		p->drawPixmap( x, 0, *m_smallm );
		x += m_smallm->width()+1;
		index++;
	  }
	  else if (str[0] == 'u')
	  {
		p->drawPixmap( x, 3, *m_smallu );
		x += m_smallu->width()+1;
		index++;
	  }
	  else if (str[0] == 'n')
	  {
		p->drawPixmap( x, 0, *m_smalln );
		x += m_smalln->width()+1;
		index++;
	  }
	  if (str.mid(index) == "Ohm")
		p->drawPixmap( x, 0, *m_smallOhm );
	  else if (str.mid(index) == "C")
		p->drawPixmap( x, 0, *m_smallDeg );
	  else if (str.mid(index) == "Hz")
		p->drawPixmap( x, 0, *m_smallHz );
	  else if (str.mid(index) == "F")
		p->drawPixmap( x, 0, *m_smallF );
	  else if (str.mid(index) == "H")
		p->drawPixmap( x, 0, *m_smallH );
	  else if (str.mid(index) == "W")
		p->drawPixmap( x, 0, *m_smallW );
	  else if (str.mid(index) == "dBm")
		p->drawPixmap( x, 0, *m_smallDBM );
	  else if (str.mid(index) == "A")
		p->drawPixmap( x, 0, *m_smallA );
	  else if (str.mid(index) == "V")
		p->drawPixmap( x, 0, *m_smallV );
	  else if (str.mid(index) == "VA")
		p->drawPixmap( x, 0, *m_smallVA );
	  else if (str.mid(index) == "cosphi")
		p->drawPixmap( x, 0, *m_smallcosphi );
	  else if (str.mid(index) == "%")
		p->drawPixmap( x, 0, *m_smallPercent );
  }
}

void DisplayWid::drawBigNumber( QPainter *p, const QString & num )
{
  int x = 0;
  int offset = 0;
  if(!num.isEmpty())
  {
	  while (num[offset] == ' ' && offset<num.length()-1)
		  ++offset;

	  if (num[offset] == '-')
	  {
		p->drawPixmap( x, 0, *m_bigMinus );
		offset++;
	  }
  }
  x += 28;

  for (unsigned i=offset; i<static_cast<unsigned>(num.length()); i++)
  {
	if (num[i] == '.')
		p->drawPixmap( x-11, 0, *m_bigDecimal );
	else if (num[i].toLower() == 'a')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*0, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'b')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*1, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'd')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*2, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'e')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*3, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'h')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*4, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'l')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*5, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'n')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*6, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'o')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*7, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'p')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*8, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'r')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*9, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 's')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*10, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 't')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*11, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].toLower() == 'y')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*12, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i] == ' ')
		x += 49;
	else
	{
	  int digit = num[i].toLatin1()-'0';

	  if (digit >= 0 && digit <= 9)
		p->drawPixmap( x+2, 2, *m_bigDigit, 34*digit, 0, 34, 60 );
	  x += 49;
	}
  }
}

int DisplayWid::calcNumDigits( int dm )
{
  int numDigits = 0;

  switch (dm)
  {
	default:
	case 0:
	case 1:
	case 8:
	  numDigits = 4;
	  break;
	case 2:
	case 3:
	case 4:
	case 9:
	  numDigits = 5;
	  break;
	case 5:
	case 6:
	case 7:
	  numDigits = 6;
	  break;
  }

  return numDigits;
}

QBitmap* DisplayWid::BitmapHelper(const QString &file) const
{
		return new QBitmap(QPixmap(file).createMaskFromColor(Qt::black,Qt::MaskOutColor));
}
