//======================================================================
// File:		displaywid.h
// Author:	Matthias Toussaint
// Created:	Fri Nov 23 22:28:36 CET 2001
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

#ifndef DISPLAYWID_HH
#define DISPLAYWID_HH

#include <QtGui>
#include <QtWidgets>


class DisplayWid : public QWidget
{
  Q_OBJECT
	public:
	  DisplayWid( QWidget *parent=Q_NULLPTR);
	  virtual	~DisplayWid();

	  void		setValue( int, const QString & );
	  void		setUnit( int, const QString & );
	  void		setMinValue( const QString & );
	  void		setMaxValue( const QString & );
	  void		setMinUnit( const QString & );
	  void		setMaxUnit( const QString & );
	  void		setMode( int, const QString & );
	  void		setDisplayMode( int, bool minMax, bool bar, int numValues );
	  void		setShowBar( bool );

	protected:
	  QBitmap	*m_bigDigit;
	  QBitmap	*m_bigSpecialChar;
	  QBitmap	*m_bigDecimal;
	  QBitmap	*m_bigMinus;
	  QBitmap	*m_bigL;
	  QBitmap	*m_bigG;
	  QBitmap	*m_bigM;
	  QBitmap	*m_bigk;
	  QBitmap	*m_bigm;
	  QBitmap	*m_bigu;
	  QBitmap	*m_bign;
	  QBitmap	*m_bigp;
	  QBitmap	*m_bigHz;
	  QBitmap	*m_bigF;
	  QBitmap	*m_bigV;
	  QBitmap	*m_bigA;
	  QBitmap	*m_bigH;
	  QBitmap	*m_bigW;
	  QBitmap	*m_bigDBM;
	  QBitmap	*m_bigOhm;
	  QBitmap	*m_bigDeg;
	  QBitmap	*m_bigPercent;
	  QBitmap	*m_smallDigit;
	  QBitmap	*m_smallSpecialChar;
	  QPixmap	*m_smallDecimal;
	  QPixmap	*m_smallMinus;
	  QPixmap	*m_smallL;
	  QPixmap	*m_smallG;
	  QPixmap	*m_smallM;
	  QPixmap	*m_smallk;
	  QPixmap	*m_smallm;
	  QPixmap	*m_smallu;
	  QPixmap	*m_smalln;
	  QPixmap	*m_smallp;
	  QPixmap	*m_smallHz;
	  QPixmap	*m_smallF;
	  QPixmap	*m_smallV;
	  QPixmap	*m_smallA;
	  QPixmap	*m_smallH;
	  QPixmap	*m_smallW;
	  QPixmap	*m_smallDBM;
	  QPixmap	*m_smallOhm;
	  QPixmap	*m_smallDeg;
	  QPixmap	*m_smallPercent;
	  QBitmap	*m_minStr;
	  QBitmap	*m_maxStr;
	  QBitmap	*m_diode;
	  QBitmap	*m_ac;
	  QBitmap	*m_dc;
	  QBitmap	*m_bar[7];
	  QString	m_value[4];
	  QString	m_minValue;
	  QString	m_maxValue;
	  QString	m_unit[4];
	  QString	m_minUnit;
	  QString	m_maxUnit;
	  QString	m_mode[4];
	  int		m_displayMode;
	  int		m_range;
	  bool		m_showMinMax;
	  bool		m_showBar;
	  bool		m_paintBar;
	  int		m_numValues;
	  int		m_minMaxW;
	  int		m_extraH;
	  int		m_minW;
	  int		m_extraW;

	  void		paintEvent( QPaintEvent *) Q_DECL_OVERRIDE;
	  void		drawSmallNumber( QPainter *, const QString & str );
	  void		drawSmallUnit( QPainter *, const QString & str );
	  void		drawBigNumber( QPainter *, const QString & str );
	  void		drawBigUnit( QPainter *, const QString & str );

	private:
	  int		calcNumDigits( int );
	  QBitmap*	BitmapHelper(const QString &file)const;

};

#endif // DISPLAYWID_HH
