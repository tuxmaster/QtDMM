//======================================================================
// File:		dmmgraph.cpp
// Author:	Matthias Toussaint
// Created:	Tue Apr 10 17:45:35 CEST 2001
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
#include <QPen>
#include <QRegularExpression>

#include "dmmgraph.h"
#include "settings.h"


DMMGraph::DMMGraph(QWidget *parent): DMMGraph(parent, Q_NULLPTR)
{
}
DMMGraph::DMMGraph(QWidget *parent, Settings *settings) :
  QWidget(parent),
  m_size(600),
  m_length(3600),
  m_scaleMin(0),
  m_scaleMax(0),
  m_autoScale(true),
  m_pointer(0),
  m_sampleTime(1),
  m_sampleLength(0),
  m_running(false),
  m_connected(false),
  m_mode(DMMGraph::Manual),
  m_lastValValid(false),
  m_dirty(false),
  m_alertUnsaved(true),
  m_externalStarted(false),
  m_crosshair(true),
  m_pointMode(Circle),
  m_intPointMode(Square),
  m_lineMode(Solid),
  m_intLineMode(NoLine),
  m_integrationScale(1.0),
  m_integrationThreshold(0.0),
  m_integrationOffset(0.0),
  m_includeZero(false)
{
  m_cfg = settings;
  // mt: changed from QArray to QVector
  m_array    = new QVector<double> (m_length);
  m_arrayInt = new QVector<double> (m_length);

  scrollbar = new QScrollBar(Qt::Horizontal, this);
  scrollbar->setGeometry(0, height() - 16, width(), 16);
  scrollbar->setTracking(true);
  scrollbar->setCursor(Qt::ArrowCursor);

  connect(scrollbar, &QScrollBar::valueChanged, this, [this](int) { updateXAxisRange(); });

  m_remainingLength = m_sampleLength;
  emitInfo();

  m_chart = new QChart();
  m_chart->legend()->hide();
  m_chart->setMargins(QMargins(4, 4, 4, 4));

  m_dataSeries = new QLineSeries();
  m_chart->addSeries(m_dataSeries);

  m_dataPoints = new QScatterSeries();
  m_chart->addSeries(m_dataPoints);

  m_xAxis = new QValueAxis();
  m_xAxis->setTitleText(tr("[sec]"));
  m_chart->addAxis(m_xAxis, Qt::AlignBottom);
  m_dataSeries->attachAxis(m_xAxis);
  m_dataPoints->attachAxis(m_xAxis);

  m_yAxis = new QValueAxis();
  m_chart->addAxis(m_yAxis, Qt::AlignLeft);
  m_dataSeries->attachAxis(m_yAxis);
  m_dataPoints->attachAxis(m_yAxis);

  updateSeriesAppearance();

  m_chartView = new QChartView(m_chart, this);
  m_chartView->setRenderHint(QPainter::Antialiasing);

  m_popup = new QMenu(this);
  connect(m_popup, SIGNAL(triggered(QAction *)), this, SLOT(popupSLOT(QAction *)));
}

DMMGraph::~DMMGraph()
{
  delete m_array;
  delete m_arrayInt;
}

void DMMGraph::print(QPrinter *prt, const QString &title, const QString &comment)
{
  if (!title.isEmpty())
    prt->setDocName(title);
  else
    prt->setDocName(tr("QtDMM: %1").arg(QDateTime::currentDateTime().toString()));

  prt->setCreator("QtDMM: (c) 2001 Matthias Toussaint");
  prt->setPrintProgram("QtDMM: (c) 2001 Matthias Toussaint");

  QPainter p;
  p.begin(prt);

  int w = prt->width();
  int h = prt->height();

  p.setFont(QFont("Helvetica", 16));

  QRect tRect = p.boundingRect(0, 0, w, h, Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, title);

  p.drawText(tRect, Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, title);

  p.setFont(QFont("Helvetica", 10));

  QFontMetrics fm = p.fontMetrics();
  int maxWidth = qMax(fm.horizontalAdvance(tr("Sampling start:")),
                      fm.horizontalAdvance(tr("Sampling resolution:")));
  int tHeight = fm.height();

  p.drawText(0, tRect.height() + 10, maxWidth, tHeight, Qt::AlignLeft | Qt::AlignVCenter,
             tr("Sampling start:"));
  p.drawText(maxWidth + 10, tRect.height() + 10, w - maxWidth - 10, tHeight, Qt::AlignLeft | Qt::AlignVCenter,
             m_graphStartDateTime.toString());

  p.drawText(0, tRect.height() + 10 + tHeight, maxWidth, tHeight, Qt::AlignLeft | Qt::AlignVCenter,
             tr("Sampling resolution:"));
  p.drawText(maxWidth + 10, tRect.height() + 10 + tHeight,
             w - maxWidth - 10, tHeight, Qt::AlignLeft | Qt::AlignVCenter,
             tr("%1 Seconds").arg(m_sampleTime));

  //p.setFont( QFont( "Helvetica", 10 ));

  QRect cRect = p.boundingRect(0, 0, w, h, Qt::AlignTop | Qt::AlignLeft | Qt::TextWordWrap, comment);

  p.drawText(0, tRect.height() + 20 + 2 * tHeight, w, cRect.height(),
             Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, comment);

  h -= tRect.height() + 30 + 2 * tHeight + cRect.height();

  QRectF chartRect(0, tRect.height() + 30 + 2 * tHeight + cRect.height(), w, h);
  m_chartView->render(&p, chartRect);

  p.end();
}

