//======================================================================
// File:		mainwid.cpp
// Author:	Matthias Toussaint
// Created:	Tue Apr 10 17:29:01 CEST 2001
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
#include <QPrinter>
#include <iostream>
#include <cmath>

#include "mainwid.h"
#include "dmmgraph.h"
#include "configdlg.h"
#include "dmm.h"
#include "displaywid.h"
#include "meterwid.h"
#include "readinglog.h"
#include "alarm.h"
#include "alarmbar.h"
#include "scpiserver.h"
#include "mdnsresponder.h"
#include <QHostInfo>
#include "siprefix.h"
#include "engnumbervalidator.h"
#include "tipdlg.h"
#include "settings.h"
#include "instancesdlg.h"
#include "sharedstatemanager.h"



MainWid::MainWid(QString instance_id, QString config_path, QWidget *parent) :  QFrame(parent),
  m_min(1.0E20),
  m_max(-1.0E20),
  m_display(0),
  m_meter(nullptr),
  m_stateMgr(nullptr),
  m_dval(0.0),
  m_tipDlg(0)
{
  setupUi(this);
  setWindowIcon(QPixmap(":/Symbols/icon.xpm"));

  m_dmm = new DMM(this);
  m_external = new QProcess(this);

  m_instanceId = instance_id;
  m_settings  = new Settings(instance_id, config_path, this);
  m_configDlg = new ConfigDlg(m_settings, this);
  m_configDlg->hide();
  m_configDlg->readPrinter(&m_printer);

  m_printDlg = new qtdmm::PrintDlg(this);
  m_printDlg->hide();

  m_instancesDlg = new InstancesDlg(m_settings, instance_id, config_path,this);

  connect(m_instancesDlg, SIGNAL(writeState(const QString &)), parent, SLOT(sendStateSLOT(const QString &)));
  connect(this, SIGNAL(sendState(const QString &)), parent, SLOT(sendStateSLOT(const QString &)));
  connect(m_dmm, SIGNAL(value(double, const QString &, const QString &, const QString &, const QString &, bool, bool, int)),
          this,  SLOT(valueSLOT(double, const QString &, const QString &, const QString &, const QString &, bool, bool, int)));
  connect(m_dmm, SIGNAL(error(const QString &)), this, SIGNAL(error(const QString &)));
  connect(ui_graph, SIGNAL(info(const QString &)), this, SIGNAL(info(const QString &)));
  connect(ui_graph, SIGNAL(error(const QString &)), this, SIGNAL(error(const QString &)));
  connect(ui_graph, SIGNAL(running(bool)), this, SLOT(runningSLOT(bool)));
  connect(m_configDlg, SIGNAL(accepted()), this, SLOT(applySLOT()));
  // Apply: take the settings over while the dialog stays open. Through a
  // lambda so sender() is not the dialog and applySLOT() does not reconnect
  // the meter, which only happens when the dialog closes.
  connect(m_configDlg, &ConfigDlg::applied, this, [this]() { applySLOT(); });
  connect(m_configDlg, SIGNAL(zoomed()), this, SLOT(zoomedSLOT()));
  connect(m_configDlg, SIGNAL(rejected()), this, SLOT(rejectSLOT()));
  connect(ui_graph, SIGNAL(sampleTime(int)), m_configDlg, SLOT(setSampleTimeSLOT(int)));
  connect(ui_graph, SIGNAL(graphSize(int, int)), m_configDlg, SLOT(setGraphSizeSLOT(int, int)));
  connect(ui_graph, SIGNAL(externalTriggered()), this, SLOT(startExternalSLOT()));
  connect(m_external, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(exitedSLOT()));
  connect(ui_graph, SIGNAL(configure()), this, SLOT(configSLOT()));
  connect(ui_graph, SIGNAL(exportData()), this, SLOT(exportSLOT()));
  connect(ui_graph, SIGNAL(importData()), this, SLOT(importSLOT()));

  connect(ui_graph, SIGNAL(connectDMM(bool)), this, SIGNAL(connectDMM(bool)));

  connect(ui_graph, SIGNAL(zoomOut(double)), m_configDlg, SLOT(zoomOutSLOT(double)));
  connect(ui_graph, SIGNAL(zoomIn(double)), m_configDlg, SLOT(zoomInSLOT(double)));
  connect(ui_graph, SIGNAL(zoomFit()), m_configDlg, SLOT(zoomFitSLOT()));
  connect(ui_graph, SIGNAL(thresholdChanged(DMMGraph::CursorMode, double)),
          m_configDlg, SLOT(thresholdChangedSLOT(DMMGraph::CursorMode, double)));

  ui_graph->setSettings(m_settings);

  // Not resetSLOT() here: it writes to m_display, which MainWin only hands us
  // later via setDisplay(). The min/max seeds live in the init list instead.
  m_settings->save();
  Q_EMIT sendState("UPDATE_INSTANCES_"+QString::number(QDateTime::currentMSecsSinceEpoch()));
  startTimer(100);

  // alarms: the manager judges every reading, the banner sits above the graph
  m_alarms = new AlarmManager(this);
  m_alarmBar = new AlarmBar(this);
  if (auto *box = qobject_cast<QBoxLayout *>(layout()))
    box->insertWidget(0, m_alarmBar);
  connect(m_alarms, &AlarmManager::raised, this, &MainWid::alarmRaised);
  connect(m_alarms, &AlarmManager::cleared, this, &MainWid::alarmCleared);
  connect(m_alarmBar, &AlarmBar::acknowledged, this, [this]
  {
    m_alarms->acknowledgeAll();
    updateAlarmBar();
  });
  m_alarms->setAlarms(m_configDlg->alarms());

  // SCPI server: the meter as a network instrument (applyScpi() starts it)
  m_scpi = new ScpiServer(this);
  m_mdns = new MdnsResponder(this);
  connect(m_scpi, &ScpiServer::startRecording, this, &MainWid::startSLOT);
  connect(m_scpi, &ScpiServer::stopRecording, this, &MainWid::stopSLOT);
  connect(m_scpi, &ScpiServer::connectRequested, this, [this](bool on)
  {
    if (on == m_dmm->isOpen())
      return;
    Q_EMIT setConnect(on);
    Q_EMIT connectDMM(on);
    connectSLOT(on);
  });
  connect(m_scpi, &ScpiServer::clientsChanged, this, [this](int) { updateScpiStatus(); });
  // HCOPy:SDUMp:DATA? - the main window as the "instrument screen"
  m_scpi->setScreenshotSource([this](const QByteArray &format) -> QByteArray
  {
    QWidget *top = window() ? window() : this;
    const QPixmap shot = top->grab();
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    if (!shot.save(&buffer, format.constData()))
      return {};
    return bytes;
  });

  if (m_configDlg->showTip())
    showTipsSLOT();
}

