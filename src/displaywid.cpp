//======================================================================
// File:		displaywid.cpp
// Author:	Matthias Toussaint
// Created:	Thu Dec 26 12:04:35 CET 2002
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

#include "displaywid.h"
#include "panelframe.h"
#include "siprefix.h"

#include <QPainter>
#include <QPainterPath>
#include <QFontMetricsF>
#include <cmath>

namespace
{
enum Segment { A = 1, B = 2, C = 4, D = 8, E = 16, F = 32, G = 64 };

// Proportions of a seven-segment cell, relative to its height.
const double kDigitW = 0.56;      // cell width
const double kThick = 0.14;       // segment thickness
const double kGap = 0.018;        // gap between segments
const double kAdvance = 0.74;     // cell pitch
const double kDpW = 0.22;         // decimal point pitch
const double kSlant = 0.08;       // italic shear

// Housing colours: same dark metal as the analog meter.
const QColor kBezelLight(0x5c, 0x5c, 0x5c);
const QColor kBezelDark(0x1e, 0x1e, 0x1e);

QFont sansFont(double px, bool bold = true)
{
  QFont f;
  f.setPixelSize(qMax(5, int(std::lround(px))));
  f.setBold(bold);
  return f;
}
}

DisplayWid::DisplayWid(QWidget *parent)
  : QWidget(parent)
  , m_face(0xb6, 0xcf, 0xa4)
  , m_segment(0x16, 0x1e, 0x14)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setDisplayMode(4000, true, true, 1);
}

QSize DisplayWid::sizeHint() const
{
  return QSize(520, 200);
}

QSize DisplayWid::minimumSizeHint() const
{
  return QSize(260, 100);
}

// ---------------------------------------------------------------- state

void DisplayWid::setValue(int id, const QString &v) { m_value[id] = v; }
void DisplayWid::setUnit(int id, const QString &v) { m_unit[id] = v; }
void DisplayWid::setMode(int id, const QString &v) { m_mode[id] = v; }
void DisplayWid::setMinValue(const QString &v) { m_minValue = v; }
void DisplayWid::setMaxValue(const QString &v) { m_maxValue = v; }
void DisplayWid::setMinUnit(const QString &v) { m_minUnit = v; }
void DisplayWid::setMaxUnit(const QString &v) { m_maxUnit = v; }
void DisplayWid::setShowBar(bool on) { m_paintBar = on; }
void DisplayWid::setHold(bool on) { m_hold = on; }

void DisplayWid::setAuto(bool on)
{
  m_auto = on;
  m_manu = !on;
}

void DisplayWid::setManu(bool on)
{
  m_manu = on;
  m_auto = !on;
}

void DisplayWid::setDisplayMode(int counts, bool minMax, bool bar, int numValues)
{
  m_counts = counts > 0 ? counts : 4000;
  m_showMinMax = minMax;
  m_showBar = bar;
  m_numValues = qBound(1, numValues, 4);
  m_staticDirty = true;
  updateGeometry();
  update();
}

void DisplayWid::setFaceColor(const QColor &c)
{
  if (!c.isValid() || c == m_face)
    return;
  m_face = c;
  m_staticDirty = true;
  update();
}

QColor DisplayWid::ghost() const
{
  QColor c = m_segment;
  c.setAlpha(m_face.lightness() < 128 ? 40 : 26);
  return c;
}

int DisplayWid::numDigits() const
{
  // digits of the display count: 2000 -> 4, 22000 -> 5, 1000000 -> 7
  return QString::number(m_counts).size();
}

// ---------------------------------------------------------- segments

