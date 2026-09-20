//======================================================================
// File:		mainwin.cpp
// Author:	Matthias Toussaint
// Created:	Sun Sep  2 12:15:28 CEST 2001
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
#include <QTimer>
#include <QMenu>

#include "mainwin.h"
#include "helpdlg.h"
#include "mainwid.h"
#include "displaywid.h"
#include "meterwid.h"
#include "settings.h"
#include <QDockWidget>
#include <QLoggingCategory>

MainWin::MainWin(QCommandLineParser &parser, QWidget *parent)
  : QMainWindow(parent)
  , m_running(false)
  , m_menu(Q_NULLPTR)
  , m_helpDlg(Q_NULLPTR)
  , m_localRecord(true)
{
  setupUi(this);
  setupIcons();
  m_config_id = parser.value("config-id");

  m_stateMgr = new SharedStateManager(m_config_id.isEmpty()?"default":m_config_id,this);

  QWidget* spacer = new QWidget();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  this->toolBarMenu->addWidget(spacer);
  this->toolBarMenu->addAction(this->action_Menu);
  m_wid = new MainWid(m_config_id, parser.value("config-dir"), this);
  setCentralWidget(m_wid);
  setConsoleLogging(parser.isSet("debug"));

  createActions();

  // digital display and analog meter live in docks: dockable on any side,
  // floatable as their own freely resizable windows, tabbable
  m_display = new DisplayWid(this);
  m_displayDock = new QDockWidget(tr("Display"), this);
  m_displayDock->setObjectName("displayDock");
  m_displayDock->setWidget(m_display);
  m_displayDock->setAllowedAreas(Qt::AllDockWidgetAreas);
  addDockWidget(Qt::TopDockWidgetArea, m_displayDock);
  m_wid->setDisplay(m_display);

  QAction *displayAction = m_displayDock->toggleViewAction();
  displayAction->setText(tr("&Display"));
  displayAction->setIcon(QIcon(":/Symbols/display.xpm"));
  displayAction->setWhatsThis(tr("<html><head/><body><p><span style=\" font-weight:600;\">Display</span></p>"
                                 "<p>Show the reading on the LCD-style digital display. The panel can be docked on any side "
                                 "of the window or dragged out as a separate window.</p></body></html>"));

  // the analog meter is hidden until the user switches it on
  m_meter = new MeterWid(this);
  m_meterDock = new QDockWidget(tr("Analog meter"), this);
  m_meterDock->setObjectName("meterDock");
  m_meterDock->setWidget(m_meter);
  m_meterDock->setAllowedAreas(Qt::AllDockWidgetAreas);
  addDockWidget(Qt::RightDockWidgetArea, m_meterDock);
  m_meterDock->hide();
  m_wid->setMeter(m_meter);
  m_wid->setStateManager(m_stateMgr);

  QAction *meterAction = m_meterDock->toggleViewAction();
  meterAction->setText(tr("Analog &meter"));
  meterAction->setIcon(QIcon(":/Symbols/meter.xpm"));
  meterAction->setWhatsThis(tr("<html><head/><body><p><span style=\" font-weight:600;\">Analog meter</span></p>"
                               "<p>Show the reading on a moving-coil style instrument. The panel can be docked on any side "
                               "of the window or dragged out as a separate window.</p></body></html>"));
  toolBarDMM->addSeparator();
  toolBarDMM->addAction(displayAction);
  toolBarDMM->addAction(meterAction);
  connect(m_displayDock, SIGNAL(visibilityChanged(bool)), this, SLOT(setToolbarVisibilitySLOT()));

  // Locked panels have no title bar (no drag handle, no float/close
  // buttons) - the instruments then sit flush in the window. Unlock to
  // rearrange them.
  m_lockPanels = new QAction(tr("&Lock panels"), this);
  m_lockPanels->setCheckable(true);
  m_lockPanels->setWhatsThis(tr("<html><head/><body><p><span style=\" font-weight:600;\">Lock panels</span></p>"
                                "<p>Hide the title bars of the display and meter panels. Unlock them to move the panels "
                                "to another side of the window or to drag them out as separate windows.</p></body></html>"));
  connect(m_lockPanels, &QAction::toggled, this, &MainWin::setPanelsLocked);
  m_lockPanels->setChecked(m_wid->settings()->getBool("MainWindow/lock-panels", true));
  setPanelsLocked(m_lockPanels->isChecked());

  updateWindowTitle();
  connect(m_wid, &MainWid::configChanged, this, &MainWin::updateWindowTitle);

  action_Graph->setChecked(m_wid->graphVisible());
  connect(action_Graph, &QAction::toggled, this, &MainWin::setGraphVisible);
  setGraphVisible(m_wid->graphVisible());

  connect(m_wid, SIGNAL(running(bool)), this, SLOT(runningSLOT(bool)));

  connectSLOT(false);

  // status bar
  m_error = new QLabel(statusBar());
  m_error->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  statusBar()->addWidget(m_error, 20);
  m_error->setLineWidth(1);

  m_info = new QLabel(statusBar());
  m_info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  statusBar()->addWidget(m_info, 10);
  m_info->setLineWidth(1);

  // messages such as the permission hint span several lines; the status bar
  // shows the first one and keeps the rest in the tooltip
  connect(m_wid, &MainWid::error, this, [this](const QString &text)
  {
    const QString firstLine = text.section('\n', 0, 0);
    m_error->setText(firstLine);
    m_error->setToolTip(text.contains('\n') ? text : QString());
  });
  connect(m_wid, SIGNAL(info(const QString &)), m_info, SLOT(setText(const QString &)));
  connect(m_wid, SIGNAL(useTextLabel(bool)), this, SLOT(setUseTextLabel(bool)));
  connect(m_wid, SIGNAL(setConnect(bool)), this, SLOT(setConnectSLOT(bool)));
  connect(m_wid, SIGNAL(connectDMM(bool)), action_Connect, SLOT(setChecked(bool)));
  connect(m_wid, SIGNAL(toolbarVisibility(bool, bool, bool, bool)),
          this, SLOT(toolbarVisibilitySLOT(bool, bool, bool, bool)));

  QRect winRect = m_wid->winRect();

  m_wid->applySLOT();
  restoreState(m_wid->settings()->getString("MainWindow/state").isEmpty()
                 ? QByteArray()
                 : QByteArray::fromBase64(m_wid->settings()->getString("MainWindow/state").toLatin1()));

  if (!winRect.isEmpty())
  {
    if (m_wid->saveWindowPosition())
    {
      move(winRect.x(), winRect.y());
    }
    if (m_wid->saveWindowSize())
      resize(winRect.width(), winRect.height());
    else
      resize(640, 480);
  }

  connect(m_stateMgr, &SharedStateManager::stateChanged, this, [=](const QString& state){
    if (state == "RECORD")
    {
      QMetaObject::invokeMethod(m_wid, "startSLOT", Qt::DirectConnection);
      m_localRecord = false;
    }
    else if (state == "STOP")
    {
      if (!m_localRecord)
        action_Stop->trigger();
    }
    else if (state == "RAISE_"+(m_config_id.isEmpty()?"default":m_config_id))
    {
      bringMainWindowToFront();
      m_stateMgr->writeState("IDLE");
    }
  });

  connect(m_stateMgr, &SharedStateManager::instanceIdAlreadyInUse, this, [=](){
    QMessageBox::critical(this, APP_NAME,tr("Another instance is running."));
    qApp->quit();
  });

  // auto-connect at start, but not before a meter was ever chosen: a fresh
  // instance would otherwise try the first serial port it finds
  if (m_stateMgr->registerInstance() && m_wid->dmmConfigured())
    QTimer::singleShot(1000, action_Connect, &QAction::trigger);
}