void DMMGraph::resizeEvent(QResizeEvent *)
{
  m_chartView->setGeometry(0, 0, width(), height() - 16);
  scrollbar->setGeometry(0, height() - 16, width(), 16);
}

void DMMGraph::rebuildSeries()
{
  QList<QPointF> points;
  points.reserve(m_pointer);

  double step = m_sampleTime / 10.0;
  for (int i = 0; i < m_pointer; i++)
    points.append(QPointF(i * step, (*m_array)[i]));

  m_dataSeries->replace(points);
  m_dataPoints->replace(points);
}

void DMMGraph::updateXAxisRange()
{
  double step = m_sampleTime / 10.0;
  int sv = qMax(0, scrollbar->value());

  m_xAxis->setRange(sv * step, (sv + qMax(1, m_size) - 1) * step);
}

void DMMGraph::updateSeriesAppearance()
{
  m_dataSeries->setPen(QPen(m_dataColor, m_lineWidth, penStyle(m_lineMode), Qt::RoundCap, Qt::RoundJoin));
  m_dataSeries->setVisible(m_lineMode != NoLine);

  QScatterSeries::MarkerShape shape = QScatterSeries::MarkerShapeCircle;
  int size = 7;

  switch (m_pointMode)
  {
    case NoPoint:                                                                     break;
    case Circle:      case LargeCircle:  shape = QScatterSeries::MarkerShapeCircle;    break;
    case Square:      case LargeSquare:  shape = QScatterSeries::MarkerShapeRectangle; break;
    // Qt Charts has no "X"/cross marker; approximate both Diamond and X with a
    // rotated square, the closest built-in shape available.
    case Diamond:     case LargeDiamond:
    case X:           case LargeX:       shape = QScatterSeries::MarkerShapeRotatedRectangle; break;
  }
  if (m_pointMode == LargeCircle || m_pointMode == LargeSquare ||
      m_pointMode == LargeDiamond || m_pointMode == LargeX)
    size = 11;

  m_dataPoints->setMarkerShape(shape);
  m_dataPoints->setMarkerSize(size);
  m_dataPoints->setColor(m_dataColor);
  m_dataPoints->setVisible(m_pointMode != NoPoint);
}

void DMMGraph::setGraphSize(int size, int length)
{
  m_size = static_cast<int>((static_cast<double>(size) / m_sampleTime * 10.));
  m_length = static_cast<int>((static_cast<double>(length) / m_sampleTime * 10. + 1));

  scrollbar->setMinimum(0);
  scrollbar->setMaximum(m_length - 1 - m_size);
  scrollbar->setSingleStep((m_size - 1) / 10);
  scrollbar->setPageStep(m_size);

  m_array->resize(m_length);
  m_arrayInt->resize(m_length);
  if (m_pointer >= m_length)
    m_pointer = m_length - 1;

  emitInfo();

  rebuildSeries();
  updateXAxisRange();
}

void DMMGraph::startSLOT()
{
  m_sampleCounter = 0;
  m_sum = 0;
  clearSLOT();
  m_running = true;

  m_remainingLength = m_sampleLength;
  m_pointer = 0;

  emitInfo();
  Q_EMIT running(true);

  m_graphStartDateTime = QDateTime::currentDateTime();
  m_externalStarted = false;

  (*m_arrayInt)[0] = 0;
}

void DMMGraph::stopSLOT()
{
  m_running = false;

  emitInfo();
  Q_EMIT running(false);
}