int DisplayWid::segmentsFor(QChar ch)
{
  switch (ch.toLower().toLatin1())
  {
    case '0': return A | B | C | D | E | F;
    case '1': return B | C;
    case '2': return A | B | G | E | D;
    case '3': return A | B | G | C | D;
    case '4': return F | G | B | C;
    case '5': return A | F | G | C | D;
    case '6': return A | F | G | E | D | C;
    case '7': return A | B | C;
    case '8': return A | B | C | D | E | F | G;
    case '9': return A | B | C | D | F | G;
    case '-': return G;
    case 'a': return A | B | C | E | F | G;
    case 'b': return F | E | G | C | D;
    case 'c': return G | E | D;
    case 'd': return B | C | D | E | G;
    case 'e': return A | F | G | E | D;
    case 'f': return A | F | G | E;
    case 'h': return F | E | G | C;
    case 'l': return F | E | D;
    case 'n': return E | G | C;
    case 'o': return G | E | C | D;
    case 'p': return A | B | F | G | E;
    case 'r': return E | G;
    case 's': return A | F | G | C | D;
    case 't': return F | G | E | D;
    case 'u': return B | C | D | E | F;
    case 'y': return F | B | G | C | D;
    default: return 0;
  }
}

void DisplayWid::drawDigit(QPainter &p, const QPointF &origin, double h, int mask) const
{
  const double w = h * kDigitW;
  const double t = h * kThick;
  const double g = h * kGap;
  const double half = t / 2.0;

  // A segment is a hexagon: a bar with pointed ends, so neighbours meet at
  // 45 degrees like on a real LCD.
  auto horizontal = [&](double y, double x0, double x1)
  {
    QPolygonF poly;
    poly << QPointF(x0 + g, y) << QPointF(x0 + g + half, y - half) << QPointF(x1 - g - half, y - half)
         << QPointF(x1 - g, y) << QPointF(x1 - g - half, y + half) << QPointF(x0 + g + half, y + half);
    return poly;
  };
  auto vertical = [&](double x, double y0, double y1)
  {
    QPolygonF poly;
    poly << QPointF(x, y0 + g) << QPointF(x + half, y0 + g + half) << QPointF(x + half, y1 - g - half)
         << QPointF(x, y1 - g) << QPointF(x - half, y1 - g - half) << QPointF(x - half, y0 + g + half);
    return poly;
  };

  const double top = half, mid = h / 2.0, bottom = h - half;
  const double left = half, right = w - half;
  const QPolygonF segs[7] = {
    horizontal(top, left, right),          // a
    vertical(right, top, mid),             // b
    vertical(right, mid, bottom),          // c
    horizontal(bottom, left, right),       // d
    vertical(left, mid, bottom),           // e
    vertical(left, top, mid),              // f
    horizontal(mid, left, right),          // g
  };

  p.save();
  p.translate(origin);
  p.shear(-kSlant, 0.0);
  p.setPen(Qt::NoPen);
  for (int i = 0; i < 7; ++i)
  {
    p.setBrush((mask & (1 << i)) ? m_segment : ghost());
    p.drawPolygon(segs[i]);
  }
  p.restore();
}

double DisplayWid::numberWidth(double h, int digits) const
{
  // sign cell + digits + one decimal point
  return (digits + 1) * h * kAdvance + h * kDpW;
}

// Draws a reading right-aligned in a field of minDigits digits: leading
// cells stay ghosted, the sign has its own cell, the decimal point sits
// between cells. Returns the field width.
double DisplayWid::drawNumber(QPainter &p, const QPointF &origin, double h, const QString &text, int minDigits, bool lit) const
{
  const double adv = h * kAdvance;
  const double dpW = h * kDpW;
  const double fieldW = numberWidth(h, minDigits);

  QString s = text.trimmed();
  bool negative = false;
  if (s.startsWith('-'))
  {
    negative = true;
    s.remove(0, 1);
  }
  else if (s.startsWith('+'))
    s.remove(0, 1);
  s = s.trimmed();

  // character cells without the decimal point, and where the point goes
  QString cells;
  int dpAfter = -1;
  for (QChar ch : s)
  {
    if (ch == '.' || ch == ',')
      dpAfter = cells.size() - 1;
    else
      cells += ch;
  }
  while (cells.size() < minDigits)
  {
    cells.prepend(' ');
    if (dpAfter >= 0)
      dpAfter++;
  }
  const int n = cells.size();

  // the minus sits right before the first digit: in the last padding cell
  // if there is one, otherwise in the dedicated sign cell
  int minusCell = -1;
  if (negative && lit)
  {
    int blanks = 0;
    while (blanks < n && cells[blanks] == ' ')
      blanks++;
    minusCell = blanks > 0 ? blanks - 1 : -1;
  }
  double x = origin.x();
  drawDigit(p, QPointF(x, origin.y()), h, (negative && lit && minusCell < 0) ? G : 0);
  x += adv;

  // the decimal point takes its slot after the cell it follows; the field is
  // sized for exactly one point, so a value without one leaves the slot empty
  int dpSlot = dpAfter >= 0 ? dpAfter : n - 1;
  for (int i = 0; i < n; ++i)
  {
    const int mask = i == minusCell ? int(G) : (lit ? segmentsFor(cells[i]) : 0);
    drawDigit(p, QPointF(x, origin.y()), h, mask);
    x += adv;
    if (i == dpSlot)
    {
      const double r = h * kThick * 0.45;
      p.setPen(Qt::NoPen);
      p.setBrush((dpAfter >= 0 && lit) ? m_segment : ghost());
      p.drawEllipse(QPointF(x - adv * 0.17 - h * kSlant * 0.5 + dpW * 0.35, origin.y() + h - r), r, r);
      x += dpW;
    }
  }
  return fieldW;
}

