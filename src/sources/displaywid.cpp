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

// less than elegant use of single pixmaps for everything.
// But hey, it works!
//
#include <numbers.xpm>
#include <decimal.xpm>
#include <specialchars.xpm>
#include <minus.xpm>
#include <G.xpm>
#include <cm.xpm>
#include <k.xpm>
#include <m.xpm>
#include <u.xpm>
#include <n.xpm>
#include <p.xpm>
#include <Hz.xpm>
#include <F.xpm>
#include <V.xpm>
#include <A.xpm>
#include <H.xpm>
#include <W.xpm>
#include <DBM.xpm>
#include <Ohm.xpm>
#include <deg.xpm>
#include <percent.xpm>

#include <numbers_small.xpm>
#include <decimal_small.xpm>
#include <specialchars_small.xpm>
#include <minus_small.xpm>
#include <G_small.xpm>
#include <cm_small.xpm>
#include <k_small.xpm>
#include <m_small.xpm>
#include <u_small.xpm>
#include <n_small.xpm>
#include <p_small.xpm>
#include <Hz_small.xpm>
#include <F_small.xpm>
#include <V_small.xpm>
#include <A_small.xpm>
#include <H_small.xpm>
#include <W_small.xpm>
#include <DBM_small.xpm>
#include <Ohm_small.xpm>
#include <deg_small.xpm>
#include <percent_small.xpm>

#include <min_str.xpm>
#include <max_str.xpm>

#include <diode.xpm>
#include <ac.xpm>
#include <dc.xpm>

#include <null_bar.xpm>
#include <ten_bar.xpm>
#include <twenty_bar.xpm>
#include <thirty_bar.xpm>
#include <fourty_bar.xpm>
#include <fifty_bar.xpm>
#include <sixty_bar.xpm>



DisplayWid::DisplayWid( QWidget *parent, const char *name ) : QWidget( parent, name ),
  m_paintBar( true ),
  m_numValues( 1 )
{
  // HACK: My GIMP doesn't save XBM !!!!
  //
  m_bigDigit = createBitmap((const char **)numbers_xpm);
  m_bigSpecialChar = createBitmap((const char **)specialchars_xpm);

  m_bigDecimal = createBitmap((const char **)decimal_xpm);
  m_bigMinus = createBitmap((const char **)minus_xpm);
  m_bigG = createBitmap((const char **)G_xpm);
  m_bigM = createBitmap((const char **)M_xpm);
  m_bigk = createBitmap((const char **)k_xpm);
  m_bigm = createBitmap((const char **)m_xpm);
  m_bigu = createBitmap((const char **)u_xpm);
  m_bign = createBitmap((const char **)n_xpm);
  m_bigp = createBitmap((const char **)p_xpm);
  m_bigHz = createBitmap((const char **)Hz_xpm);
  m_bigF = createBitmap((const char **)F_xpm);
  m_bigV = createBitmap((const char **)V_xpm);
  m_bigA = createBitmap((const char **)A_xpm);
  m_bigH = createBitmap((const char **)H_xpm);
  m_bigW = createBitmap((const char **)W_xpm);
  m_bigDBM = createBitmap((const char **)DBM_xpm);
  m_bigOhm = createBitmap((const char **)Ohm_xpm);
  m_bigDeg = createBitmap((const char **)deg_xpm);
  m_bigPercent = createBitmap((const char **)percent_xpm);

  m_smallDigit = createBitmap((const char **)numbers_small_xpm);
  m_smallSpecialChar = createBitmap((const char **)specialchars_small_xpm);

  m_smallDecimal = createBitmap((const char **)decimal_small_xpm);
  m_smallMinus = createBitmap((const char **)minus_small_xpm);
  m_smallG = createBitmap((const char **)G_small_xpm);
  m_smallM = createBitmap((const char **)M_small_xpm);
  m_smallk = createBitmap((const char **)k_small_xpm);
  m_smallm = createBitmap((const char **)m_small_xpm);
  m_smallu = createBitmap((const char **)u_small_xpm);
  m_smalln = createBitmap((const char **)n_small_xpm);
  m_smallp = createBitmap((const char **)p_small_xpm);
  m_smallHz = createBitmap((const char **)Hz_small_xpm);
  m_smallF = createBitmap((const char **)F_small_xpm);
  m_smallV = createBitmap((const char **)V_small_xpm);
  m_smallA = createBitmap((const char **)A_small_xpm);
  m_smallH = createBitmap((const char **)H_small_xpm);
  m_smallW = createBitmap((const char **)W_small_xpm);
  m_smallDBM = createBitmap((const char **)DBM_small_xpm);
  m_smallOhm = createBitmap((const char **)Ohm_small_xpm);
  m_smallDeg = createBitmap((const char **)deg_small_xpm);
  m_smallPercent = createBitmap((const char **)percent_small_xpm);

  m_minStr = createBitmap((const char **)min_str_xpm);
  m_maxStr = createBitmap((const char **)max_str_xpm);

  m_diode = createBitmap((const char **)diode_xpm);
  m_ac = createBitmap((const char **)ac_xpm);
  m_dc = createBitmap((const char **)dc_xpm);

  m_bar[0] = createBitmap((const char **)null_bar_xpm);
  m_bar[1] = createBitmap((const char **)ten_bar_xpm);
  m_bar[2] = createBitmap((const char **)twenty_bar_xpm);
  m_bar[3] = createBitmap((const char **)thirty_bar_xpm);
  m_bar[4] = createBitmap((const char **)fourty_bar_xpm);
  m_bar[5] = createBitmap((const char **)fifty_bar_xpm);
  m_bar[6] = createBitmap((const char **)sixty_bar_xpm);

  setBackgroundMode( Qt::NoBackground );

  setDisplayMode( 1, true, true, 1 );
}

