// Tests for the digital display: seven-segment glyph table and a headless
// render check. TEST_DISPLAY_DUMP=<dir> writes PNGs for visual review.
#include <QApplication>
#include <QImage>
#include <QDir>
#include <QDebug>

#include "displaywid.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAILED:" << what;
    failed++;
  }
}

static QImage render(DisplayWid &w, const QSize &size)
{
  w.resize(size);
  QImage img(size, QImage::Format_ARGB32_Premultiplied);
  img.fill(Qt::transparent);
  w.render(&img, QPoint(), QRegion(), QWidget::DrawChildren);
  return img;
}

static int darkPixels(const QImage &img)
{
  int n = 0;
  for (int y = 0; y < img.height(); ++y)
    for (int x = 0; x < img.width(); ++x)
      if (qAlpha(img.pixel(x, y)) > 0 && qGray(img.pixel(x, y)) < 60)
        n++;
  return n;
}

int main(int argc, char **argv)
{
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
    qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication app(argc, argv);

  // --- 1. glyphs: every digit, the letters meters send (OL, Err, dIodE, bAt) ---
  check(DisplayWid::segmentsFor('8') == 0x7f, "8 lights all seven segments");
  check(DisplayWid::segmentsFor('1') == 0x06, "1 lights b and c");
  check(DisplayWid::segmentsFor('-') == 0x40, "minus is the middle segment");
  check(DisplayWid::segmentsFor(' ') == 0, "space is blank");
  for (QChar ch : QString("0123456789abcdefhlnoprstuy"))
    check(DisplayWid::segmentsFor(ch) != 0, QString("glyph for '%1'").arg(ch));
  check(DisplayWid::segmentsFor('L') == DisplayWid::segmentsFor('l'), "letters are case-insensitive");
  check(DisplayWid::segmentsFor('0') != DisplayWid::segmentsFor('o'), "0 and o differ (o is lower case)");

  // --- 2. render: a value paints more dark pixels than an empty display ---
  const QString dump = qEnvironmentVariable("TEST_DISPLAY_DUMP");
  const QSize size(520, 200);
  DisplayWid w;
  w.setDisplayMode(4000, true, true, 1);
  QImage empty = render(w, size);
  check(!empty.isNull(), "renders empty");
  if (!dump.isEmpty()) empty.save(QDir(dump).filePath("display_empty.png"));

  w.setValue(0, "-3.856");
  w.setUnit(0, "mV");
  w.setMode(0, "DC");
  w.setAuto(true);
  w.setShowBar(true);
  w.setMinValue("-3.900");
  w.setMinUnit("mV");
  w.setMaxValue("0.412");
  w.setMaxUnit("mV");
  QImage value = render(w, size);
  check(darkPixels(value) > darkPixels(empty) + 500, "lit segments add dark pixels");
  if (!dump.isEmpty()) value.save(QDir(dump).filePath("display_value.png"));
  check(qAlpha(value.pixel(0, 0)) == 0, "corners outside the bezel stay transparent");

  // --- 3. aspect clamp: a strip-shaped widget keeps a panel of sane shape ---
  {
    QImage wide = render(w, QSize(1400, 200));
    // far left/right of a very wide widget is outside the panel
    check(qAlpha(wide.pixel(20, 100)) == 0 && qAlpha(wide.pixel(1379, 100)) == 0,
          "very wide widget: panel does not stretch to the edges");
    check(qAlpha(wide.pixel(700, 100)) != 0, "very wide widget: panel is centred");
    QImage tall = render(w, QSize(300, 400));
    check(qAlpha(tall.pixel(150, 10)) == 0 && qAlpha(tall.pixel(150, 389)) == 0,
          "very tall widget: panel does not stretch vertically");
  }

  if (!dump.isEmpty())
  {
    w.setValue(0, "0.L");
    w.setUnit(0, "Ohm");
    w.setMode(0, "OH");
    w.setHold(true);
    render(w, size).save(QDir(dump).filePath("display_ol.png"));
    w.setHold(false);
    w.setValue(0, "1.234");
    w.setUnit(0, "V");
    w.setMode(0, "DI");
    render(w, size).save(QDir(dump).filePath("display_diode.png"));
    w.setDisplayMode(22000, true, true, 2);
    w.setValue(0, "12.345");
    w.setUnit(0, "V");
    w.setMode(0, "AC");
    w.setValue(1, "50.02");
    w.setUnit(1, "Hz");
    render(w, size).save(QDir(dump).filePath("display_two_values.png"));
    w.setDisplayMode(4000, false, false, 1);
    w.setValue(0, "3.856");
    w.setUnit(0, "kOhm");
    w.setMode(0, "OH");
    render(w, QSize(300, 110)).save(QDir(dump).filePath("display_small_plain.png"));
    render(w, QSize(1000, 400)).save(QDir(dump).filePath("display_large_plain.png"));
    w.setFaceColor(QColor(0x20, 0x30, 0x40));
    render(w, size).save(QDir(dump).filePath("display_dark_face.png"));
    w.setFaceColor(QColor(0xb6, 0xcf, 0xa4));
    w.setDisplayMode(4000, true, true, 1);
    render(w, QSize(1400, 200)).save(QDir(dump).filePath("display_very_wide.png"));
    render(w, QSize(300, 400)).save(QDir(dump).filePath("display_very_tall.png"));
  }

  if (failed == 0)
    qInfo() << "All digital display tests passed.";
  else
    qWarning() << failed << "digital display test(s) failed.";
  return failed == 0 ? 0 : 1;
}
