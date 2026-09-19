//======================================================================
// File:		meterwid.cpp
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
//======================================================================

#include "meterwid.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QRegularExpression>
#include <QFontMetricsF>
#include <cmath>
#include <limits>

namespace
{
const double kSweep = 45.0;      // needle travels -kSweep .. +kSweep
const double kStop = 49.5;       // mechanical end stops
const double kNaN = std::numeric_limits<double>::quiet_NaN();

// Ballistics: a slightly under-damped second order system, so the needle
// has the weight of a real moving coil and overshoots by a hair.
const double kOmega = 2.0 * M_PI * 1.5;   // rad/s
const double kZeta = 0.7;
const int kTickMs = 16;

// Point on a circle around the pivot; angle in degrees, 0 = straight up,
// positive = clockwise (to the right).
QPointF polar(const QPointF &pivot, double radius, double angleDeg)
{
  const double a = angleDeg * M_PI / 180.0;
  return QPointF(pivot.x() + radius * std::sin(a), pivot.y() - radius * std::cos(a));
}

// Arc around the pivot from angle a0 to a1 (our angle convention), as a
// sub-path that continues the current point when `lineToStart` is false.
void addArc(QPainterPath &path, const QPointF &pivot, double radius, double a0, double a1, bool moveToStart)
{
  const QRectF r(pivot.x() - radius, pivot.y() - radius, 2 * radius, 2 * radius);
  // Qt: 0 deg = 3 o'clock, counter-clockwise positive.
  const double qtStart = 90.0 - a0;
  const double qtSpan = -(a1 - a0);
  if (moveToStart)
    path.arcMoveTo(r, qtStart);
  path.arcTo(r, qtStart, qtSpan);
}

QFont scaledFont(double px, bool bold = false)
{
  QFont f;
  f.setPixelSize(qMax(5, int(std::lround(px))));
  f.setBold(bold);
  return f;
}
}

MeterStyle MeterStyle::dark()
{
  MeterStyle s;
  s.face = QColor(0x1c, 0x1c, 0x1c);
  s.faceGlow = QColor(0x3c, 0x3c, 0x3c);
  s.bezelLight = QColor(0x5c, 0x5c, 0x5c);
  s.bezelDark = QColor(0x1e, 0x1e, 0x1e);
  s.scale = QColor(0xf2, 0xf2, 0xf2);
  s.needle = QColor(0xff, 0xff, 0xff);
  s.redZone = QColor(0xd8, 0x22, 0x22);
  s.boxBg = QColor(0x2c, 0x2c, 0x2c);
  s.boxText = QColor(0xea, 0xea, 0xea);
  s.lampOff = QColor(0x4a, 0x12, 0x12);
  s.lampOn = QColor(0xff, 0x30, 0x30);
  s.hold = QColor(0xff, 0xb0, 0x20);
  return s;
}

MeterStyle MeterStyle::ivory()
{
  MeterStyle s;
  s.face = QColor(0xf1, 0xe9, 0xd2);
  s.faceGlow = QColor(0xff, 0xf9, 0xe8);
  s.bezelLight = QColor(0x6c, 0x6c, 0x6c);
  s.bezelDark = QColor(0x28, 0x28, 0x28);
  s.scale = QColor(0x1e, 0x1e, 0x1e);
  s.needle = QColor(0x10, 0x10, 0x10);
  s.redZone = QColor(0xc8, 0x18, 0x18);
  s.boxBg = QColor(0xe4, 0xda, 0xbe);
  s.boxText = QColor(0x1e, 0x1e, 0x1e);
  s.lampOff = QColor(0x6a, 0x20, 0x20);
  s.lampOn = QColor(0xff, 0x30, 0x30);
  s.hold = QColor(0xb0, 0x60, 0x00);
  return s;
}

MeterWid::MeterWid(QWidget *parent)
  : QWidget(parent)
  , m_peak(kNaN)
{
  setAttribute(Qt::WA_OpaquePaintEvent, false);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_timer.setInterval(kTickMs);
  connect(&m_timer, &QTimer::timeout, this, &MeterWid::stepBallistics);
}