QBitmap *DisplayWid::createBitmap( const char **data )
{
  QBitmap *bm = new QBitmap(QPixmap(data).createMaskFromColor(QColor(0,0,0),Qt::MaskOutColor));
  //QBitmap *bm = new QBitmap;
  //*bm = QImage(data).convertDepth( 1, Qt::ThresholdDither );
  //bm->setMask( *bm );

  return bm;
}

DisplayWid::~DisplayWid()
{
  delete m_bigDigit;
  delete m_smallDigit;
  delete m_bigDecimal;
  delete m_smallDecimal;
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

  setFixedSize( QMAX( m_minW, digitsW+m_minMaxW ), 76 + (m_showBar ? 26 : 2)+m_extraH );

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
  pix.fill( colorGroup().background() );
  QPainter p;

  int numDigits = calcNumDigits( m_displayMode );

  if (!m_value[0].isEmpty())
  {
	p.begin(&pix);

	p.setPen( colorGroup().foreground() );

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

	  for (unsigned i=0; i<m_value[0].length(); ++i)
	  {
		if (m_value[0][i].digitValue() != -1)
		{
		  val += m_value[0][i];
		}
	  }
	  double percent = val.toDouble() / (double)m_range;

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
					(int)qRound((double)(width()-2*off)*percent ), 5,
					colorGroup().foreground() );
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

  while (num[offset] == ' ' && offset<num.length()) ++offset;

  if (num[offset] == '-')
  {
	p->drawPixmap( 0, 9, *m_smallMinus );
	offset++;
  }

  x += 12;

  for (unsigned i=offset; i<num.length(); i++)
  {
	if (num[i] == '.')
	{
	  p->drawPixmap( x-5, 0, *m_smallDecimal );
	}
	else if (num[i].lower() == 'a')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*0, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'b')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*1, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'd')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*2, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'e')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*3, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'h')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*4, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'l')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*5, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'n')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*6, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'o')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*7, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'p')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*8, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'r')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*9, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 's')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*10, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 't')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*11, 0, 12, 21 );
	  x += 18;
	}
	else if (num[i].lower() == 'y')
	{
	  p->drawPixmap( x, 0, *m_smallSpecialChar, 12*12, 0, 12, 21 );
	  x += 18;
	}
	else
	{
	  int digit = num[i].latin1()-'0';

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
  {
	p->drawPixmap( x, 0, *m_bigOhm );
  }
  else if (str.mid(index) == "C")
  {
	p->drawPixmap( x, 0, *m_bigDeg );
  }
  else if (str.mid(index) == "Hz")
  {
	p->drawPixmap( x, 0, *m_bigHz );
  }
  else if (str.mid(index) == "F") // ignore Farenheit
  {
	p->drawPixmap( x, 0, *m_bigF );
  }
  else if (str.mid(index) == "H")
  {
	p->drawPixmap( x, 0, *m_bigH );
  }
  else if (str.mid(index) == "W")
  {
	p->drawPixmap( x, 0, *m_bigW );
  }
  else if (str.mid(index) == "dBm")
  {
	p->drawPixmap( x, 0, *m_bigDBM );
  }
  else if (str.mid(index) == "A")
  {
	p->drawPixmap( x, 0, *m_bigA );
  }
  else if (str.mid(index) == "V")
  {
	p->drawPixmap( x, 0, *m_bigV );
  }
  else if (str.mid(index) == "%")
  {
	p->drawPixmap( x, 0, *m_bigPercent );
  }
}