void DMMGraph::addValue(double val)
{
  if (m_mode == DMMGraph::Time && !m_running)
  {
    // we may miss the start due to aliasing otherwise
    int diff = m_startTime.secsTo(QTime::currentTime());

    if (diff >= 0 && diff < 2)
    {
      qApp->beep();
      startSLOT();
    }
  }

  if (m_mode == DMMGraph::Raising && !m_running)
  {
    if (m_lastValValid)
    {
      if (m_lastVal < m_raisingThreshold && val >= m_raisingThreshold)
      {
        qApp->beep();
        startSLOT();
      }
    }
  }

  if (m_mode == DMMGraph::Falling && !m_running)
  {
    if (m_lastValValid)
    {
      if (m_lastVal > m_fallingThreshold && val <= m_fallingThreshold)
      {
        qApp->beep();
        startSLOT();
      }
    }
  }

  if (!m_externalStarted && m_running && m_startExternal)
  {
    if (m_externalFalling &&
        m_lastVal > m_externalThreshold &&
        val <= m_externalThreshold)
    {
      m_externalStarted = true;
      Q_EMIT externalTriggered();
    }
    else if (!m_externalFalling &&
             m_lastVal < m_externalThreshold &&
             val >= m_externalThreshold)
    {
      m_externalStarted = true;
      Q_EMIT externalTriggered();
    }
  }

  m_lastValValid = true;
  m_lastVal = val;

  if (!m_running)
    return;

  m_sum += val;

  if (0 == m_sampleCounter)
  {
    m_dirty = true;

    if (!m_first)
      val = m_sum / static_cast<double>(m_sampleTime);
    m_first = false;
    m_sum = 0.0;

    bool shifted = m_pointer >= m_length;

    if (shifted)
    {
      for (int i = 1; i < m_length; i++)
      {
        (*m_array)[i - 1] = (*m_array)[i];
        (*m_arrayInt)[i - 1] = (*m_arrayInt)[i];
      }
      m_pointer = m_length - 1;
    }

    if (m_pointer > 0)
    {
      if (m_lastVal <= m_integrationThreshold)
        (*m_arrayInt)[m_pointer] = 0.0;
      else
        (*m_arrayInt)[m_pointer] = (*m_arrayInt)[m_pointer - 1] + val;
    }
    else
      (*m_arrayInt)[m_pointer] = qMax(val, m_integrationThreshold);

    (*m_array)[m_pointer++] = val;
    bool resFlag = false;

    if (m_autoScale)
    {
      resFlag = computeMinMax(val);
      //cerr << "val=" << val << " min=" << m_scaleMin << " max=" << m_scaleMax << endl;
      computeUnitFactor();
    }

    if (shifted)
      rebuildSeries();
    else
    {
      double x = (m_pointer - 1) * m_sampleTime / 10.0;
      m_dataSeries->append(x, val);
      m_dataPoints->append(x, val);
    }

    if (resFlag)
      m_yAxis->setRange(m_scaleMin, m_scaleMax);
  }

  m_sampleCounter++;
  m_remainingLength = qMax(0, m_remainingLength - 1);

  if (m_sampleCounter == m_sampleTime)
  {
    m_sampleCounter = 0;
    emitInfo();
  }

  if (0 == m_remainingLength && m_sampleLength != 0)
  {
    qApp->beep();
    stopSLOT();
    return;
  }
}

void DMMGraph::setUnit(const QString &unit)
{
  if (unit.left(1) == "n")
    m_unit = unit.mid(1);
  else if (unit.left(1) == "u")
    m_unit = unit.mid(1);
  else if (unit.left(1) == "m")
    m_unit = unit.mid(1);
  else if (unit.left(1) == "k")
    m_unit = unit.mid(1);
  else if (unit.left(1) == "M")
    m_unit = unit.mid(1);
  else
    m_unit = unit;

  m_yAxis->setTitleText(m_unit.isEmpty() ? QString() : QString("[%1]").arg(m_unit));
}

void DMMGraph::clearSLOT()
{
  m_pointer = 0;
  if (m_autoScale)
  {
    if (m_includeZero)
      m_scaleMin = m_scaleMax = 0;
    else
    {
      m_scaleMin =  1e40;
      m_scaleMax = -1e40;
    }
  }

  m_graphStartDateTime = QDateTime::currentDateTime();
  m_first = true;
  m_dirty = false;

  m_dataSeries->clear();
  m_dataPoints->clear();
}