double DisplayWid::drawUnit(QPainter &p, const QPointF &origin, double h, const QString &unit) const
{
  if (unit.isEmpty())
    return 0.0;
  SiPrefix::Split parts = SiPrefix::split(unit);
  QString text = parts.prefix + parts.baseUnit;
  if (parts.baseUnit == "Ohm")
    text = parts.prefix + QStringLiteral("Ω");
  else if (parts.baseUnit == "dF")
    text = QStringLiteral("°F");
  else if (parts.baseUnit == "C")
    text = QStringLiteral("°C");
  else if (parts.baseUnit == "cosphi")
    text = QStringLiteral("cosφ");

  const QFont f = sansFont(h * 0.62);
  p.setFont(f);
  p.setPen(m_segment);
  const QFontMetricsF fm(f);
  const double w = fm.horizontalAdvance(text);
  p.drawText(QRectF(origin.x(), origin.y(), w + 2, h), Qt::AlignLeft | Qt::AlignBottom, text);
  return w + 2;
}

void DisplayWid::drawAnnunciator(QPainter &p, const QRectF &r, const QString &text, bool on, double fontPx) const
{
  p.setFont(sansFont(fontPx));
  p.setPen(on ? m_segment : ghost());
  p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, text);
}

// ---------------------------------------------------------------- layout

DisplayWid::Layout DisplayWid::layout() const
{
  Layout l;
  l.bezel = PanelFrame::panelRect(QRectF(rect()).adjusted(1, 1, -1, -1), 1.8, 3.2);
  l.face = PanelFrame::faceRect(l.bezel);

  const double fh = l.face.height();
  const double fw = l.face.width();
  const double mx = fw * 0.04;
  const double my = fh * 0.06;
  const QRectF inner = l.face.adjusted(mx, my, -mx, -my);

  // row weights; the main row takes what is left
  const double flagsH = inner.height() * 0.14;
  const double minMaxH = m_showMinMax ? inner.height() * 0.20 : 0.0;
  const double barH = m_showBar ? inner.height() * 0.15 : 0.0;
  const double extraH = m_numValues > 1 ? inner.height() * 0.20 : 0.0;
  const double mainH = inner.height() - flagsH - minMaxH - barH - extraH;

  double y = inner.top();
  l.flags = QRectF(inner.left(), y, inner.width(), flagsH);
  y += flagsH;
  l.main = QRectF(inner.left(), y, inner.width(), mainH);
  y += mainH;
  l.minMax = QRectF(inner.left(), y, inner.width(), minMaxH);
  y += minMaxH;
  l.extra = QRectF(inner.left(), y, inner.width(), extraH);
  y += extraH;
  l.bar = QRectF(inner.left(), y, inner.width(), barH);

  // digit height: fits the row, and the number plus unit fit the width
  const int digits = numDigits();
  double h = mainH * 0.78;
  const double unitFactor = 0.62 * 2.6;     // ~ width of "mV" at 0.62 h
  const double needed = numberWidth(1.0, digits) + 0.3 + unitFactor;
  if (h * needed > inner.width())
    h = inner.width() / needed;
  l.digitH = h;

  double sh = qMin(minMaxH > 0 ? minMaxH * 0.7 : extraH * 0.7, h * 0.5);
  if (sh <= 0)
    sh = h * 0.4;
  l.smallH = sh;
  return l;
}

