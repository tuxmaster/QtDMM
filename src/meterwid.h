//======================================================================
// File:		meterwid.h
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

#include <QWidget>
#include <QColor>
#include <QPixmap>
#include <QTimer>

/// Colour set and options of the analog meter (MeterWid).
struct MeterStyle
{
  QColor face;   ///< dial background
  QColor faceGlow;   ///< radial highlight around the pivot
  QColor bezelLight;   ///< bezel gradient top
  QColor bezelDark;   ///< bezel gradient bottom
  QColor scale;   ///< ticks, labels, arc
  QColor needle;
  QColor redZone;
  QColor boxBg;   ///< readout / unit boxes
  QColor boxText;
  QColor lampOff;
  QColor lampOn;
  QColor hold;   ///< HOLD indicator
  QColor minMark;   ///< min/max memory marks on the arc
  QColor maxMark;
  bool   percentScale = true;   ///< inner 0..100 % arc
  bool   ballistics = true;   ///< damped needle movement
  double redZoneFrom = 0.9;   ///< fraction of full scale

  static MeterStyle dark();    ///< black dial, white scale (the default)
  static MeterStyle ivory();   ///< cream dial, black scale, classic look
};

/// Analog moving-coil instrument in the style of a studio VU meter: dark
/// dial, white scale on an arc, red zone at the top end, needle swinging
/// from a pivot below the dial, readout boxes for the current value and the
/// peak, an overload lamp. Everything is drawn with QPainter, so it scales
/// with the widget and re-labels itself when the meter changes range.
///
/// The scale is in the unit the multimeter displays (with SI prefix), the
/// full scale comes from the display count of the meter - see
/// fullScaleFromReading(). Values are plain doubles in that unit.
class MeterWid : public QWidget
{
  Q_OBJECT
public:
  /// Where zero sits on the scale.
  enum ScaleMode
  {
    Unipolar,   ///< 0 at the left end, negative values push the needle below it
    Bipolar,    ///< -FS .. 0 .. +FS, zero in the middle
    Auto        ///< Unipolar until a value below -5 % FS arrives, then Bipolar until reset()
  };

  explicit MeterWid(QWidget *parent = nullptr);

  /// Current reading in display units. text is the meter's own rendering of
  /// it ("3.856", shown verbatim in the CURRENT box), unit what the dial
  /// label shows (e.g. "mV DC"); overload parks the needle at the right stop.
  void setReading(double value, const QString &text, const QString &unit, bool overload, bool hold);
  void setFullScale(double fs);   ///< > 0, in display units
  void setPeak(double value);   ///< MAX box, display units; NaN hides it
  /// Min/max memory as marks on the scale arc (display units; NaN hides one).
  void setMinMax(double minValue, double maxValue);
  void setScaleMode(ScaleMode mode);
  /// Colours and options; the static layers are re-rendered.
  void setStyle(const MeterStyle &style);
  void reset();   ///< peak and Auto-bipolar latch

  ScaleMode scaleMode() const { return m_scaleMode; }
  bool bipolar() const { return m_bipolar; }
  double fullScale() const { return m_fullScale; }
  double needleAngle() const { return m_angle; }   ///< degrees, 0 = straight up
  /// True once the needle ballistics have come to rest.
  bool isSettled() const { return !m_timer.isActive(); }

  /// Needle angle for a value: -45 deg at the left end of the scale, +45 deg at
  /// the right end, clamped to the mechanical stops at +-49.5 deg.
  static double angleForValue(double value, double fullScale, bool bipolar);

  /// Full scale implied by a reading string and the meter's display count:
  /// ("3.856", 4000) -> 4, ("385.6", 4000) -> 400. NaN if the string is not
  /// a number (OL etc.), so the caller keeps the previous scale.
  static double fullScaleFromReading(const QString &value, int counts);
  /// The same, but a percentage (unit "%": state of charge, duty cycle) is
  /// always a 0..100 scale, whatever the display count says.
  static double fullScaleFromReading(const QString &value, int counts, const QString &unit);

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

protected:
  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;

private:
  struct Geometry
  {
    QRectF bezel, face;
    QPointF pivot;
    double radius = 0;
    double fontPx = 10;
  };
  Geometry geometry() const;
  void renderStatic();   ///< dial, scale, boxes -> m_static
  void drawBezel(QPainter &p, const Geometry &g) const;
  void drawScale(QPainter &p, const Geometry &g) const;
  void drawBoxes(QPainter &p, const Geometry &g) const;
  void drawNeedle(QPainter &p, const Geometry &g) const;
  void drawReadouts(QPainter &p, const Geometry &g) const;
  void drawLamp(QPainter &p, const Geometry &g) const;
  void drawMarks(QPainter &p, const Geometry &g) const;
  void retarget();
  void stepBallistics();

  static QString formatLabel(double v);
  static double niceStep(double range, int targetMajors);

  MeterStyle m_style = MeterStyle::dark();
  ScaleMode m_scaleMode = Auto;
  bool m_bipolar = false;
  double m_fullScale = 4.0;
  double m_value = 0.0;
  QString m_text;
  QString m_unit;
  bool m_overload = false;
  bool m_hold = false;
  double m_peak;   ///< NaN = none
  double m_markMin;   ///< NaN = none
  double m_markMax;

  double m_angle = -45.0;   ///< where the needle is
  double m_velocity = 0.0;
  double m_target = -45.0;   ///< where it should be
  QTimer m_timer;

  QPixmap m_static;
  bool m_staticDirty = true;
};