void DMMGraph::emitInfo()
{
  int seconds = m_remainingLength / 10;

  int w = seconds / 60 / 60 / 24 / 7;
  int d = (seconds / 60 / 60 / 24) % 7;
  int h = (seconds / 60 / 60) % (24);
  int m = (seconds / 60) % 60;
  int s = seconds % 60;

  QString txt;

  if (w)
    txt = QString("%1/%2 - %3week%4 %5day&6 %7:%8:%9 - %10").arg(m_pointer).arg(m_length).arg(w).arg((w > 1 ? "s" : "")).arg(d).arg((d > 1 ? "s" : "")).arg(h).arg(m).arg(s)
          .arg(m_running ? tr("Sampling") : tr("Stopped"));
  else if (d)
    txt = QString("%1/%2 - %3day%4 %5:%6:%7 - %8").arg(m_pointer).arg(m_length).arg(d).arg((d > 1 ? "s" : "")).arg(h).arg(m).arg(s).arg(m_running ? tr("Sampling") : tr("Stopped"));
  else if (h)
    txt = QString("%1/%2 - %3:%4:%5 - %6").arg(m_pointer).arg(m_length).arg(h).arg(m).arg(s).arg(m_running ? tr("Sampling") : tr("Stopped"));
  else
    txt = QString("%1/%2 - %3:%4 - %5").arg(m_pointer).arg(m_length).arg(m).arg(s).arg(m_running ? tr("Sampling") : tr("Stopped"));
  Q_EMIT info(txt);
}

void DMMGraph::mousePressEvent(QMouseEvent *ev)
{
  // Cursor-drag (left button) and pan (middle button) are deferred to a later
  // step along with the threshold/cursor overlays they position; only the
  // right-click popup menu (self-contained, no painting-state dependency) is
  // kept working here.
  if (ev->button() != Qt::RightButton)
  {
    QWidget::mousePressEvent(ev);
    return;
  }

  QPoint globalPos(qRound(ev->globalPosition().x()), qRound(ev->globalPosition().y()));

  {
    m_popup->clear();

    if (m_connected)
    {
      QAction *action = new QAction(tr("Disconnect"), m_popup);
      action->setProperty("ID", IDDisconnect);
      m_popup->addAction(action);
      //m_popup->insertItem( tr("Disconnect"), IDDisconnect );
    }
    else
    {
      QAction *action = new QAction(tr("Connect"), m_popup);
      action->setProperty("ID", IDConnect);
      m_popup->addAction(action);
      //m_popup->insertItem( tr("Connect"), IDConnect );
    }
    m_popup->addSeparator();

    if (m_running)
    {
      QAction *action = new QAction(tr("Stop recorder"), m_popup);
      action->setProperty("ID", IDStopRecorder);
      m_popup->addAction(action);
      //m_popup->insertItem( tr("Stop recorder"), IDStopRecorder );
    }
    else
    {
      QAction *action = new QAction(tr("Start recorder"), m_popup);
      action->setProperty("ID", IDStartRecorder);
      m_popup->addAction(action);
      //m_popup->insertItem( tr("Start recorder"), IDStartRecorder );
    }
    QAction *action = new QAction(tr("Clear graph"), m_popup);
    action->setProperty("ID", IDClearGraph);
    m_popup->addAction(action);
    //m_popup->insertItem( tr("Clear graph"), IDClearGraph );
    m_popup->addSeparator();

    action = new QAction(tr("Configure..."), m_popup);
    action->setProperty("ID", IDConfigure);
    m_popup->addAction(action);
    //m_popup->insertItem( tr("Configure..."), IDConfigure );

    if (!m_running)
    {
      m_popup->addSeparator();
      QAction *action = new QAction(tr("Export data..."), m_popup);
      action->setProperty("ID", IDExportData);
      m_popup->addAction(action);
      //m_popup->insertItem( tr("Export data..."), IDExportData );
      action = new QAction(tr("Import data..."), m_popup);
      action->setProperty("ID", IDImportData);
      m_popup->addAction(action);
      //m_popup->insertItem( tr("Import data..."), IDImportData );
    }

    m_popup->popup(globalPos);
  }
}

bool DMMGraph::exportDataSLOT()
{
  QDir path;
  QFileInfo fileInfo(m_cfg->getString("QtDMM/LastUsesPath"));
  QStringList validSuffixes = { "csv" };
  QString fnSuffix = validSuffixes.contains(fileInfo.suffix()) ? fileInfo.suffix() : "csv";
  QString fn = fileInfo.baseName().isEmpty() ? "untitled.csv" : fileInfo.absolutePath() + "/untitled." + fnSuffix;
  fn = QFileDialog::getSaveFileName(this, tr("Export data"), fn, "CSV (*.csv)");

  if (fn.isNull())
    return false;

  return exportCsvFile(fn);
}