void DisplayWid::resizeEvent(QResizeEvent *)
{
  m_staticDirty = true;
}

// --------------------------------------------------------------- drawing

void DisplayWid::drawFlags(QPainter &p, const Layout &l) const
{
  const double fontPx = l.flags.height() * 0.75;
  const QFontMetricsF fm(sansFont(fontPx));
  const double gap = fontPx * 0.9;
  const QString mode = m_mode[0];

  struct Flag { QString text; bool on; };
  const Flag left[] = {
    { tr("HOLD"), m_hold },
    { tr("AUTO"), m_auto },
    { tr("MANU"), m_manu },
  };
  const Flag right[] = {
    { QStringLiteral("AC"), mode == "AC" || mode == "ACDC" },
    { QStringLiteral("DC"), mode == "DC" || mode == "ACDC" },
    { QStringLiteral("→|←"), mode == "DI" || mode == "Diode" },   // diode
    { QStringLiteral("●))"), mode == "BUZ" },                          // continuity
  };

  double x = l.flags.left();
  for (const Flag &f : left)
  {
    const double w = fm.horizontalAdvance(f.text);
    drawAnnunciator(p, QRectF(x, l.flags.top(), w + 2, l.flags.height()), f.text, f.on, fontPx);
    x += w + gap;
  }
  double xr = l.flags.right();
  for (int i = int(sizeof(right) / sizeof(right[0])) - 1; i >= 0; --i)
  {
    const double w = fm.horizontalAdvance(right[i].text);
    xr -= w;
    drawAnnunciator(p, QRectF(xr, l.flags.top(), w + 2, l.flags.height()), right[i].text, right[i].on, fontPx);
    xr -= gap;
  }
}

void DisplayWid::drawMain(QPainter &p, const Layout &l) const
{
  const double h = l.digitH;
  const int digits = numDigits();
  const double numW = numberWidth(h, digits);
  const double y = l.main.top() + (l.main.height() - h) / 2.0;

  // number left, unit right after it; the whole group is centred
  const QString unit = m_unit[0];
  const QFontMetricsF fm(sansFont(h * 0.62));
  const double unitW = unit.isEmpty() ? 0.0 : fm.horizontalAdvance(unit) + 2;
  const double groupW = numW + h * 0.3 + unitW;
  double x = l.main.left() + qMax(0.0, (l.main.width() - groupW) / 2.0);

  drawNumber(p, QPointF(x, y), h, m_value[0], digits, !m_value[0].isEmpty());
  x += numW + h * 0.3;
  drawUnit(p, QPointF(x, y), h, unit);
}

void DisplayWid::drawMinMax(QPainter &p, const Layout &l) const
{
  if (!m_showMinMax)
    return;
  const double h = l.smallH;
  const int digits = numDigits();
  const double y = l.minMax.top() + (l.minMax.height() - h) / 2.0;
  const double fontPx = h * 0.7;
  const QFontMetricsF fm(sansFont(fontPx));
  const double labelW = fm.horizontalAdvance(tr("MAX")) + h * 0.3;

  auto block = [&](double x, const QString &label, const QString &value, const QString &unit)
  {
    p.setFont(sansFont(fontPx));
    p.setPen(value.isEmpty() ? ghost() : m_segment);
    p.drawText(QRectF(x, y, labelW, h), Qt::AlignLeft | Qt::AlignVCenter, label);
    x += labelW;
    x += drawNumber(p, QPointF(x, y), h, value, digits, !value.isEmpty());
    x += h * 0.25;
    drawUnit(p, QPointF(x, y), h, unit);
  };

  block(l.minMax.left(), tr("MIN"), m_minValue, m_minUnit);
  block(l.minMax.left() + l.minMax.width() / 2.0, tr("MAX"), m_maxValue, m_maxUnit);
}