QSize MeterWid::sizeHint() const
{
  return QSize(480, 270);
}

QSize MeterWid::minimumSizeHint() const
{
  return QSize(240, 135);
}

// ---------------------------------------------------------------- values

double MeterWid::angleForValue(double value, double fullScale, bool bipolar)
{
  if (!(fullScale > 0.0) || std::isnan(value))
    return -kSweep;
  const double frac = bipolar ? (value / fullScale + 1.0) / 2.0 : value / fullScale;
  return qBound(-kStop, -kSweep + 2.0 * kSweep * frac, kStop);
}

double MeterWid::fullScaleFromReading(const QString &value, int counts)
{
  if (counts <= 0)
    return kNaN;
  static const QRegularExpression re("^\\s*[-+]?\\s*(\\d+)(?:\\.(\\d*))?\\s*$");
  const QRegularExpressionMatch m = re.match(value);
  if (!m.hasMatch())
    return kNaN;
  const int decimals = m.captured(2).size();
  return counts / std::pow(10.0, decimals);
}

void MeterWid::setReading(double value, const QString &text, const QString &unit, bool overload, bool hold)
{
  if (m_unit != unit)
  {
    m_unit = unit;
    m_staticDirty = true;
  }
  m_value = value;
  m_text = text;
  m_overload = overload;
  m_hold = hold;

  if (m_scaleMode == Auto && !overload && value < -0.05 * m_fullScale && !m_bipolar)
  {
    m_bipolar = true;
    m_staticDirty = true;
  }
  retarget();
  update();
}

void MeterWid::setFullScale(double fs)
{
  if (!(fs > 0.0) || qFuzzyCompare(fs, m_fullScale))
    return;
  m_fullScale = fs;
  m_staticDirty = true;
  retarget();
  update();
}

void MeterWid::setPeak(double value)
{
  m_peak = value;
  update();
}

void MeterWid::setScaleMode(ScaleMode mode)
{
  m_scaleMode = mode;
  const bool bipolar = (mode == Bipolar);
  if (bipolar != m_bipolar)
  {
    m_bipolar = bipolar;
    m_staticDirty = true;
  }
  retarget();
  update();
}

void MeterWid::setStyle(const MeterStyle &style)
{
  m_style = style;
  m_staticDirty = true;
  if (!m_style.ballistics)
  {
    m_timer.stop();
    m_angle = m_target;
  }
  update();
}

void MeterWid::reset()
{
  m_peak = kNaN;
  if (m_scaleMode == Auto && m_bipolar)
  {
    m_bipolar = false;
    m_staticDirty = true;
  }
  retarget();
  update();
}

void MeterWid::retarget()
{
  m_target = m_overload ? kStop : angleForValue(m_value, m_fullScale, m_bipolar);
  if (!m_style.ballistics)
  {
    m_angle = m_target;
    return;
  }
  if (!m_timer.isActive() && std::fabs(m_target - m_angle) > 0.05)
    m_timer.start();
}

void MeterWid::stepBallistics()
{
  const double dt = kTickMs / 1000.0;
  const double acc = kOmega * kOmega * (m_target - m_angle) - 2.0 * kZeta * kOmega * m_velocity;
  m_velocity += acc * dt;
  m_angle += m_velocity * dt;
  m_angle = qBound(-kStop - 1.0, m_angle, kStop + 1.0);

  if (std::fabs(m_target - m_angle) < 0.05 && std::fabs(m_velocity) < 0.5)
  {
    m_angle = m_target;
    m_velocity = 0.0;
    m_timer.stop();
  }
  update();
}

// -------------------------------------------------------------- geometry

