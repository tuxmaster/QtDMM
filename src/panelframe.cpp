//======================================================================
// File:		panelframe.cpp
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

#include "panelframe.h"

#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QRadialGradient>

namespace PanelFrame
{

static double bezelWidth(const QRectF &bezel)
{
  return qMax(4.0, bezel.height() * 0.06);
}

QRectF faceRect(const QRectF &bezel)
{
  const double w = bezelWidth(bezel);
  return bezel.adjusted(w, w, -w, -w);
}

void clipToFace(QPainter &p, const QRectF &face)
{
  const double r = face.height() * 0.05;
  QPainterPath clip;
  clip.addRoundedRect(face, r, r);
  p.setClipPath(clip);
}

void paint(QPainter &p, const QRectF &bezel, const Colors &c, const QPointF &glowCenter, double glowRadius)
{
  const double radius = bezel.height() * 0.07;

  QLinearGradient metal(bezel.topLeft(), bezel.bottomLeft());
  metal.setColorAt(0.0, c.bezelLight);
  metal.setColorAt(0.5, c.bezelDark.lighter(130));
  metal.setColorAt(1.0, c.bezelDark);
  p.setPen(QPen(c.bezelDark.darker(160), 1));
  p.setBrush(metal);
  p.drawRoundedRect(bezel, radius, radius);

  // thin highlight along the top edge
  p.setPen(QPen(QColor(255, 255, 255, 60), 1));
  p.setBrush(Qt::NoBrush);
  p.drawRoundedRect(bezel.adjusted(1, 1, -1, -1), radius, radius);

  // recessed face: base colour, lit from glowCenter, inner shadow
  const QRectF face = faceRect(bezel);
  const double faceRadius = face.height() * 0.05;
  p.save();
  clipToFace(p, face);
  p.fillRect(face, c.face);

  if (c.faceGlow.alpha() > 0)
  {
    QRadialGradient glow(glowCenter, glowRadius);
    glow.setColorAt(0.0, c.faceGlow);
    glow.setColorAt(1.0, Qt::transparent);
    p.fillRect(face, glow);
  }

  for (int i = 0; i < 4; ++i)
  {
    p.setPen(QPen(QColor(0, 0, 0, 70 - i * 15), 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(face.adjusted(i, i, -i, -i), faceRadius, faceRadius);
  }
  p.restore();
}

}