void DisplayWid::drawSmallUnit( QPainter *p, const QString & str )
{
  int index = 0;
  int x = 0;

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
  {
	p->drawPixmap( x, 0, *m_smallOhm );
  }
  else if (str.mid(index) == "C")
  {
	p->drawPixmap( x, 0, *m_smallDeg );
  }
  else if (str.mid(index) == "Hz")
  {
	p->drawPixmap( x, 0, *m_smallHz );
  }
  else if (str.mid(index) == "F")
  {
	p->drawPixmap( x, 0, *m_smallF );
  }
  else if (str.mid(index) == "H")
  {
	p->drawPixmap( x, 0, *m_smallH );
  }
  else if (str.mid(index) == "W")
  {
	p->drawPixmap( x, 0, *m_smallW );
  }
  else if (str.mid(index) == "dBm")
  {
	p->drawPixmap( x, 0, *m_smallDBM );
  }
  else if (str.mid(index) == "A")
  {
	p->drawPixmap( x, 0, *m_smallA );
  }
  else if (str.mid(index) == "V")
  {
	p->drawPixmap( x, 0, *m_smallV );
  }
  else if (str.mid(index) == "%")
  {
	p->drawPixmap( x, 0, *m_smallPercent );
  }
}

void DisplayWid::drawBigNumber( QPainter *p, const QString & num )
{
  int x = 0;
  int offset = 0;
  while (num[offset] == ' ' && offset<num.length()) ++offset;
  bool comma = false;

  if (num[offset] == '-')
  {
	p->drawPixmap( x, 0, *m_bigMinus );
	offset++;
  }

  x += 28;

  for (unsigned i=offset; i<num.length(); i++)
  {
	if (num[i] == '.')
	{
	  p->drawPixmap( x-11, 0, *m_bigDecimal );
	  comma = true;
	}
	else if (num[i].lower() == 'a')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*0, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'b')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*1, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'd')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*2, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'e')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*3, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'h')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*4, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'l')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*5, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'n')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*6, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'o')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*7, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'p')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*8, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'r')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*9, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 's')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*10, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 't')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*11, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i].lower() == 'y')
	{
	  p->drawPixmap( x+2, 2, *m_bigSpecialChar, 34*12, 0, 34, 60 );
	  x += 49;
	}
	else if (num[i] == ' ')
	{
	  x += 49;
	}
	else
	{
	  int digit = num[i].latin1()-'0';

	  if (digit >= 0 && digit <= 9)
	  {
		p->drawPixmap( x+2, 2, *m_bigDigit, 34*digit, 0, 34, 60 );
	  }
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