void MainWid::setConsoleLogging(bool on)
{
  m_dmm->setConsoleLogging(on);
}

void MainWid::setDisplay(DisplayWid *display)
{
  m_display = display;
}

void MainWid::setMeter(MeterWid *meter)
{
  m_meter = meter;
}

bool MainWid::closeWin()
{
  m_dmm->close();
  m_configDlg->setWinRect(parentRect());
  m_configDlg->on_ui_buttonBox_accepted();

  Q_EMIT setConnect(false);

  if (ui_graph->dirty() && m_configDlg->alertUnsavedData())
  {
    QMessageBox question;
    question.setWindowTitle(tr("QtDMM: Unsaved data"));
    question.setText(tr("<font size=+2><b>Unsaved data</b></font><p>"
                        "You still have unsaved measured data in memory."
                        " If you quit now it will be lost."
                        "<p>Do you want to export your unsaved data first?"));
    question.setIcon(QMessageBox::Information);
    question.setIconPixmap(QPixmap(":/Symbols/icon.xpm"));

    // Set standard buttons
    question.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    question.setDefaultButton(QMessageBox::Yes);
    question.setEscapeButton(QMessageBox::Cancel);

    // Set custom button texts
    QAbstractButton *yesButton = question.button(QMessageBox::Yes);
    if (yesButton)
      yesButton->setText(tr("Export data first"));

    QAbstractButton *noButton = question.button(QMessageBox::No);
    if (noButton)
      noButton->setText(tr("Quit without saving"));

    switch (question.exec())
    {
      case QMessageBox::Yes:
        return ui_graph->exportDataSLOT();

      case QMessageBox::No:
        break;

      case QMessageBox::Cancel:
        return false;
    }
  }

  return true;
}