bool DMMGraph::exportCsvFile(const QString &fileName)
{
  if (m_pointer <= 0)
    return false;

  m_cfg->setString("QtDMM/LastUsesPath", QDir().absoluteFilePath(fileName));

  QFile file(fileName);
  file.open(QIODevice::WriteOnly);

  QTextStream ts(&file);
  QString line = QString("timestamp;time (s);value;unit\n");
  ts << line;

  for (int i = 0; i < m_pointer; i++)
  {
    QDateTime dt = m_graphStartDateTime.addMSecs(i * m_sampleTime * 100);
    //timestamp: ISO8601
    double deltaTime = (dt.toMSecsSinceEpoch()-m_graphStartDateTime.toMSecsSinceEpoch())/1000.0f;
    line = QString("%1;%2;%3;%4\n").arg(dt.toString("yyyy-MM-ddTHH:mm:ss,zzz")).arg(deltaTime).arg((*m_array)[i], 0, 'f').arg(m_unit);
    ts << line;
  }
  m_dirty = false;

  file.close();

  return true;
}


void DMMGraph::importDataSLOT()
{
  if (m_dirty && m_alertUnsaved)
  {
    QMessageBox question;
    question.setWindowTitle(tr("QtDMM: Unsaved data"));
    question.setText(tr("<font size=+2><b>Unsaved data</b></font><p>"
                        "Importing data will overwrite your measured data"
                        "<p>Do you want to export your unsaved data first?"));
    question.setIcon(QMessageBox::Question);

    // Standard-Buttons
    question.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    question.setDefaultButton(QMessageBox::Yes);
    question.setEscapeButton(QMessageBox::Cancel);

    QAbstractButton *yesButton = question.button(QMessageBox::Yes);
    if (yesButton)
      yesButton->setText(tr("Export data first"));

    QAbstractButton *noButton = question.button(QMessageBox::No);
    if (noButton)
      noButton->setText(tr("Import & overwrite data"));

    switch (question.exec())
    {
      case QMessageBox::Yes:
        exportDataSLOT();
        return;
      case QMessageBox::Cancel:
        return;
    }
  }
  QString fn = QFileDialog::getOpenFileName(this, tr("Import data"), m_cfg->getString("QtDMM/LastUsesPath", tr("CSV (*.csv);;All files (*)")));

  if (!fn.isNull())
    importCsvFile(fn);
}

bool DMMGraph::importCsvFile(const QString &fileName)
{
  QDir path;
  int sample = 0;

  QDateTime graphEnd;

  m_cfg->setString("QtDMM/LastUsesPath", path.absoluteFilePath(fileName));
  // First pass -> figure out size and sample time
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
  {
    Q_EMIT error(tr("Cannot open file."));
    return false;
  }

  QTextStream ts(&file);

  QString line = ts.readLine();
  if (line.isNull())
  {
    Q_EMIT error(tr("Oops! Seems not to be a valid file"));
    file.close();
    return false;
  }

  // skip CSV-Header
  if (line.startsWith("timestamp"))
  {
    line = ts.readLine();
    if (line.isNull())
    {
      Q_EMIT error(tr("File contains only header"));
      file.close();
      return false;
    }
  }

  QRegularExpression reLegacy(
    R"(^(?<day>\d{2})\.(?<month>\d{2})\.(?<year>\d{4})\t(?<hour>\d{2}):(?<minute>\d{2}):(?<second>\d{2})(?::(?<ms>\d{1,3}))?\t(?<value>-?\d+(?:\.\d+)?|nan)\t(?<unit>.*)$)",
    QRegularExpression::CaseInsensitiveOption
  );

  QRegularExpression reCSV(
    R"(^(?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2})T(?<hour>\d{2}):(?<minute>\d{2}):(?<second>\d{2})[,\.](?<ms>\d{1,3});(?<delta>-?\d+(?:\.\d+)?|nan);(?<value>-?\d+(?:\.\d+)?|nan);(?<unit>.*)$)",
    QRegularExpression::CaseInsensitiveOption
  );

  // detect format
  bool isLegacy = reLegacy.match(line).hasMatch();

  //(*m_array).clear();
  QVector<double> values;
  QRegularExpressionMatch match;
  do
  {
    if (!line.trimmed().isEmpty())
    {
      match = isLegacy ? reLegacy.match(line) : reCSV.match(line);

      if (!match.hasMatch())
      {
        qInfo() << line;
        Q_EMIT error(tr("Oops! Seems not to be a valid file"));
        file.close();
        return false;
      }

      QDate valueDate(
        match.captured("year").toInt(),
        match.captured("month").toInt(),
        match.captured("day").toInt()
      );

      QTime valueTime(
        match.captured("hour").toInt(),
        match.captured("minute").toInt(),
        match.captured("second").toInt(),
        match.captured("ms").toInt()
      );

      if (values.isEmpty())
      {
        setUnit(match.captured("unit"));
        m_graphStartDateTime = QDateTime(valueDate, valueTime);
      }

      graphEnd = QDateTime(valueDate, valueTime);
      values << (match.captured("value") == "nan" ? 0.0f : match.captured("value").toDouble());
    }

    line = ts.readLine();
  }
  while (!line.isNull());
  file.close();

  sample = m_graphStartDateTime.secsTo(graphEnd);

  int cnt = values.size();
  m_sampleTime = (sample / (cnt > 1 ? cnt - 1 : 1))/10;
  if (m_sampleTime<1) m_sampleTime=1;
  qInfo() << m_graphStartDateTime.secsTo( graphEnd ) << m_sampleTime;

  int size = m_size * m_sampleTime;

  if (cnt > 1)
  {
    Q_EMIT sampleTime(m_sampleTime);
    m_sampleTime = (sample / (cnt - 1))/10;
  }
  if (m_sampleTime<1) m_sampleTime=1;
  qInfo() << sample << cnt << m_sampleTime << m_graphStartDateTime.secsTo( graphEnd );

  m_scaleMin =  1e40;
  m_scaleMax = -1e40;

  // TEST
  setGraphSize(size, cnt * m_sampleTime);

  for(int i=0; i<values.size(); i++)
    (*m_array)[i] = values[i];

  m_sampleCounter = m_pointer = cnt;
  setScale(true, true, 0, 0);

  m_dirty = false;

  Q_EMIT error(fileName);

  update();

  computeUnitFactor();

  // TEST
  Q_EMIT graphSize(size, cnt * m_sampleTime);

  return true;
}