MeterWid::Geometry MeterWid::geometry() const
{
  Geometry g;
  const QRectF r = rect().adjusted(1, 1, -1, -1);
  g.bezel = r;
  const double bezelW = qMax(4.0, r.height() * 0.06);
  g.face = r.adjusted(bezelW, bezelW, -bezelW, -bezelW);
  const double fh = g.face.height();
  const double fw = g.face.width();
  g.radius = qMin(fh * 0.85, fw * 0.66);
  g.pivot = QPointF(g.face.center().x(), g.face.bottom() + fh * 0.02);
  g.fontPx = qBound(6.0, fh * 0.085, 48.0);
  return g;
}

void MeterWid::resizeEvent(QResizeEvent *)
{
  m_staticDirty = true;
}

// --------------------------------------------------------------- drawing

void MeterWid::drawBezel(QPainter &p, const Geometry &g) const
{
  const double radius = g.bezel.height() * 0.07;

  QLinearGradient metal(g.bezel.topLeft(), g.bezel.bottomLeft());
  metal.setColorAt(0.0, m_style.bezelLight);
  metal.setColorAt(0.5, m_style.bezelDark.lighter(130));
  metal.setColorAt(1.0, m_style.bezelDark);
  p.setPen(QPen(m_style.bezelDark.darker(160), 1));
  p.setBrush(metal);
  p.drawRoundedRect(g.bezel, radius, radius);

  // thin highlight along the top edge
  p.setPen(QPen(QColor(255, 255, 255, 60), 1));
  p.setBrush(Qt::NoBrush);
  p.drawRoundedRect(g.bezel.adjusted(1, 1, -1, -1), radius, radius);

  // dial: base colour, lit from around the pivot, recessed with an inner shadow
  const double faceRadius = g.face.height() * 0.05;
  QPainterPath faceClip;
  faceClip.addRoundedRect(g.face, faceRadius, faceRadius);
  p.save();
  p.setClipPath(faceClip);
  p.fillRect(g.face, m_style.face);

  QRadialGradient glow(g.pivot, g.radius * 1.2);
  glow.setColorAt(0.0, m_style.faceGlow);
  glow.setColorAt(1.0, Qt::transparent);
  p.fillRect(g.face, glow);

  for (int i = 0; i < 4; ++i)
  {
    p.setPen(QPen(QColor(0, 0, 0, 70 - i * 15), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(g.face.adjusted(i, i, -i, -i), faceRadius, faceRadius);
  }
  p.restore();
}

double MeterWid::niceStep(double range, int targetMajors)
{
  const double raw = range / qMax(1, targetMajors);
  const double mag = std::pow(10.0, std::floor(std::log10(raw)));
  const double norm = raw / mag;
  double step;
  if (norm < 1.5)
    step = 1.0;
  else if (norm < 3.5)
    step = 2.0;
  else if (norm < 7.5)
    step = 5.0;
  else
    step = 10.0;
  return step * mag;
}

QString MeterWid::formatLabel(double v)
{
  QString s = QString::number(v, 'f', 3);
  while (s.endsWith('0'))
    s.chop(1);
  if (s.endsWith('.'))
    s.chop(1);
  if (s == "-0")
    s = "0";
  return s;
}

void MeterWid::drawScale(QPainter &p, const Geometry &g) const
{
  const double R = g.radius;
  const double vMin = m_bipolar ? -m_fullScale : 0.0;
  const double vMax = m_fullScale;
  const double step = niceStep(vMax - vMin, m_bipolar ? 8 : 5);
  const int minorsPerMajor = (std::fmod(step / std::pow(10.0, std::floor(std::log10(step))), 2.0) == 0.0) ? 4 : 5;
  const double minor = step / minorsPerMajor;
  auto angleOf = [&](double v) { return angleForValue(v, m_fullScale, m_bipolar); };

  // main arc, with the underswing stub left of zero
  QPainterPath arc;
  addArc(arc, g.pivot, R, -kStop, kSweep, true);
  p.setPen(QPen(m_style.scale, qMax(1.0, R * 0.012), Qt::SolidLine, Qt::FlatCap));
  p.setBrush(Qt::NoBrush);
  p.drawPath(arc);

  // red zone: band on the arc from redZoneFrom * FS to the end (both ends
  // when bipolar); the ticks are drawn over it afterwards
  {
    auto band = [&](double v0, double v1)
    {
      QPainterPath path;
      addArc(path, g.pivot, R * 1.012, angleOf(v0), angleOf(v1), true);
      addArc(path, g.pivot, R * 0.94, angleOf(v1), angleOf(v0), false);
      path.closeSubpath();
      p.setPen(Qt::NoPen);
      p.setBrush(m_style.redZone);
      p.drawPath(path);
    };
    const double from = m_style.redZoneFrom * m_fullScale;
    band(from, vMax);
    if (m_bipolar)
      band(-vMax, -from);
  }

  // ticks and labels
  const QFont labelFont = scaledFont(g.fontPx, true);
  p.setFont(labelFont);
  const QFontMetricsF fm(labelFont);
  const double majorLen = R * 0.11;
  const double minorLen = R * 0.055;
  const int nMinor = int(std::lround((vMax - vMin) / minor));
  for (int i = 0; i <= nMinor; ++i)
  {
    const double v = vMin + i * minor;
    const bool major = (i % minorsPerMajor) == 0;
    const double a = angleOf(v);
    const bool inRed = std::fabs(v) >= m_style.redZoneFrom * m_fullScale - 1e-9 && v != 0.0;
    const QColor c = inRed ? m_style.redZone.lighter(115) : m_style.scale;
    p.setPen(QPen(m_style.scale, major ? qMax(1.0, R * 0.012) : qMax(0.8, R * 0.007), Qt::SolidLine, Qt::FlatCap));
    p.drawLine(polar(g.pivot, R, a), polar(g.pivot, R - (major ? majorLen : minorLen), a));

    if (major)
    {
      const QString label = formatLabel(v);
      const QPointF c0 = polar(g.pivot, R + g.fontPx * 1.0, a);
      const double w = fm.horizontalAdvance(label) + 4;
      const double h = fm.height();
      p.setPen(c);
      p.drawText(QRectF(c0.x() - w / 2, c0.y() - h / 2, w, h), Qt::AlignCenter, label);
    }
  }

  // inner percentage scale (unipolar only, it has no meaning around zero)
  if (m_style.percentScale && !m_bipolar)
  {
    const double r2 = R * 0.74;
    QPainterPath arc2;
    addArc(arc2, g.pivot, r2, angleOf(0.0), angleOf(vMax), true);
    p.setPen(QPen(m_style.scale, qMax(0.8, R * 0.006)));
    p.setBrush(Qt::NoBrush);
    p.drawPath(arc2);
    const QFont small = scaledFont(g.fontPx * 0.6);
    p.setFont(small);
    const QFontMetricsF fm2(small);
    for (int pct = 0; pct <= 100; pct += 20)
    {
      const double a = angleOf(pct / 100.0 * vMax);
      p.drawLine(polar(g.pivot, r2, a), polar(g.pivot, r2 - R * 0.035, a));
      const QString label = QString::number(pct);
      const QPointF c0 = polar(g.pivot, r2 - g.fontPx * 0.75, a);
      const double w = fm2.horizontalAdvance(label) + 4;
      const double h = fm2.height();
      p.drawText(QRectF(c0.x() - w / 2, c0.y() - h / 2, w, h), Qt::AlignCenter, label);
    }
  }

  // pivot cap at the bottom edge of the dial
  {
    const double capR = g.face.height() * 0.09;
    p.save();
    QPainterPath faceClip;
    faceClip.addRect(g.face);
    p.setClipPath(faceClip);
    QRadialGradient cap(g.pivot - QPointF(capR * 0.3, capR * 0.6), capR * 1.4);
    cap.setColorAt(0.0, m_style.bezelLight.lighter(120));
    cap.setColorAt(1.0, m_style.bezelDark);
    p.setPen(QPen(m_style.bezelDark.darker(150), 1));
    p.setBrush(cap);
    p.drawEllipse(g.pivot, capR, capR);
    p.setPen(Qt::NoPen);
    p.setBrush(m_style.scale);
    p.drawEllipse(g.pivot - QPointF(0, capR * 0.55), capR * 0.09, capR * 0.09);
    if (m_bipolar)
    {
      p.setFont(scaledFont(g.fontPx * 0.7, true));
      p.setPen(m_style.scale);
      const double h = g.fontPx;
      p.drawText(QRectF(g.pivot.x() - capR * 2.2, g.pivot.y() - capR * 0.9, capR, h), Qt::AlignCenter, QStringLiteral("−"));
      p.drawText(QRectF(g.pivot.x() + capR * 1.2, g.pivot.y() - capR * 0.9, capR, h), Qt::AlignCenter, QStringLiteral("+"));
    }
    p.restore();
  }
}

void MeterWid::drawBoxes(QPainter &p, const Geometry &g) const
{
  const double fw = g.face.width();
  const double fh = g.face.height();
  const double rr = fh * 0.03;

  auto box = [&](const QRectF &r)
  {
    p.setPen(QPen(QColor(0, 0, 0, 120), 1));
    p.setBrush(m_style.boxBg);
    p.drawRoundedRect(r, rr, rr);
    p.setPen(QPen(QColor(255, 255, 255, 30), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r.adjusted(1, 1, -1, -1), rr, rr);
  };

  // unit label in the middle, like the "VU" plate
  const QRectF unitBox(g.face.center().x() - fw * 0.13, g.face.top() + fh * 0.60, fw * 0.26, fh * 0.17);
  box(unitBox);
  p.setFont(scaledFont(g.fontPx * 1.35, true));
  p.setPen(m_style.boxText);
  p.drawText(unitBox, Qt::AlignCenter, m_unit.isEmpty() ? QStringLiteral("—") : m_unit);

  // readout boxes with their captions
  const QRectF cur(g.face.left() + fw * 0.05, g.face.top() + fh * 0.78, fw * 0.28, fh * 0.16);
  const QRectF max(g.face.right() - fw * 0.05 - fw * 0.28, cur.top(), fw * 0.28, fh * 0.16);
  box(cur);
  box(max);
  p.setFont(scaledFont(g.fontPx * 0.7));
  p.setPen(m_style.scale.darker(m_style.face.lightness() < 128 ? 150 : 100));
  p.drawText(QRectF(cur.left(), cur.top() - g.fontPx * 1.05, cur.width(), g.fontPx), Qt::AlignLeft | Qt::AlignVCenter, tr("CURRENT"));
  p.drawText(QRectF(max.left(), max.top() - g.fontPx * 1.05, max.width(), g.fontPx), Qt::AlignRight | Qt::AlignVCenter, tr("MAX"));

  // "OL" caption next to the lamp
  const QPointF lamp(g.face.right() - fw * 0.09, g.face.top() + fh * 0.66);
  p.setFont(scaledFont(g.fontPx * 0.7, true));
  p.drawText(QRectF(lamp.x() + fh * 0.05, lamp.y() - g.fontPx * 0.5, fw * 0.05, g.fontPx), Qt::AlignLeft | Qt::AlignVCenter, tr("OL"));
}

void MeterWid::drawReadouts(QPainter &p, const Geometry &g) const
{
  const double fw = g.face.width();
  const double fh = g.face.height();
  const QRectF cur(g.face.left() + fw * 0.05, g.face.top() + fh * 0.78, fw * 0.28, fh * 0.16);
  const QRectF max(g.face.right() - fw * 0.05 - fw * 0.28, cur.top(), fw * 0.28, fh * 0.16);

  p.setFont(scaledFont(g.fontPx * 1.15, true));
  p.setPen(m_style.boxText);
  p.drawText(cur.adjusted(fh * 0.02, 0, -fh * 0.02, 0), Qt::AlignCenter,
             m_overload ? tr("OL") : m_text.trimmed());

  QString peakText = QStringLiteral("—");
  if (!std::isnan(m_peak))
  {
    // same number of decimals as the meter's own rendering of the value
    const int dot = m_text.indexOf('.');
    const int decimals = dot < 0 ? 0 : m_text.trimmed().size() - dot - 1;
    peakText = QString::number(m_peak, 'f', qBound(0, decimals, 6));
  }
  p.drawText(max.adjusted(fh * 0.02, 0, -fh * 0.02, 0), Qt::AlignCenter, peakText);

  if (m_hold)
  {
    p.setFont(scaledFont(g.fontPx * 0.8, true));
    p.setPen(m_style.hold);
    p.drawText(QRectF(g.face.left() + fw * 0.05, g.face.top() + fh * 0.60, fw * 0.2, g.fontPx * 1.2),
               Qt::AlignLeft | Qt::AlignVCenter, tr("HOLD"));
  }
}

void MeterWid::drawLamp(QPainter &p, const Geometry &g) const
{
  const double fw = g.face.width();
  const double fh = g.face.height();
  const QPointF c(g.face.right() - fw * 0.09, g.face.top() + fh * 0.66);
  const double r = fh * 0.035;

  if (m_overload)
  {
    QRadialGradient halo(c, r * 3.0);
    halo.setColorAt(0.0, QColor(m_style.lampOn.red(), m_style.lampOn.green(), m_style.lampOn.blue(), 140));
    halo.setColorAt(1.0, Qt::transparent);
    p.setPen(Qt::NoPen);
    p.setBrush(halo);
    p.drawEllipse(c, r * 3.0, r * 3.0);
  }

  QRadialGradient lamp(c - QPointF(r * 0.3, r * 0.3), r * 1.3);
  const QColor base = m_overload ? m_style.lampOn : m_style.lampOff;
  lamp.setColorAt(0.0, base.lighter(m_overload ? 160 : 120));
  lamp.setColorAt(1.0, base.darker(140));
  p.setPen(QPen(QColor(0, 0, 0, 150), 1));
  p.setBrush(lamp);
  p.drawEllipse(c, r, r);
}

void MeterWid::drawNeedle(QPainter &p, const Geometry &g) const
{
  const double len = g.radius * 1.02;
  const double baseHalf = qMax(1.2, g.face.height() * 0.011);
  const double tipHalf = qMax(0.4, baseHalf * 0.2);

  // needle polygon in a frame pointing straight up, then rotated
  QPolygonF poly;
  poly << QPointF(-baseHalf, 0) << QPointF(-tipHalf, -len) << QPointF(tipHalf, -len) << QPointF(baseHalf, 0);

  p.save();
  QPainterPath clip;
  clip.addRoundedRect(g.face, g.face.height() * 0.05, g.face.height() * 0.05);
  p.setClipPath(clip);

  // shadow
  p.save();
  p.translate(g.pivot + QPointF(baseHalf * 1.5, baseHalf * 1.5));
  p.rotate(m_angle);
  p.setPen(Qt::NoPen);
  p.setBrush(QColor(0, 0, 0, 110));
  p.drawPolygon(poly);
  p.restore();

  p.translate(g.pivot);
  p.rotate(m_angle);
  p.setPen(Qt::NoPen);
  p.setBrush(m_style.needle);
  p.drawPolygon(poly);
  p.restore();
}

void MeterWid::renderStatic()
{
  const qreal dpr = devicePixelRatioF();
  m_static = QPixmap(size() * dpr);
  m_static.setDevicePixelRatio(dpr);
  m_static.fill(Qt::transparent);

  QPainter p(&m_static);
  p.setRenderHint(QPainter::Antialiasing);
  p.setRenderHint(QPainter::TextAntialiasing);
  const Geometry g = geometry();
  drawBezel(p, g);
  drawScale(p, g);
  drawBoxes(p, g);
  m_staticDirty = false;
}

void MeterWid::paintEvent(QPaintEvent *)
{
  if (m_staticDirty || m_static.isNull())
    renderStatic();

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.setRenderHint(QPainter::TextAntialiasing);
  p.drawPixmap(0, 0, m_static);

  const Geometry g = geometry();
  drawReadouts(p, g);
  drawLamp(p, g);
  drawNeedle(p, g);
}