// Without the graph the window may shrink to the panels and toolbars; the
// height it had before hiding comes back when the graph is shown again.
void MainWin::setGraphVisible(bool on)
{
  static const int kMinHeightWithGraph = 450;
  static const int kMinHeightWithoutGraph = 220;
  m_wid->setGraphVisible(on);
  setMinimumHeight(on ? kMinHeightWithGraph : kMinHeightWithoutGraph);
  if (!on)
  {
    m_heightWithGraph = height();
    resize(width(), qMax(kMinHeightWithoutGraph, minimumSizeHint().height()));
  }
  else if (m_heightWithGraph > 0 && height() < m_heightWithGraph)
    resize(width(), m_heightWithGraph);
}

// "QtDMM: UNI-T UT61E", with the instance id for non-default instances
void MainWin::updateWindowTitle()
{
  QString title = APP_NAME;
  if (!m_config_id.isEmpty())
    title += QString(" [%1]").arg(m_config_id);
  setWindowTitle(QString("%1: %2").arg(title, m_wid->dmmTitle()));
}

void MainWin::sendStateSLOT(const QString & state)
{
  m_stateMgr->writeState(state);
}


void MainWin::setConsoleLogging(bool on)
{
  if (on)
    QLoggingCategory::setFilterRules("qtdmm.hid.debug=true");
  m_wid->setConsoleLogging(on);
}