QRect MainWid::winRect() const
{
  return m_configDlg->winRect();
}

bool MainWid::saveWindowPosition() const
{
  return m_configDlg->saveWindowPosition();
}

bool MainWid::saveWindowSize() const
{
  return m_configDlg->saveWindowSize();
}

QRect MainWid::parentRect() const
{
  QRect fRect = parentWidget()->frameGeometry();
  QRect rect  = parentWidget()->rect();

  return QRect(fRect.x(), fRect.y(), rect.width(), rect.height());
}

void MainWid::timerEvent(QTimerEvent *)
{
  ui_graph->addValue(m_dval);
  m_alarms->tick(QDateTime::currentMSecsSinceEpoch());
}

void MainWid::valueSLOT(double dval, const QString &val, const QString &u, const QString &s, const QString &r, bool hold, bool showBar, int id)
{
/*
  std::cerr << "valueSLOT " << dval
     << " val=" << val.toLocal8Bit().data()
     << " u=" << u.toLocal8Bit().data()
     << " s=" << s.toLocal8Bit().data()
     << " r=" << r.toLocal8Bit().data()
     << " showBar=" << showBar
     << " hold=" << hold
     << " id=" << id << std::endl;
*/
  if (m_readingLog)
  {
    ReadingLog::Entry e;
    e.when = QDateTime::currentDateTime();
    e.dval = dval;
    e.val = val;
    e.unit = u;
    e.special = s;
    e.range = r;
    e.hold = hold;
    e.id = id;
    m_readingLog->append(e);
  }

  m_display->setHold(hold);
  if (r == "AUTO") m_display->setAuto(true);
  if (r == "MANU") m_display->setManu(true);

  m_display->setShowBar(showBar);
  m_display->setMode(id, s);


  if (!hold)
  {
    m_display->setValue(id, val);
    m_display->setUnit(id, u);

    if (id == 0)
    {
      if (m_lastUnit != u)
      {
        resetSLOT();
        ui_graph->setUnit(u);
      }
      m_lastUnit = u;

      if (dval > m_max)
      {
        m_max = dval;
        m_display->setMaxUnit(u);
        m_display->setMaxValue(val);
      }

      if (dval < m_min)
      {
        m_min = dval;
        m_display->setMinUnit(u);
        m_display->setMinValue(val);
      }

      m_dval = dval;
    }
  }

  if (id == 0)
  {
    feedMeter(val, u, s, hold);

    static const QRegularExpression alarmLetters("[A-Za-z]");
    m_overload = val.contains(alarmLetters);
    m_baseUnit = SiPrefix::split(u).baseUnit;
    m_alarms->feed(dval, m_overload, QDateTime::currentMSecsSinceEpoch());

    // let the other instances see this value (calculated values, P = U * I)
    if (m_stateMgr)
    {
      SharedStateManager::Reading reading;
      reading.value = dval;
      reading.unit = SiPrefix::split(u).baseUnit;
      reading.special = s;
      reading.msecs = QDateTime::currentMSecsSinceEpoch();
      reading.valid = !hold && !val.contains(QRegularExpression("[A-Za-z]"));
      m_stateMgr->publishReading(reading);
    }
  }

  if (m_scpi)
  {
    ScpiServer::Reading reading;
    reading.value = dval;
    reading.unit = SiPrefix::split(u).baseUnit;
    reading.special = s;
    reading.range = r;
    reading.hold = hold;
    reading.overload = val.contains(QRegularExpression("[A-Za-z]"));
    reading.valid = true;
    reading.msecs = QDateTime::currentMSecsSinceEpoch();
    m_scpi->setReading(id, reading);
  }

  m_display->update();
}

