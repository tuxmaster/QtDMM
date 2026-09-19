//======================================================================
// File:		panelframe.h
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

#pragma once

#include <QColor>
#include <QRectF>
#include <QPointF>

class QPainter;

// The instrument housing shared by the analog meter and the digital
// display: a rounded metal bezel with a recessed face, lit from a point.
namespace PanelFrame
{
  struct Colors
  {
    QColor bezelLight;
    QColor bezelDark;
    QColor face;
    QColor faceGlow;    // radial highlight on the face; transparent = none
  };

  // Face rect for a bezel rect (the inset the frame uses).
  QRectF faceRect(const QRectF &bezel);

  // Paints bezel and face. glowCenter is where the face is lit from.
  void paint(QPainter &p, const QRectF &bezel, const Colors &c, const QPointF &glowCenter, double glowRadius);

  // Clip path of the face (rounded), for drawing that must stay inside.
  void clipToFace(QPainter &p, const QRectF &face);
}
