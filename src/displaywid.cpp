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

DisplayWid::DisplayWid(QWidget *parent) : QWidget(parent),
  m_showAuto(false),
  m_showManu(false),
  m_paintBar(true),
  m_numValues(1)
{

  m_bigDigit = BitmapHelper(":/Symbols/numbers.xpm");
  m_bigSpecialChar = BitmapHelper(":/Symbols/specialchars.xpm");

  m_bigDecimal =  BitmapHelper(":/Symbols/decimal.xpm");
  m_bigMinus =  BitmapHelper(":/Symbols/minus.xpm");
  m_bigG = BitmapHelper(":/Symbols/G.xpm");
  m_bigM =  BitmapHelper(":/Symbols/cm.xpm");
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
  m_bigDegF =  BitmapHelper(":/Symbols/degf.xpm");
  m_bigPercent =  BitmapHelper(":/Symbols/percent.xpm");

  m_smallDigit =  BitmapHelper(":/Symbols/numbers_small.xpm");
  m_smallSpecialChar =  BitmapHelper(":/Symbols/specialchars_small.xpm");

  m_smallDecimal =  BitmapHelper(":/Symbols/decimal_small.xpm");
  m_smallMinus =  BitmapHelper(":/Symbols/minus_small.xpm");
  m_smallG =  BitmapHelper(":/Symbols/G_small.xpm");
  m_smallM =  BitmapHelper(":/Symbols/cm_small.xpm");
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
  m_smallDegF =  BitmapHelper(":/Symbols/degf_small.xpm");
  m_smallPercent =  BitmapHelper(":/Symbols/percent_small.xpm");

  m_minStr =  BitmapHelper(":/Symbols/min_str.xpm");
  m_maxStr =  BitmapHelper(":/Symbols/max_str.xpm");

  m_diode =  BitmapHelper(":/Symbols/diode.xpm");
  m_buzzer =  BitmapHelper(":/Symbols/buzzer.xpm");
  m_ac =  BitmapHelper(":/Symbols/ac.xpm");
  m_dc =  BitmapHelper(":/Symbols/dc.xpm");

  m_hold =  BitmapHelper(":/Symbols/hold.xpm");
  m_auto =  BitmapHelper(":/Symbols/auto.xpm");
  m_manu =  BitmapHelper(":/Symbols/manu.xpm");

  m_bar[0] =  BitmapHelper(":/Symbols/null_bar.xpm");
  m_bar[1] =  BitmapHelper(":/Symbols/ten_bar.xpm");
  m_bar[2] =  BitmapHelper(":/Symbols/twenty_bar.xpm");
  m_bar[3] =  BitmapHelper(":/Symbols/thirty_bar.xpm");
  m_bar[4] =  BitmapHelper(":/Symbols/fourty_bar.xpm");
  m_bar[5] =  BitmapHelper(":/Symbols/fifty_bar.xpm");
  m_bar[6] =  BitmapHelper(":/Symbols/sixty_bar.xpm");

  setAttribute(Qt::WA_OpaquePaintEvent);
  setAutoFillBackground(false);

  setDisplayMode(1, true, true, 1);
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
  delete m_bigDegF;
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
  delete m_smallDegF;
  delete m_smallPercent;
  delete m_minStr;
  delete m_maxStr;
  delete m_diode;
  delete m_buzzer;
  delete m_ac;
  delete m_dc;
  delete m_bar[0];
  delete m_bar[1];
  delete m_bar[2];
  delete m_bar[3];
  delete m_bar[4];
  delete m_bar[5];
  delete m_bar[6];
  delete m_hold;
  delete m_auto;
  delete m_manu;
}

void DisplayWid::setHold(bool enabled)
{
  m_showHold = enabled;
}

void DisplayWid::setAuto(bool enabled)
{
  m_showAuto = enabled;
  m_showManu = !enabled;
}

void DisplayWid::setManu(bool enabled)
{
  m_showManu = enabled;
  m_showAuto = !enabled;
}