void MainWid::setReadingLog(ReadingLog *log)
{
  m_readingLog = log;
}

void MainWid::setStateManager(SharedStateManager *mgr)
{
  m_stateMgr = mgr;
  m_dmm->setStateManager(mgr);
  m_configDlg->setStateManager(mgr);
  m_instancesDlg->setStateManager(mgr);
}

// The analog meter works in the unit the multimeter displays (with prefix),
// so its full scale follows the display count and the decimals of the
// reading, exactly like the meter's own bar graph.
void MainWid::feedMeter(const QString &val, const QString &unit, const QString &special, bool hold)
{
  if (!m_meter)
    return;

  static const QRegularExpression letters("[A-Za-z]");
  const bool overload = val.contains(letters);

  const double fs = MeterWid::fullScaleFromReading(val, m_configDlg->display(), unit);
  if (!std::isnan(fs))
    m_meter->setFullScale(fs);

  QString label = unit;
  if (special == "AC" || special == "DC")
    label += " " + special;
  else if (special == "ACDC")
    label += " AC+DC";
  else if (special == "DI" || special == "Diode")
    label += " DIODE";
  else if (special == "BUZ")
    label += " CONT";

  const double value = overload ? 0.0 : QString(val).remove(' ').toDouble();
  m_meter->setReading(value, val, label, overload, hold);

  // min/max memory is kept in SI base units; bring it into display units
  const double factor = SiPrefix::factor(SiPrefix::split(unit).prefix);
  const double minMark = m_min < 1.0E19 ? m_min / factor : std::nan("");
  const double maxMark = m_max > -1.0E19 ? m_max / factor : std::nan("");
  if (!std::isnan(maxMark))
    m_meter->setPeak(maxMark);
  m_meter->setMinMax(minMark, maxMark);
}

void MainWid::resetSLOT()
{
  m_min =  1.0E20;
  m_max = -1.0E20;
  if (m_meter)
    m_meter->reset();

  m_display->setMinValue("");
  m_display->setMaxValue("");
  m_display->setMinUnit("");
  m_display->setMaxUnit("");
  m_display->update();
}

void MainWid::connectSLOT(bool on)
{
  if (on)
  {
    if (m_dmm->open())
      ui_graph->clearSLOT();
    else
      Q_EMIT setConnect(false);   // the port could not be opened: button back to "off"
  }
  else
  {
    m_dmm->close();
    ui_graph->stopSLOT();
  }

  m_configDlg->connectSLOT(on);

  ui_graph->connectSLOT(on);
  m_scpi->setConnected(m_dmm->isOpen());
  // a "no readings" alarm watches a connected meter, so its clock starts here
  m_alarms->setConnected(m_dmm->isOpen(), QDateTime::currentMSecsSinceEpoch());
}

void MainWid::quitSLOT()
{
  if (closeWin()) qApp->quit();
}

void MainWid::helpSLOT()
{
  QWhatsThis::enterWhatsThisMode();
}

void MainWid::configSLOT()
{
  Q_EMIT setConnect(false);
  Q_EMIT connectDMM(false);
  connectSLOT(false);

  m_configDlg->show();
  m_configDlg->raise();
}

void MainWid::configDmmSLOT()
{
  configSLOT();
  m_configDlg->showPage(ConfigDlg::DMM);
}

void MainWid::configRecorderSLOT()
{
  configSLOT();
  m_configDlg->showPage(ConfigDlg::Recorder);
}

void MainWid::rejectSLOT()
{
  if ((sender() == m_configDlg))
  {
    Q_EMIT setConnect(true);
    Q_EMIT connectDMM(true);
    connectSLOT(true);
  }
}