/*

void DMMGraph::importDataSLOT()
{
  if (m_dirty && m_alertUnsaved)
  {
    QMessageBox question;
    question.setWindowTitle(tr("QtDMM: Unsaved data"));
    question.setText(tr("<font size=+2><b>Unsaved data</b></font><p>"
                        "Importing data will overwrite your measured data"
                        "<p>Do you want to export your unsaved data first?"));
    question.setIcon(QMessageBox::Information);
    question.setIconPixmap(QPixmap(":/Symbols/icon.xpm"));

    // Standard-Buttons
    question.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    question.setDefaultButton(QMessageBox::Yes);
    question.setEscapeButton(QMessageBox::Cancel);

    QAbstractButton *yesButton = question.button(QMessageBox::Yes);
    if (yesButton)
      yesButton->setText(tr("Export data first"));

    QAbstractButton *noButton = question.button(QMessageBox::No);
    if (noButton)
      noButton->setText(tr("Import & overwrite data"));

    switch (question.exec())
    {
      case QMessageBox::Yes:
        exportDataSLOT();
        return;
      case QMessageBox::Cancel:
        return;
    }
  }
  QDir path;
  QString fn = QFileDialog::getOpenFileName(this, tr("Import data"), m_cfg->getString("QtDMM/LastUsesPath", ""));

  int cnt = 0;
  int sample = 0;

  QDateTime graphEnd;

  if (!fn.isNull())
  {
    m_cfg->setString("QtDMM/LastUsesPath", path.absoluteFilePath(fn));
    // First pass -> figure out size and sample time
    QFile file(fn);

    QStringList token;
    QStringList dateToken;
    QStringList timeToken;
    QStringList timeParts;

    if (file.open(QIODevice::ReadOnly))
    {
      QTextStream ts(&file);

      QString line = ts.readLine();

      if (!line.isNull())
      {
        QRegularExpression reLegacy(
          R"(^(?<day>\d{2})\.(?<month>\d{2})\.(?<year>\d{4})\t(?<hour>\d{2}):(?<minute>\d{2}):(?<second>\d{2}):(?<ms>\d{1,3})\t(?<value>-?\d+(?:\.\d+)?)\t.*$)"
        );
        QRegularExpression reCSV(
          R"(^(?<year>\d{4})-(?<month>\d{2})-(?<day>\d{2})T(?<hour>\d{2}):(?<minute>\d{2}):(?<second>\d{2}),(?<ms>\d{1,3});(?<delta>-?\d+(?:\.\d+)?);(?<value>-?\d+(?:\.\d+)?);(?<unit>.+)$)"
        );
        if (!reCSV.match(line).hasMatch())
        {
          Q_EMIT error(tr("Oops! Seems not to be a valid file"));

          return;
        }

        token = line.split("\t");
        dateToken = token[0].split(".");
        timeParts = token[1].split('.');
        timeToken = timeParts[0].split(':');
        timeToken.append(timeParts.value(1, "0"));

        QTime startTime = QTime(timeToken[0].toInt(),
                                timeToken[1].toInt(),
                                timeToken[2].toInt(),
                                timeToken[3].toInt());

        QDate startDate = QDate(dateToken[2].toInt(),
                                dateToken[1].toInt(),
                                dateToken[0].toInt());

        m_graphStartDateTime = QDateTime(startDate, startTime);
        graphEnd = QDateTime(startDate, startTime);

        setUnit(line.mid(27, 3));

        cnt++;

        while (!ts.atEnd())
        {
          line = ts.readLine();

          if (!line.isEmpty())
          {
            token = line.split("\t");
            dateToken = token[0].split(".");

            timeParts = token[1].split('.');
            timeToken = timeParts[0].split(':');
            timeToken.append(timeParts.value(1, "0"));

            QTime nowTime = QTime(timeToken[0].toInt(),
                                  timeToken[1].toInt(),
                                  timeToken[2].toInt(),
                                  timeToken[3].toInt());
            QDate nowDate = QDate(dateToken[2].toInt(),
                                  dateToken[1].toInt(),
                                  dateToken[0].toInt());

            sample += QDateTime(startDate, startTime).secsTo(QDateTime(nowDate, nowTime));

            startTime = nowTime;
            startDate = nowDate;

            graphEnd = QDateTime(startDate, startTime);

            cnt++;
          }
        }
      }
      file.close();
    }

    //std::cerr << "sample=" << sample << std::endl;

    m_sampleTime = sample / (cnt > 1 ? cnt - 1 : 1); //m_graphStartDateTime.secsTo( graphEnd );

    int size = m_size * m_sampleTime;
    //int length = (m_length-1)*m_sampleTime;

    if (cnt > 1)
    {
      //if (sample/(cnt-1) != m_sampleTime)
      {
        Q_EMIT sampleTime(m_sampleTime);
      }
      m_sampleTime = sample / (cnt - 1);
    }

    //   if (cnt*m_sampleTime > length)
    // {
    //   if (size > cnt*m_sampleTime) size = cnt*m_sampleTime;
    //   emit graphSize( size, cnt*m_sampleTime );
    //   setGraphSize( size, cnt*m_sampleTime );
    // }

    m_scaleMin =  1e40;
    m_scaleMax = -1e40;

    if (file.open(QIODevice::ReadOnly))
    {
      // TEST
      setGraphSize(size, cnt * m_sampleTime);

      int i = 0;

      QTextStream ts(&file);
      QString line;

      while (!(line = ts.readLine()).isNull())
      {
        if (!line.isEmpty())
        {
          token = line.split("\t");
          (*m_array)[i++] = token[2].toDouble();
        }
      }

      m_sampleCounter = m_pointer = cnt;
      setScale(true, true, 0, 0);

      file.close();
      m_dirty = false;

      Q_EMIT error(fn);

      update();

      computeUnitFactor();
    }

//std::cerr << "min=" << m_scaleMin << " max=" << m_scaleMax << std::endl;
//std::cerr << "factor=" << m_factor << " prefix=" <<
//  m_prefix.latin1() << " unit=" << m_unit.latin1() << std::endl;

//std::cerr << "size=" << size << "cnt*m_sampleTime=" << cnt*m_sampleTime << std::endl;
    // TEST
    Q_EMIT graphSize(size, cnt * m_sampleTime);
  }
}

*/