void MainWin::setUseTextLabel(bool on)
{
  Qt::ToolButtonStyle Style = Qt::ToolButtonTextUnderIcon;
  if (!on)
    Style = Qt::ToolButtonIconOnly;
  toolBarDMM->setToolButtonStyle(Style);
  toolBarRecorder->setToolButtonStyle(Style);
  toolBarFile->setToolButtonStyle(Style);
  toolBarMenu->setToolButtonStyle(Style);
}

void MainWin::createActions()
{
  connect(action_Connect, SIGNAL(triggered(bool)), m_wid, SLOT(connectSLOT(bool)));
  connect(action_Connect, SIGNAL(triggered(bool)), this, SLOT(connectSLOT(bool)));
  connect(action_Reset, SIGNAL(triggered()), m_wid, SLOT(resetSLOT()));
  connect(action_Start, SIGNAL(triggered()), this, SLOT(startSLOT()));
  connect(action_Stop, SIGNAL(triggered()), m_wid, SLOT(stopSLOT()));
  connect(action_Stop, SIGNAL(triggered()), this, SLOT(stopSLOT()));
  connect(action_Clear, SIGNAL(triggered()), m_wid, SLOT(clearSLOT()));
  connect(action_Print, SIGNAL(triggered()), m_wid, SLOT(printSLOT()));
  connect(action_Import, SIGNAL(triggered()), m_wid, SLOT(importSLOT()));
  connect(action_Export, SIGNAL(triggered()), m_wid, SLOT(exportSLOT()));
  connect(action_Configure, SIGNAL(triggered()), m_wid, SLOT(configSLOT()));
  connect(action_ConfigureDMM, SIGNAL(triggered()), m_wid, SLOT(configDmmSLOT()));
  connect(actionConfigureRecorder, SIGNAL(triggered()), m_wid, SLOT(configRecorderSLOT()));
  connect(action_Quit, SIGNAL(triggered()), this, SLOT(setToolbarVisibilitySLOT()));
  connect(action_Quit, SIGNAL(triggered()), m_wid, SLOT(quitSLOT()));
  connect(action_Direct_help, SIGNAL(triggered()), m_wid, SLOT(helpSLOT()));
  connect(action_Tip_of_the_day, SIGNAL(triggered()), m_wid, SLOT(showTipsSLOT()));
  connect(action_Instances, SIGNAL(triggered()), m_wid, SLOT(instancesSLOT()));

  connect(toolBarMenu, SIGNAL(visibilityChanged(bool)),  this, SLOT(setToolbarVisibilitySLOT()));
  connect(toolBarFile, SIGNAL(visibilityChanged(bool)), this, SLOT(setToolbarVisibilitySLOT()));
  connect(toolBarRecorder, SIGNAL(visibilityChanged(bool)), this, SLOT(setToolbarVisibilitySLOT()));
  connect(toolBarDMM, SIGNAL(visibilityChanged(bool)), this, SLOT(setToolbarVisibilitySLOT()));

  connect(m_stateMgr, SIGNAL(instancesChanged(QStringList&)), m_wid, SLOT(instancesChangedSlot(QStringList&)));

  connect(new QShortcut(action_Configure->shortcut(), this), SIGNAL(activated()), action_Configure, SLOT(trigger()));
  connect(new QShortcut(action_Direct_help->shortcut(), this), SIGNAL(activated()), action_Direct_help, SLOT(trigger()));
  connect(new QShortcut(action_Help->shortcut(), this), SIGNAL(activated()), action_Help, SLOT(trigger()));
  connect(new QShortcut(action_Quit->shortcut(), this), SIGNAL(activated()), action_Quit, SLOT(trigger()));
}

