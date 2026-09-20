// Tests for the analog meter widget: value-to-angle mapping, full-scale
// derivation, needle ballistics and a headless render smoke test.
//
// Set TEST_METER_DUMP=<dir> to also write the rendered variants as PNG for
// visual review.
#include <QApplication>
#include <QImage>
#include <QDir>
#include <QDebug>
#include <QSignalSpy>
#include <QTest>
#include <cmath>

#include "meterwid.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAILED:" << what;
    failed++;
  }
}

static bool near(double a, double b, double eps = 1e-6)
{
  return std::fabs(a - b) <= eps;
}

static QImage render(MeterWid &w, const QSize &size)
{
  w.resize(size);
  QImage img(size, QImage::Format_ARGB32_Premultiplied);
  img.fill(Qt::transparent);
  w.render(&img, QPoint(), QRegion(), QWidget::DrawChildren); // no window background
  return img;
}

// Brightness at a point on the needle's sweep for a given angle.
static int lumaAt(const QImage &img, const QPointF &pivot, double radius, double angleDeg)
{
  const double a = angleDeg * M_PI / 180.0;
  const QPoint pt(int(pivot.x() + radius * std::sin(a)), int(pivot.y() - radius * std::cos(a)));
  int sum = 0, n = 0;
  for (int dy = -1; dy <= 1; ++dy)
    for (int dx = -1; dx <= 1; ++dx)
    {
      const QPoint q = pt + QPoint(dx, dy);
      if (img.rect().contains(q))
      {
        sum += qGray(img.pixel(q));
        n++;
      }
    }
  return n ? sum / n : 0;
}