void MainWid::applySLOT()
{
  readConfig();
  m_alarms->setAlarms(m_configDlg->alarms());
  updateAlarmBar();
  ui_graph->setAlertUnsaved(m_configDlg->alertUnsavedData());
  m_dmm->setName(m_configDlg->dmmName());
  applyScpi();
  Q_EMIT configChanged();

  if ((sender() == m_configDlg))
  {
    Q_EMIT setConnect(true);
    Q_EMIT connectDMM(true);
    connectSLOT(true);
  }
}

void MainWid::zoomedSLOT()
{
  ui_graph->setGraphSize(m_configDlg->windowSeconds(), m_configDlg->totalSeconds());
}

void MainWid::exportSLOT()
{
  ui_graph->exportDataSLOT();
}

void MainWid::importSLOT()
{
  ui_graph->importDataSLOT();
}

void MainWid::printSLOT()
{
  m_printDlg->setPrinter(&m_printer);

  if (m_printDlg->exec())
  {
    m_configDlg->writePrinter(&m_printer);
    ui_graph->print(&m_printer, m_printDlg->title(), m_printDlg->comment());
  }
}

void MainWid::clearSLOT()
{
  ui_graph->clearSLOT();
}

void MainWid::startSLOT()
{
  ui_graph->startSLOT();
}

void MainWid::stopSLOT()
{
  ui_graph->stopSLOT();
}

void MainWid::readConfig()
{
  bool reopen = false;

  if (m_dmm->isOpen())
  {
    m_dmm->close();
    reopen = true;
  }

  m_dmm->setDmmInfo(m_configDlg->dmmInfo());
  m_dmm->setDevice(m_configDlg->device());
  m_dmm->setSpeed(m_configDlg->speed());
  m_dmm->setFormat(m_configDlg->format());
  m_dmm->setPortSettings(static_cast<QSerialPort::DataBits>(m_configDlg->bits()), static_cast<QSerialPort::StopBits>(m_configDlg->stopBits()),
                         m_configDlg->parity(), m_configDlg->externalSetup(), m_configDlg->rts(), m_configDlg->dtr() );

  ui_graph->setGraphSize(m_configDlg->windowSeconds(), m_configDlg->totalSeconds());
  ui_graph->setStartTime(m_configDlg->startTime());
  ui_graph->setMode(m_configDlg->sampleMode());

  ui_graph->setSampleTime(m_configDlg->sampleStep());
  ui_graph->setSampleLength(m_configDlg->sampleLength());

  ui_graph->setCrosshair(m_configDlg->crosshair());

  ui_graph->setThresholds(m_configDlg->fallingThreshold(),
                          m_configDlg->raisingThreshold());

  ui_graph->setScale(m_configDlg->automaticScale(),
                     m_configDlg->includeZero(),
                     m_configDlg->scaleMin(),
                     m_configDlg->scaleMax());

  ui_graph->setColors(m_configDlg->bgColor(),
                      m_configDlg->gridColor(),
                      m_configDlg->dataColor(),
                      m_configDlg->cursorColor(),
                      m_configDlg->startColor(),
                      m_configDlg->externalColor(),
                      m_configDlg->intColor(),
                      m_configDlg->intThresholdColor());

  ui_graph->setExternal(m_configDlg->startExternal(),
                        m_configDlg->externalFalling(),
                        m_configDlg->externalThreshold());

  ui_graph->setLineStyle(m_configDlg->lineMode(),
                         m_configDlg->pointMode(),
                         m_configDlg->intLineMode(),
                         m_configDlg->intPointMode());

  m_display->setFaceColor(m_configDlg->displayBgColor());
  m_display->setDisplayMode(m_configDlg->display(), m_configDlg->showMinMax(),
                            m_configDlg->showBar(), m_configDlg->numValues());
  m_dmm->setNumValues(m_configDlg->numValues());

  if (m_meter)
  {
    MeterStyle style = m_configDlg->meterStyle() == 1 ? MeterStyle::ivory() : MeterStyle::dark();
    style.ballistics = m_configDlg->meterBallistics();
    style.redZoneFrom = m_configDlg->meterRedZone() / 100.0;
    m_meter->setStyle(style);
    m_meter->setScaleMode(static_cast<MeterWid::ScaleMode>(
      m_configDlg->meterScaleMode() == 1 ? MeterWid::Unipolar :
      m_configDlg->meterScaleMode() == 2 ? MeterWid::Bipolar : MeterWid::Auto));
  }

  ui_graph->setLine(m_configDlg->lineWidth(), m_configDlg->intLineWidth());

  ui_graph->setIntegration(m_configDlg->showIntegration(),
                           m_configDlg->intScale(),
                           m_configDlg->intThreshold(),
                           m_configDlg->intOffset());

  if (m_configDlg->sampleMode() == DMMGraph::Time)
    Q_EMIT info(tr("Automatic start at %1").arg(m_configDlg->startTime().toString()));
  else if (m_configDlg->sampleMode() == DMMGraph::Raising)
    Q_EMIT info(tr("Raising threshold %1").arg(m_configDlg->raisingThreshold()));
  else if (m_configDlg->sampleMode() == DMMGraph::Falling)
    Q_EMIT info(tr("Falling threshold %1").arg(m_configDlg->fallingThreshold()));
  Q_EMIT useTextLabel(m_configDlg->useTextLabel());
  Q_EMIT toolbarVisibility(m_configDlg->showDisplay(),
                           m_configDlg->showDmmToolbar(),
                           m_configDlg->showGraphToolbar(),
                           m_configDlg->showFileToolbar());

  if (reopen)
    m_dmm->open();
}