void MainWin::startSLOT()
{
  if (m_stateMgr->instances().count()<=1)
  {
    QMetaObject::invokeMethod(m_wid, "startSLOT", Qt::DirectConnection);
    m_localRecord = true;
  }
  else
  {
    QMessageBox question(
      QMessageBox::Question,
      tr("Record DMM data"),
      tr("Multiple instances of QtDMM have been detected.\n"
         "Please choose which instance should record."),
      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    question.button(QMessageBox::Yes)->setText(tr("This instance"));
    question.button(QMessageBox::No)->setText(tr("All instances"));
    question.setEscapeButton(QMessageBox::Cancel);

    switch (question.exec())
    {
      case QMessageBox::Yes:
        QMetaObject::invokeMethod(m_wid, "startSLOT", Qt::DirectConnection);
        m_localRecord = true;
        return;
      case QMessageBox::No:
        m_stateMgr->writeState("RECORD");
        m_localRecord = false;
        return;
    }
  }
}

void MainWin::stopSLOT()
{
  qInfo() << "stop" << m_localRecord;
  if (! m_localRecord)
    m_stateMgr->writeState("STOP");
  m_localRecord = false;

}

void MainWin::runningSLOT(bool on)
{
  m_running = on;
  if (on)
    action_Graph->setChecked(true);   // a recording wants to be seen

  action_Start->setEnabled(!on);
  action_Stop->setEnabled(on);
  action_Print->setEnabled(!on);
  action_Export->setEnabled(!on);
  action_Import->setEnabled(!on);
}

void MainWin::connectSLOT(bool on)
{
  action_Start->setEnabled(on);
  action_Stop->setEnabled(on && m_running);

  if (!on)
    m_running = false;
}

void MainWin::on_action_Help_triggered()
{
  if (!m_helpDlg)
    m_helpDlg = new HelpDlg(m_wid->settings(), this);
  m_helpDlg->show();
  m_helpDlg->raise();
  m_helpDlg->activateWindow();
}

void MainWin::on_action_About_triggered()
{
  QMessageBox about(this);
  about.setWindowTitle(tr("About QtDMM"));
  about.setIconPixmap(QPixmap(":/Symbols/icon.xpm"));
  about.setTextFormat(Qt::RichText);
  about.setText(tr("<h2>QtDMM %1</h2>"
                   "<p>A readout and transient recorder for digital multimeters.</p>"
                   "<p>Built with <b>Qt</b> %2. Licensed under the <b>GNU GPL 3</b> "
                   "(versions before 0.9.0 under GPL 2).</p>"
                   "<p>0.9.5 onwards: tuxmaster and contributors, see the AUTHORS file.<br>"
                   "0.9.3 and before: &copy; 2001-2016 M. Toussaint "
                   "&lt;<a href='mailto:qtdmm@mtoussaint.de'>qtdmm@mtoussaint.de</a>&gt;</p>"
                   "<p>Website: <a href='https://qtdmm.de'>qtdmm.de</a> &middot; "
                   "Contact: <a href='mailto:hello@qtdmm.de'>hello@qtdmm.de</a><br>"
                   "Source and bug reports: <a href='https://github.com/tuxmaster/QtDMM'>github.com/tuxmaster/QtDMM</a><br>"
                   "Icons (except the DMM icon) are taken from the KDE project.</p>")
                .arg(APP_VERSION).arg(qVersion()));

  // The device list used to be pasted in here as a table; it lives in the
  // handbook now, where it is readable, searchable on the web and generated
  // from the decoders instead of maintained by hand.
  QPushButton *devices = about.addButton(tr("Supported devices..."), QMessageBox::ActionRole);
  about.addButton(QMessageBox::Close);
  about.setDefaultButton(QMessageBox::Close);
  about.exec();

  if (about.clickedButton() == devices)
  {
    on_action_Help_triggered();
    m_helpDlg->showPage("supported-devices.md");
  }
}

void MainWin::on_action_Menu_triggered()
{
  if (!m_menu)
  {
    m_menu = new QMenu(this);
    m_menu->addAction(action_Configure);
    m_menu->addAction(action_Graph);
    m_menu->addAction(m_displayDock->toggleViewAction());
    m_menu->addAction(m_meterDock->toggleViewAction());
    m_menu->addAction(m_lockPanels);
    m_menu->addSeparator();
    m_menu->addAction(action_Help);
    m_menu->addAction(action_Tip_of_the_day);
    m_menu->addAction(action_Direct_help);
    m_menu->addAction(action_About);
    m_menu->addSeparator();
    m_menu->addAction(action_Quit);
  }

  QWidget* widget = this->toolBarMenu->widgetForAction(this->action_Menu);
  if (widget)
  {
    m_menu->popup(widget->mapToGlobal(
      QPoint( widget->width() - m_menu->sizeHint().width(), widget->height())
    ));
  }
}


void MainWin::closeEvent(QCloseEvent *ev)
{
  setToolbarVisibilitySLOT();
  // dock layout (meter position, floating state, size) and toolbar layout
  m_wid->settings()->setString("MainWindow/state", QString::fromLatin1(saveState().toBase64()));
  m_wid->settings()->setBool("MainWindow/lock-panels", m_lockPanels->isChecked());

  if (m_wid->closeWin())
    ev->accept();
  else
    ev->ignore();
}

void MainWin::setPanelsLocked(bool locked)
{
  for (QDockWidget *dock : { m_displayDock, m_meterDock })
  {
    QWidget *old = dock->titleBarWidget();
    // an empty widget as title bar hides it; nullptr restores the default one
    dock->setTitleBarWidget(locked ? new QWidget(dock) : nullptr);
    delete old;
  }
}

void MainWin::setToolbarVisibilitySLOT()
{
  m_wid->setToolbarVisibility(m_displayDock->isVisible(),
                              toolBarDMM->isVisible(),
                              toolBarRecorder->isVisible(),
                              toolBarFile->isVisible());
}

void MainWin::setConnectSLOT(bool on)
{
  action_Connect->setChecked(on);
}

void MainWin::toolbarVisibilitySLOT(bool disp, bool dmm, bool graph, bool file)
{
  toolBarDMM->setVisible(dmm);
  toolBarRecorder->setVisible(graph);
  toolBarFile->setVisible(file);
  m_displayDock->setVisible(disp);
}

void MainWin::setupIcons()
{
  // theme icons exist on Linux desktops only; Windows and macOS get the
  // bundled ones
  QIcon iconConnectOn = QIcon::fromTheme("network-connect", QIcon(":/Symbols/connect_on.xpm"));
  QIcon iconConnectOff = QIcon::fromTheme("network-disconnect", QIcon(":/Symbols/connect_icon.xpm"));

  this->action_Connect->setIcon(iconConnectOff);
  connect(this->action_Connect, &QAction::toggled, this, [ = ](bool checked)
  {
    this->action_Connect->setIcon(checked ? iconConnectOn : iconConnectOff);
  });
}

void MainWin::bringMainWindowToFront()
{
  QWidget *mainWin = nullptr;
  const auto topWidgets = QApplication::topLevelWidgets();

  for (QWidget *w : topWidgets)
  {
    if (w->inherits("MainWin"))
    {
      mainWin = w;
      break;
    }
  }

  if (!mainWin)
    return;

  mainWin->showNormal();
  mainWin->raise();
  mainWin->activateWindow();
}