void DMMGraph::setThresholds(double falling, double raising)
{
  m_fallingThreshold = falling;
  m_raisingThreshold = raising;
}

void DMMGraph::setScale(bool autoScale, bool includeZero, double min, double max)
{
  m_autoScale = autoScale;
  m_includeZero = includeZero;

  if (!autoScale)
  {
    m_scaleMin = min;
    m_scaleMax = max;

    computeUnitFactor();
  }
  else
  {
    if (m_includeZero)
      m_scaleMin = m_scaleMax = 0;
    else
    {
      m_scaleMin =  1e40;
      m_scaleMax = -1e40;
    }


    for (int i = 0; i < m_pointer; i++)
    {
      const double val = (*m_array)[i];
      computeMinMax(val);
    }

    computeUnitFactor();
  }

  m_yAxis->setRange(m_scaleMin, m_scaleMax);
}

bool DMMGraph::computeMinMax(double val)
{
  bool ret = false;

  if (val > m_scaleMax * 0.95)
  {
    if (val > 0)
      m_scaleMax = val * 1.2;
    else
      m_scaleMax = val / 1.2;

    ret = true;
  }
  if (val < m_scaleMin * 0.95)
  {
    if (val > 0)
      m_scaleMin = val / 1.2;
    else
      m_scaleMin = val * 1.2;

    ret = true;
  }

  return ret;
}