void MainWid::runningSLOT(bool on)
{
  m_scpi->setRecording(on);
  Q_EMIT running(on);
}

void MainWid::applyScpi()
{
  m_scpi->setModel(m_configDlg->dmmName());
  const bool wanted = m_configDlg->scpiEnabled();
  const QHostAddress address = m_configDlg->scpiAllInterfaces() ? QHostAddress::Any : QHostAddress::LocalHost;
  const quint16 port = quint16(m_configDlg->scpiPort());
  // keep a running server when nothing about it changed: clients stay
  const bool same = m_scpi->isListening() && m_scpi->address() == address
                    && m_scpi->port() >= port && m_scpi->port() < port + 10;
  if (!wanted)
  {
    m_mdns->stop();
    m_scpi->stop();
  }
  else if (!same)
  {
    m_mdns->stop();
    if (!m_scpi->start(address, port))
      Q_EMIT error(tr("SCPI server: %1").arg(m_scpi->errorString()));
  }
  if (m_scpi->isListening() && m_configDlg->scpiMdns() && !m_mdns->isActive())
  {
    QMap<QString, QString> txt;
    txt["txtvers"] = "1";
    txt["model"] = m_configDlg->dmmName();
    txt["version"] = APP_VERSION;
    txt["instance"] = m_instanceId;
    const QString instance = QString("QtDMM %1").arg(m_instanceId == "default"
                                                     ? QHostInfo::localHostName() : m_instanceId);
    m_mdns->start("_scpi-raw._tcp", instance, m_scpi->port(), txt);
  }
  else if (!m_configDlg->scpiMdns())
    m_mdns->stop();
  updateScpiStatus();
}

void MainWid::updateScpiStatus()
{
  if (!m_scpi->isListening())
  {
    Q_EMIT scpiStatus(QString());
    m_configDlg->setScpiStatus(tr("The server is not running."));
    return;
  }
  const QString where = m_scpi->address() == QHostAddress::LocalHost ? QString("localhost") : QHostInfo::localHostName();
  const int n = m_scpi->clientCount();
  const QString clients = n == 1 ? tr("1 client") : tr("%1 clients").arg(n);
  Q_EMIT scpiStatus(tr("SCPI %1:%2 (%3)").arg(where).arg(m_scpi->port()).arg(clients));
  QString text = tr("Listening on %1, port %2, %3 connected.").arg(where).arg(m_scpi->port()).arg(clients);
  if (m_mdns->isActive())
    text += ' ' + tr("Announced as \"%1\".").arg(m_mdns->instanceName());
  m_configDlg->setScpiStatus(text);
}