void DisplayWid::setDisplayMode(int dm, bool minMax, bool bar, int numValues)
{
  m_range = dm;
  m_showMinMax  = minMax;
  m_showBar     = bar;
  m_numValues   = numValues;

  int numDigits = calcNumDigits(m_range);
  m_minMaxW = m_showMinMax ? (numDigits) * 18 + 30 + 24 : 10;
  int digitsW = (numDigits) * 49 + 30 + 30;
  m_extraH = m_numValues > 1 ? 24 : 0;
  m_extraW = (numDigits) * 18 + 30 + 10;
  m_minW = m_extraW * (m_numValues - 1);

  setFixedSize(qMax(m_minW, digitsW + m_minMaxW), 76 + (m_showBar ? 26 : 2) + m_extraH);
  update();
}

void DisplayWid::setShowBar(bool bar)
{
  m_paintBar = bar;
}

void DisplayWid::setValue(int id, const QString &value)
{
  m_value[id] = value;
}

void DisplayWid::setMinValue(const QString &value)
{
  m_minValue = value;
}

void DisplayWid::setMaxValue(const QString &value)
{
  m_maxValue = value;
}

void DisplayWid::setUnit(int id, const QString &value)
{
  m_unit[id] = value;
}

void DisplayWid::setMinUnit(const QString &value)
{
  m_minUnit = value;
}

void DisplayWid::setMaxUnit(const QString &value)
{
  m_maxUnit = value;
}

void DisplayWid::setMode(int id, const QString &value)
{
  m_mode[id] = value;
}

void DisplayWid::paintEvent(QPaintEvent *)
{
  QRect drawRect = contentsRect();

  QPixmap pix(drawRect.size());
  pix.fill(palette().window().color());
  QPainter p;

  int numDigits = calcNumDigits(m_range);

  if (!m_value[0].isEmpty())
  {
    p.begin(&pix);

    p.setPen(palette().windowText().style());

    if (m_showMinMax)
    {
      p.save();
      p.drawPixmap(6, 16, *m_minStr);
      p.translate(28, 12);
      drawSmallNumber(&p, m_minValue);
      p.translate((numDigits) * 18 + 12, 12);
      drawSmallUnit(&p, m_minUnit);
      p.restore();

      p.save();
      p.drawPixmap(6, 12 + 34, *m_maxStr);
      p.translate(28, 8 + 34);
      drawSmallNumber(&p, m_maxValue);
      p.translate((numDigits) * 18 + 12, 12);
      drawSmallUnit(&p, m_maxUnit);
      p.restore();
    }

    p.save();
    p.translate(6 + (m_showMinMax ? (numDigits * 18) + 46 : 0), 8);

    drawBigNumber(&p, m_value[0]);
    p.translate((numDigits) * 49 + 24, 42);
    drawBigUnit(&p, m_unit[0]);

    if (m_mode[0] == "DI")
      p.drawPixmap(0, -36, *m_diode);
    else if (m_mode[0] == "BUZ")
      p.drawPixmap(0, -36, *m_buzzer);
    else if (m_mode[0] == "AC")
      p.drawPixmap(0, -36, *m_ac);
    else if (m_mode[0] == "DC")
      p.drawPixmap(0, -36, *m_dc);
    else if (m_mode[0] == "ACDC")
    {
      p.drawPixmap(0, -36, *m_ac);
      p.drawPixmap(16, -36, *m_dc);
    }

    p.restore();

    if (m_showBar)
    {
      QString val;

      for (unsigned i = 0; i < static_cast<unsigned>(m_value[0].length()); ++i)
      {
        if (m_value[0][i].digitValue() != -1)
          val += m_value[0][i];
      }
      double percent = val.toDouble() / static_cast<double>(m_range);
      if (percent > 1.0)
        percent = 1.0;

      int divisions = static_cast<double>(m_range) / static_cast<double>(std::pow(10, numDigits-2));
      if (divisions<20 || divisions>60)
        divisions=50;

      int off = 25;

      int step = (width() - off * 2) / (divisions);

      for (int c = 0, n = 0; c <= divisions; ++c)
      {
        int x = off + c * step + 5;
        if (!(c % 5))
        {
          p.drawLine(x, height() - 16, x, height() - 12);

          if (!(c % 10))
            p.drawPixmap(x - 4, height() - 16 - 9, *m_bar[n++]);
        }
        else
          p.drawLine(x, height() - 14, x, height() - 12);
      }

      if (m_paintBar)
      {
        int width = static_cast<int>(qRound(static_cast<double>((step * divisions)) * percent));
        p.fillRect(off + 5, height() - 10, width, 5, palette().windowText().color());
      }
    }

    if (m_showHold)
      p.drawPixmap(6, height() - 25, *m_hold);

    if (m_showAuto)
      p.drawPixmap(width() - 23, height() - 25, *m_auto);
    else if (m_showManu)
      p.drawPixmap(width() - 23, height() - 25, *m_manu);

    p.translate(6, 76);
    for (int i = 1; i < m_numValues; ++i)
    {
      drawSmallNumber(&p, m_value[i]);
      p.save();
      p.translate((numDigits) * 18 + 12, 12);
      drawSmallUnit(&p, m_unit[i]);
      p.restore();
      p.translate(m_extraW, 0);
    }

    p.end();
  }

  p.begin(this);
  p.drawPixmap(drawRect.topLeft(), pix);
  p.end();
}