void DMMGraph::setColors(const QColor &bg, const QColor &grid,
                         const QColor &data, const QColor &cursor,
                         const QColor &start, const QColor &external,
                         const QColor &integration, const QColor &intThreshold)
{
  m_bgColor           = bg;
  m_gridColor         = grid;
  m_dataColor         = data;
  m_cursorColor       = cursor;
  m_startColor        = start;
  m_externalColor     = external;
  m_intColor          = integration;
  m_intThresholdColor = intThreshold;

  m_chart->setBackgroundBrush(m_bgColor);
  m_xAxis->setGridLineColor(m_gridColor);
  m_yAxis->setGridLineColor(m_gridColor);
  updateSeriesAppearance();
}

void DMMGraph::setLineStyle(int lineMode, int pointMode, int intLineMode, int intPointMode)
{
  m_lineMode = static_cast<LineMode>(lineMode);
  m_pointMode = static_cast<PointMode>(pointMode);
  m_intLineMode = static_cast<LineMode>(intLineMode);
  m_intPointMode = static_cast<PointMode>(intPointMode);

  updateSeriesAppearance();
}

void DMMGraph::setLine(int d, int i)
{
  m_lineWidth    = d;
  m_intLineWidth = i;

  updateSeriesAppearance();
}

void DMMGraph::setExternal(bool on, bool falling, double threshold)
{
  m_startExternal = on;
  m_externalFalling = falling;
  m_externalThreshold = threshold;
}

Qt::PenStyle DMMGraph::penStyle(LineMode mode)
{
  switch (mode)
  {
    case NoLine:
      return Qt::NoPen;
    case Solid:
      return Qt::SolidLine;
    case Dot:
      return Qt::DotLine;
  }
  return Qt::SolidLine;
}

void DMMGraph::setIntegration(bool showInt, double sc, double th, double off)
{
  m_showIntegration = showInt;
  m_integrationScale = sc;
  m_integrationThreshold = th;
  m_integrationOffset = off;
}

void DMMGraph::computeUnitFactor()
{
  m_factor = 1.;
  m_prefix = "";

  if (m_unit == "C" || m_unit == "%") return;

  if (qMax(fabs(m_scaleMax * m_factor), fabs(m_scaleMin * m_factor)) > 1000)
  {
    m_factor /= 1000.;
    m_prefix = "k";
  }
  if (qMax(fabs(m_scaleMax * m_factor), fabs(m_scaleMin * m_factor)) > 1000)
  {
    m_factor /= 1000.;
    m_prefix = "M";
  }
  if (qMax(fabs(m_scaleMax * m_factor), fabs(m_scaleMin * m_factor)) > 1000)
  {
    m_factor /= 1000.;
    m_prefix = "G";
  }
  if (qMax(fabs(m_scaleMax * m_factor), fabs(m_scaleMin * m_factor)) < 1)
  {
    m_factor *= 1000.;
    m_prefix = "m";
  }
  if (qMax(fabs(m_scaleMax * m_factor), fabs(m_scaleMin * m_factor)) < 1)
  {
    m_factor *= 1000.;
    m_prefix = "µ";
  }
  if (qMax(fabs(m_scaleMax * m_factor), fabs(m_scaleMin * m_factor)) < 1)
  {
    m_factor *= 1000.;
    m_prefix = "n";
  }
  if (qMax(fabs(m_scaleMax * m_factor), fabs(m_scaleMin * m_factor)) < 1)
  {
    m_factor *= 1000.;
    m_prefix = "p";
  }
}

void DMMGraph::popupSLOT(QAction *action)
{
  switch (action->property("ID").toInt())
  {
    case IDConnect:
      Q_EMIT connectDMM(true);
      break;
    case IDDisconnect:
      Q_EMIT connectDMM(false);
      break;
    case IDStopRecorder:
      stopSLOT();
      break;
    case IDStartRecorder:
      startSLOT();
      break;
    case IDClearGraph:
      clearSLOT();
      break;
    case IDConfigure:
      Q_EMIT configure();
      break;
    case IDExportData:
      Q_EMIT exportData();
      break;
    case IDImportData:
      Q_EMIT importData();
      break;
  }
}