void MainWid::startExternalSLOT()
{
  if (m_external->state() == QProcess::Running)
  {
    QMessageBox question;
    question.setWindowTitle(tr("QtDMM: Launch error"));
    question.setText(tr("<font size=+2><b>Launch error</b></font><p>"
                        "Application %1 is still running!<p>"
                        "Do you want to kill it now?")
                     .arg(m_configDlg->externalCommand()));
    question.setIcon(QMessageBox::Information);
    question.setIconPixmap(QPixmap(":/Symbols/icon.xpm"));

    question.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    question.setDefaultButton(QMessageBox::Yes);

    QAbstractButton *yesButton = question.button(QMessageBox::Yes);
    if (yesButton)
      yesButton->setText(tr("Yes, kill it!"));

    QAbstractButton *noButton = question.button(QMessageBox::No);
    if (noButton)
      noButton->setText(tr("No, keep running"));

    switch (question.exec())
    {
      case QMessageBox::Yes:
        m_external->kill();
        break;
      default:
        return;
    }
  }

  if (m_configDlg->disconnectExternal())
    Q_EMIT setConnect(false);

  QStringList args;
  args.append(m_configDlg->externalCommand());
  m_external->setArguments(args);

  // mt: call with empty string
  m_external->start();
  if (m_external->state() != QProcess::Starting)
  {
    QMessageBox question;
    question.setWindowTitle(tr("QtDMM: Launch error"));
    question.setText(tr("<font size=+2><b>Launch error</b></font><p>"
                        "Couldn't launch %1").arg(m_configDlg->externalCommand()));
    question.setIcon(QMessageBox::Information);
    question.setIconPixmap(QPixmap(":/Symbols/icon.xpm"));

    // Nur ein "OK"-Button mit benutzerdefiniertem Text
    question.setStandardButtons(QMessageBox::Yes);
    question.setDefaultButton(QMessageBox::Yes);

    QAbstractButton *yesButton = question.button(QMessageBox::Yes);
    if (yesButton)
      yesButton->setText(tr("Bummer!"));


    question.exec();
  }
  else
    Q_EMIT error(tr("Launched %1").arg(m_configDlg->externalCommand()));
}

void MainWid::exitedSLOT()
{
  Q_EMIT error(tr("%1 terminated with exit code %2.").arg(m_configDlg->externalCommand()).arg(m_external->exitStatus()));
}

void MainWid::showTipsSLOT()
{
  if (!m_tipDlg)
  {
    m_tipDlg = new TipDlg(this);

    m_tipDlg->setShowTipsSLOT(m_configDlg->showTip());
    m_tipDlg->setCurrentTip(m_configDlg->currentTipId());

    connect(m_tipDlg, SIGNAL(showTips(bool)), m_configDlg, SLOT(setShowTipsSLOT(bool)));
    connect(m_configDlg, SIGNAL(showTips(bool)), m_tipDlg, SLOT(setShowTipsSLOT(bool)));
    connect(m_tipDlg, SIGNAL(currentTip(int)), m_configDlg, SLOT(setCurrentTipSLOT(int)));
  }

  m_tipDlg->show();
}

void MainWid::setGraphVisible(bool on)
{
  ui_graph->setVisible(on);
  // the graph is all this frame shows; without it collapse the frame so the
  // panels get the space and no empty strip remains
  setFrameShape(on ? QFrame::StyledPanel : QFrame::NoFrame);
  setMaximumHeight(on ? QWIDGETSIZE_MAX : 0);
  m_settings->setBool("MainWindow/show-graph", on);
  m_settings->save();
}

bool MainWid::graphVisible() const
{
  return m_settings->getBool("MainWindow/show-graph", false);
}