void DisplayWid::drawExtra(QPainter &p, const Layout &l) const
{
  if (m_numValues <= 1)
    return;
  const double h = l.smallH;
  const int digits = numDigits();
  const double y = l.extra.top() + (l.extra.height() - h) / 2.0;
  const double slot = l.extra.width() / (m_numValues - 1);
  for (int i = 1; i < m_numValues; ++i)
  {
    double x = l.extra.left() + (i - 1) * slot;
    x += drawNumber(p, QPointF(x, y), h, m_value[i], digits, !m_value[i].isEmpty());
    x += h * 0.25;
    drawUnit(p, QPointF(x, y), h, m_unit[i]);
  }
}

// The bar graph mirrors the one on the meter's own LCD: the reading's
// digits over the display count, in blocks with a tick scale above.
void DisplayWid::drawBar(QPainter &p, const Layout &l, bool staticPart) const
{
  if (!m_showBar)
    return;

  const int digits = numDigits();
  int divisions = int(m_counts / std::pow(10.0, digits - 2));
  if (divisions < 20 || divisions > 60)
    divisions = 50;

  const double tickH = l.bar.height() * 0.30;
  const double blockH = l.bar.height() * 0.42;
  const double fontPx = l.bar.height() * 0.32;
  const double x0 = l.bar.left();
  const double w = l.bar.width();
  const double step = w / divisions;
  const double tickTop = l.bar.top() + fontPx * 1.1;
  const double blockTop = l.bar.bottom() - blockH;

  if (staticPart)
  {
    p.setFont(sansFont(fontPx, false));
    for (int c = 0; c <= divisions; ++c)
    {
      const double x = x0 + c * step;
      const bool major = (c % 5) == 0;
      p.setPen(QPen(m_segment, major ? 1.5 : 1.0));
      p.drawLine(QPointF(x, tickTop + (major ? 0 : tickH * 0.5)), QPointF(x, tickTop + tickH));
      if (c % 10 == 0)
      {
        p.setPen(m_segment);
        p.drawText(QRectF(x - fontPx * 2, l.bar.top() - fontPx * 0.1, fontPx * 4, fontPx * 1.2), Qt::AlignCenter, QString::number(c));
      }
    }
    // ghost blocks
    p.setPen(Qt::NoPen);
    p.setBrush(ghost());
    for (int c = 0; c < divisions; ++c)
      p.drawRect(QRectF(x0 + c * step + 1, blockTop, step - 2, blockH));
    return;
  }

  if (!m_paintBar || m_value[0].isEmpty())
    return;
  QString val;
  for (QChar ch : m_value[0])
    if (ch.isDigit())
      val += ch;
  const double percent = qBound(0.0, val.toDouble() / m_counts, 1.0);
  const int lit = int(std::lround(percent * divisions));
  p.setPen(Qt::NoPen);
  p.setBrush(m_segment);
  for (int c = 0; c < lit; ++c)
    p.drawRect(QRectF(x0 + c * step + 1, blockTop, step - 2, blockH));
}

void DisplayWid::renderStatic()
{
  const qreal dpr = devicePixelRatioF();
  m_static = QPixmap(size() * dpr);
  m_static.setDevicePixelRatio(dpr);
  m_static.fill(Qt::transparent);

  QPainter p(&m_static);
  p.setRenderHint(QPainter::Antialiasing);
  p.setRenderHint(QPainter::TextAntialiasing);
  const Layout l = layout();

  PanelFrame::Colors c;
  c.bezelLight = kBezelLight;
  c.bezelDark = kBezelDark;
  c.face = m_face;
  c.faceGlow = m_face.lighter(112);
  PanelFrame::paint(p, l.bezel, c, l.face.center(), l.face.width() * 0.6);

  PanelFrame::clipToFace(p, l.face);
  drawBar(p, l, true);
  m_staticDirty = false;
}

void DisplayWid::paintEvent(QPaintEvent *)
{
  if (m_staticDirty || m_static.isNull())
    renderStatic();

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.setRenderHint(QPainter::TextAntialiasing);
  p.drawPixmap(0, 0, m_static);

  const Layout l = layout();
  PanelFrame::clipToFace(p, l.face);
  drawFlags(p, l);
  drawMain(p, l);
  drawMinMax(p, l);
  drawExtra(p, l);
  drawBar(p, l, false);
}