void DisplayWid::drawSmallNumber(QPainter *p, const QString &num)
{
  if (num.isEmpty())
    return;

  int x = 0;
  int offset = 0;

  // Skip leading spaces
  while (offset < num.length() - 1 && num[offset] == ' ')
    ++offset;

  // Draw minus sign if present
  if (num[offset] == '-')
  {
    p->drawPixmap(0, 9, *m_smallMinus);
    ++offset;
  }

  x += 12;

  // Mapping of lowercase letters to their position in m_smallSpecialChar (0-based index)
  static const QMap<QChar, int> specialCharMap =
  {
    { 'a', 0 },
    { 'b', 1 },
    { 'd', 2 },
    { 'e', 3 },
    { 'h', 4 },
    { 'l', 5 },
    { 'n', 6 },
    { 'o', 7 },
    { 'p', 8 },
    { 'r', 9 },
    { 's', 10 },
    { 't', 11 },
    { 'y', 12 }
  };

  // Draw each character from the offset to the end
  for (int i = offset; i < num.length(); ++i)
  {
    QChar ch = num[i];

    if (ch == '.')
    {
      // Draw decimal point slightly offset
      p->drawPixmap(x - 5, 0, *m_smallDecimal);
    }
    else if (specialCharMap.contains(ch.toLower()))
    {
      // Draw special letters from m_smallSpecialChar strip
      int index = specialCharMap.value(ch.toLower());
      p->drawPixmap(x, 0, *m_smallSpecialChar, 12 * index, 0, 12, 21);
      x += 18;
    }
    else if (ch.isDigit())
    {
      // Draw digits from m_smallDigit strip
      int digit = ch.digitValue();
      p->drawPixmap(x, 0, *m_smallDigit, 12 * digit, 0, 12, 21);
      x += 18;
    }
    else
    {
      // Unknown character, just advance x to keep spacing
      x += 18;
    }
  }
}


void DisplayWid::drawBigUnit(QPainter *p, const QString &str)
{
  if (str.isEmpty())
    return;

  int x = 0;
  int index = 0;

  // Map of SI prefixes to corresponding big-sized pixmaps
  static const QMap<QChar, const QPixmap *> prefixMap =
  {
    { 'G', m_bigG },
    { 'M', m_bigM },
    { 'k', m_bigk },
    { 'm', m_bigm },
    { 'u', m_bigu },
    { 'n', m_bign }
  };

  QChar firstChar = str.at(0);
  if (prefixMap.contains(firstChar))
  {
    const QPixmap *prefixPixmap = prefixMap.value(firstChar);
    int y = (firstChar == 'u') ? 3 : 0; // offset µ prefix vertically
    p->drawPixmap(x, y, *prefixPixmap);
    x += prefixPixmap->width() + 2;
    ++index;
        qInfo() << "x: " << QString(firstChar);

  }

  // Unit suffix after prefix
  const QString unitSuffix = str.mid(index);

  // Map of unit suffixes to corresponding big-sized pixmaps
  static const QMap<QString, const QPixmap *> unitMap =
  {
    //{ "s",    m_bigS },
    { "Ohm",    m_bigOhm },
    { "C",      m_bigDeg },
    { "dF",     m_bigDegF },
    { "Hz",     m_bigHz },
    { "F",      m_bigF },      // not Fahrenheit
    { "H",      m_bigH },
    { "W",      m_bigW },
    { "dBm",    m_bigDBM },
    { "A",      m_bigA },
    { "V",      m_bigV },
    { "VA",     m_bigVA },
    { "cosphi", m_bigcosphi },
    { "%",      m_bigPercent }
  };

  if (const QPixmap *unitPixmap = unitMap.value(unitSuffix, nullptr))
    p->drawPixmap(x, 0, *unitPixmap);
}