bool MainWid::dmmConfigured() const
{
  // DMM/configured is set when the settings dialog is confirmed with OK (or
  // by the instances dialog); the model check keeps configs from before
  // that key working. "Manual" alone proves nothing: applySLOT() writes it
  // at every exit, dialog or not.
  if (m_settings->getBool("DMM/configured", false))
    return true;
  const QString model = m_settings->getString("DMM/model");
  return !model.isEmpty() && model != "Manual";
}

QString MainWid::dmmTitle() const
{
  if (!dmmConfigured())
    return tr("no meter configured");
  const QString model = m_configDlg->dmmName().trimmed();
  if (!model.isEmpty())
    return model;
  return m_configDlg->device().trimmed();
}

void MainWid::setToolbarVisibility(bool disp, bool dmm, bool graph, bool file)
{
  m_configDlg->setToolbarVisibility(disp, dmm, graph, file);
}

void MainWid::instancesSLOT()
{
  m_instancesDlg->show();
}

void MainWid::instancesChangedSlot(QStringList& instances)
{
  m_instancesDlg->setInstancesOnline(instances);
}

// ---------------------------------------------------------------- alarms

void MainWid::alarmRaised(int, const Alarm &alarm, double value)
{
  const QString shown = m_overload ? QStringLiteral("OL") : EngNumberValidator::engValue(value) + m_baseUnit;
  const QString text = alarm.message.isEmpty() ? alarm.describe(m_baseUnit) : alarm.message;
  Q_EMIT error(tr("Alarm %1: %2 (%3)").arg(alarm.name, text, shown));

  if (alarm.beep)
    QApplication::beep();
  if (alarm.raiseWindow && window())
  {
    window()->raise();
    window()->activateWindow();
    QApplication::alert(window());
  }
  if (alarm.recorder == Alarm::RecorderStart)
    startSLOT();
  else if (alarm.recorder == Alarm::RecorderStop)
    stopSLOT();
  if (alarm.markGraph)
    ui_graph->addMark(alarm.color, alarm.name);
  if (alarm.markTable && m_readingLog)
    m_readingLog->markLast(alarm.color, alarm.name);
  if (!alarm.command.isEmpty())
  {
    QString cmd = alarm.command;
    cmd.replace("%v", EngNumberValidator::engValue(value)).replace("%u", m_baseUnit).replace("%n", alarm.name);
    QStringList args = QProcess::splitCommand(cmd);
    if (!args.isEmpty())
    {
      const QString program = args.takeFirst();
      if (!QProcess::startDetached(program, args))
        Q_EMIT error(tr("Alarm %1: could not run %2").arg(alarm.name, program));
    }
  }
  if (alarm.popup)
  {
    auto *box = new QMessageBox(QMessageBox::Warning, tr("QtDMM alarm: %1").arg(alarm.name),
                                QString("%1\n%2   %3").arg(text, shown, QDateTime::currentDateTime().toString("HH:mm:ss")),
                                QMessageBox::Ok, this);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setModal(false);
    box->show();
  }
  updateAlarmBar();
}

void MainWid::alarmCleared(int, const Alarm &alarm)
{
  Q_EMIT info(tr("Alarm %1 cleared").arg(alarm.name));
  updateAlarmBar();
}

// One line per raised alarm (acknowledged ones are silent), on the colour
// of the first one.
void MainWid::updateAlarmBar()
{
  QStringList lines;
  QColor color;
  const QList<Alarm> &list = m_alarms->alarms();
  for (int i = 0; i < list.size(); ++i)
  {
    if (m_alarms->state(i) != AlarmManager::Raised || !list[i].banner)
      continue;
    const Alarm &al = list[i];
    lines << QString("%1: %2").arg(al.name, al.message.isEmpty() ? al.describe(m_baseUnit) : al.message);
    if (!color.isValid())
      color = al.color;
  }
  m_alarmBar->setAlarms(lines.join('\n'), color);
}