int main(int argc, char **argv)
{
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication app(argc, argv);

  // --- 1. angle mapping ---
  check(near(MeterWid::angleForValue(0.0, 4.0, false), -45.0), "unipolar: 0 -> -45 deg");
  check(near(MeterWid::angleForValue(4.0, 4.0, false), 45.0), "unipolar: FS -> +45 deg");
  check(near(MeterWid::angleForValue(2.0, 4.0, false), 0.0), "unipolar: FS/2 -> 0 deg");
  check(near(MeterWid::angleForValue(-0.2, 4.0, false), -49.5), "unipolar: -5% FS -> left stop");
  check(near(MeterWid::angleForValue(-1.0, 4.0, false), -49.5), "unipolar: far negative clamps at the stop");
  check(near(MeterWid::angleForValue(9.0, 4.0, false), 49.5), "unipolar: over range clamps at the right stop");
  check(near(MeterWid::angleForValue(0.0, 4.0, true), 0.0), "bipolar: 0 -> 0 deg");
  check(near(MeterWid::angleForValue(-4.0, 4.0, true), -45.0), "bipolar: -FS -> -45 deg");
  check(near(MeterWid::angleForValue(4.0, 4.0, true), 45.0), "bipolar: +FS -> +45 deg");
  check(near(MeterWid::angleForValue(1.0, 0.0, false), -45.0), "no full scale: rests at the left");
  {
    double last = -100;
    bool monotonic = true;
    for (double v = -0.5; v <= 4.5; v += 0.01)
    {
      const double a = MeterWid::angleForValue(v, 4.0, false);
      monotonic = monotonic && a >= last;
      last = a;
    }
    check(monotonic, "angle is monotonic in the value");
  }

  // --- 2. full scale from the reading string ---
  check(near(MeterWid::fullScaleFromReading("3.856", 4000), 4.0), "\"3.856\" @4000 -> 4");
  check(near(MeterWid::fullScaleFromReading(" 385.6", 4000), 400.0), "\"385.6\" @4000 -> 400");
  check(near(MeterWid::fullScaleFromReading("-1234", 2000), 2000.0), "\"-1234\" @2000 -> 2000");
  check(near(MeterWid::fullScaleFromReading("0.000", 6000), 6.0), "\"0.000\" @6000 -> 6");
  check(near(MeterWid::fullScaleFromReading("19.99", 2000), 20.0), "\"19.99\" @2000 -> 20");
  check(std::isnan(MeterWid::fullScaleFromReading("0.L", 4000)), "\"0.L\" is not a number");
  check(std::isnan(MeterWid::fullScaleFromReading("OL", 4000)), "\"OL\" is not a number");
  check(std::isnan(MeterWid::fullScaleFromReading("", 4000)), "empty string is not a number");
  check(std::isnan(MeterWid::fullScaleFromReading("3.856", 0)), "no counts, no scale");

  // --- 3. ballistics: converges, bounded overshoot, timer stops ---
  {
    MeterWid w;
    w.setFullScale(4.0);
    w.setScaleMode(MeterWid::Unipolar);
    w.setReading(0.0, "0.000", "V DC", false, false);
    QTest::qWait(1500);
    check(w.isSettled(), "needle settles at rest after 1.5 s");
    check(near(w.needleAngle(), -45.0, 0.1), "resting needle sits at -45 deg");

    w.setReading(2.0, "2.000", "V DC", false, false);
    check(!w.isSettled(), "a step starts the ballistics timer");
    double maxAngle = -100;
    for (int i = 0; i < 100 && !w.isSettled(); ++i)
    {
      QTest::qWait(16);
      maxAngle = qMax(maxAngle, w.needleAngle());
    }
    check(w.isSettled(), "needle settles within 1.6 s after a step");
    check(near(w.needleAngle(), 0.0, 0.1), "needle ends at the target");
    check(maxAngle < 0.0 + 4.5, "overshoot stays below 5% of full scale (4.5 deg)");
    check(maxAngle > 0.0, "slightly under-damped: some overshoot is expected");

    // without ballistics the needle jumps
    MeterStyle s = MeterStyle::dark();
    s.ballistics = false;
    w.setStyle(s);
    w.setReading(4.0, "4.000", "V DC", false, false);
    check(w.isSettled() && near(w.needleAngle(), 45.0), "ballistics off: needle jumps to the target");
  }

  // --- 4. auto scale mode latches to bipolar on a negative reading ---
  {
    MeterWid w;
    MeterStyle s = MeterStyle::dark();
    s.ballistics = false;
    w.setStyle(s);
    w.setFullScale(4.0);
    w.setScaleMode(MeterWid::Auto);
    w.setReading(1.0, "1.000", "V DC", false, false);
    check(!w.bipolar(), "auto: positive readings keep the unipolar scale");
    w.setReading(-0.1, "-0.100", "V DC", false, false);
    check(!w.bipolar(), "auto: a small negative value stays in the underswing zone");
    w.setReading(-1.0, "-1.000", "V DC", false, false);
    check(w.bipolar(), "auto: a clearly negative value switches to bipolar");
    w.setReading(1.0, "1.000", "V DC", false, false);
    check(w.bipolar(), "auto: bipolar is latched");
    w.reset();
    check(!w.bipolar(), "reset() releases the latch");
    w.setScaleMode(MeterWid::Bipolar);
    w.setReading(-1.0, "-1.000", "V DC", false, false);
    check(w.bipolar(), "explicit bipolar");
    w.setScaleMode(MeterWid::Unipolar);
    w.setReading(-1.0, "-1.000", "V DC", false, false);
    check(!w.bipolar(), "explicit unipolar ignores negative readings");
  }

  // --- 5. render smoke test ---
  {
    const QString dump = qEnvironmentVariable("TEST_METER_DUMP");
    const QSize size(480, 270);
    MeterWid w;
    MeterStyle s = MeterStyle::dark();
    s.ballistics = false;
    w.setStyle(s);
    w.setFullScale(4.0);
    w.setScaleMode(MeterWid::Unipolar);
    w.setReading(2.0, "2.000", "V DC", false, false);
    w.setPeak(3.5);
    QImage half = render(w, size);
    check(!half.isNull() && half.size() == size, "renders to an image");
    if (!dump.isEmpty()) half.save(QDir(dump).filePath("meter_dark_half.png"));

    // rough geometry, mirrors MeterWid::geometry()
    const double bezelW = qMax(4.0, (size.height() - 2) * 0.06);
    const double fh = size.height() - 2 - 2 * bezelW;
    const double fw = size.width() - 2 - 2 * bezelW;
    const QPointF pivot(size.width() / 2.0, 1 + bezelW + fh + fh * 0.02);
    const double radius = qMin(fh * 0.85, fw * 0.58);

    // the needle at 0 deg lights up the point straight above the pivot;
    // at -45 deg (value 0) it doesn't
    const int onNeedle = lumaAt(half, pivot, radius * 0.6, 0.0);
    w.setReading(0.0, "0.000", "V DC", false, false);
    QImage zero = render(w, size);
    const int offNeedle = lumaAt(zero, pivot, radius * 0.6, 0.0);
    check(onNeedle > offNeedle + 60, QString("needle is visible where it points (%1 vs %2)").arg(onNeedle).arg(offNeedle));
    if (!dump.isEmpty()) zero.save(QDir(dump).filePath("meter_dark_zero.png"));

    // corners outside the rounded bezel stay transparent
    check(qAlpha(half.pixel(0, 0)) == 0 && qAlpha(half.pixel(size.width() - 1, size.height() - 1)) == 0,
          "nothing is painted outside the bezel");

    // red zone has red pixels on the arc near the right end
    {
      bool red = false;
      for (double a = 40.0; a <= 45.0 && !red; a += 0.5)
      {
        const double ar = a * M_PI / 180.0;
        const QPoint pt(int(pivot.x() + radius * 0.95 * std::sin(ar)), int(pivot.y() - radius * 0.95 * std::cos(ar)));
        const QRgb px = half.pixel(pt);
        red = qRed(px) > 150 && qGreen(px) < 90 && qBlue(px) < 90;
      }
      check(red, "red zone is painted at the top end of the scale");
    }

    // min/max marks: a green triangle just outside the arc at the max value,
    // a red one at the min; both gone after reset()
    {
      auto markColor = [&](const QImage &img, double value, bool wantGreen)
      {
        const double a = MeterWid::angleForValue(value, 4.0, false) * M_PI / 180.0;
        for (double f = 0.90; f <= 0.98; f += 0.01)
        {
          const QPoint pt(int(pivot.x() + radius * f * std::sin(a)), int(pivot.y() - radius * f * std::cos(a)));
          if (!img.rect().contains(pt)) continue;
          const QRgb px = img.pixel(pt);
          if (wantGreen && qGreen(px) > 150 && qRed(px) < 120) return true;
          if (!wantGreen && qRed(px) > 150 && qGreen(px) < 110) return true;
        }
        return false;
      };
      w.setReading(2.0, "2.000", "V DC", false, false);
      w.setMinMax(1.0, 3.0);
      QImage marks = render(w, size);
      check(markColor(marks, 3.0, true), "green max mark at 3.0");
      check(markColor(marks, 1.0, false), "red min mark at 1.0");
      if (!dump.isEmpty()) marks.save(QDir(dump).filePath("meter_dark_marks.png"));
      w.reset();
      QImage cleared = render(w, size);
      check(!markColor(cleared, 3.0, true) && !markColor(cleared, 1.0, false), "reset() removes the marks");
      w.setMinMax(1.0, 3.0);
    }

    // other variants for the dump
    if (!dump.isEmpty())
    {
      w.setReading(5.0, "0.L", "V DC", true, false);
      render(w, size).save(QDir(dump).filePath("meter_dark_ol.png"));
      w.setReading(1.234, "1.234", "V DC", false, true);
      render(w, size).save(QDir(dump).filePath("meter_dark_hold.png"));
      w.setScaleMode(MeterWid::Bipolar);
      w.setReading(-1.5, "-1.500", "V DC", false, false);
      render(w, size).save(QDir(dump).filePath("meter_dark_bipolar.png"));
      w.setScaleMode(MeterWid::Unipolar);
      w.setFullScale(400.0);
      w.setReading(385.6, "385.6", "mV AC", false, false);
      w.setPeak(391.2);
      render(w, size).save(QDir(dump).filePath("meter_dark_400mv.png"));
      MeterStyle iv = MeterStyle::ivory();
      iv.ballistics = false;
      w.setStyle(iv);
      w.setFullScale(4.0);
      w.setReading(2.0, "2.000", "V DC", false, false);
      w.setPeak(3.5);
      render(w, size).save(QDir(dump).filePath("meter_ivory_half.png"));
      render(w, QSize(960, 540)).save(QDir(dump).filePath("meter_ivory_large.png"));
      render(w, QSize(240, 135)).save(QDir(dump).filePath("meter_ivory_small.png"));
    }
  }

  if (failed == 0)
    qInfo() << "All analog meter tests passed.";
  else
    qWarning() << failed << "analog meter test(s) failed.";
  return failed == 0 ? 0 : 1;
}