void DisplayWid::drawSmallUnit(QPainter *p, const QString &str)
{
  if (str.isEmpty())
    return;

  int x = 0;
  int index = 0;

  static const QMap<QChar, const QPixmap *> prefixMap =
  {
    { 'G', m_smallG }, { 'M', m_smallM }, { 'k', m_smallk },
    { 'm', m_smallm }, { 'u', m_smallu }, { 'n', m_smalln }
  };

  QChar first = str.at(0);
  if (prefixMap.contains(first))
  {
    const QPixmap *pix = prefixMap.value(first);
    int y = (first == 'u') ? 3 : 0;  // Ausnahme für µ
    p->drawPixmap(x, y, *pix);
    x += pix->width() + 1;
    ++index;
  }

  const QString unitSuffix = str.mid(index);

  static const QMap<QString, const QPixmap *> unitMap =
  {
    //{ "s",       m_smallS },
    { "Ohm",     m_smallOhm },
    { "C",       m_smallDeg },
    { "dF",      m_smallDegF },
    { "Hz",      m_smallHz },
    { "F",       m_smallF },
    { "H",       m_smallH },
    { "W",       m_smallW },
    { "dBm",     m_smallDBM },
    { "A",       m_smallA },
    { "V",       m_smallV },
    { "VA",      m_smallVA },
    { "cosphi",  m_smallcosphi },
    { "%",       m_smallPercent }
  };

  if (unitMap.contains(unitSuffix))
    p->drawPixmap(x, 0, *unitMap.value(unitSuffix));
}

void DisplayWid::drawBigNumber(QPainter *p, const QString &num)
{
  if (num.isEmpty())
    return;

  int x = 0;
  int offset = 0;

  // skip spaces
  while (offset < num.length() - 1 && num.at(offset) == QLatin1Char(' '))
    ++offset;

  // handle minus
  if (num.at(offset) == QLatin1Char('-'))
  {
    p->drawPixmap(x, 0, *m_bigMinus);
    ++offset;
    x += 28;
  }
  else
    x += 28;

  // map special chars
  static const QMap<QChar, int> specialCharMap =
  {
    { 'a', 0 }, { 'b', 1 }, { 'd', 2 }, { 'e', 3 },
    { 'h', 4 }, { 'l', 5 }, { 'n', 6 }, { 'o', 7 },
    { 'p', 8 }, { 'r', 9 }, { 's', 10 }, { 't', 11 }, { 'y', 12 }
  };

  for (int i = offset; i < num.length(); ++i)
  {
    QChar ch = num.at(i).toLower();

    if (ch == QLatin1Char('.'))
      p->drawPixmap(x - 11, 0, *m_bigDecimal);
    else if (specialCharMap.contains(ch))
    {
      int index = specialCharMap.value(ch);
      p->drawPixmap(x + 2, 2, *m_bigSpecialChar, 34 * index, 0, 34, 60);
      x += 49;
    }
    else if (ch == QLatin1Char(' '))
      x += 49;
    else if (ch.isDigit())
    {
      int digit = ch.toLatin1() - '0';
      p->drawPixmap(x + 2, 2, *m_bigDigit, 34 * digit, 0, 34, 60);
      x += 49;
    }
  }
}

unsigned int DisplayWid::calcNumDigits(unsigned int dm)
{
  unsigned int numDigits = 0;
  for (unsigned int n = 1; dm >= n; ++numDigits, n *= 10) {}

  return numDigits;
}

QBitmap *DisplayWid::BitmapHelper(const QString &file) const
{
  return new QBitmap(QPixmap(file).createMaskFromColor(Qt::black, Qt::MaskOutColor));
}
