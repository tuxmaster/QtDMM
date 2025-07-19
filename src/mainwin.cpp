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
#include "mainwid.h"
#include "displaywid.h"




MainWin::MainWin(QWidget *parent)
  : QMainWindow(parent)
  , m_running(false)
  , m_menu(Q_NULLPTR)
{
  setupUi(this);
  setupIcons();


  QWidget* spacer = new QWidget();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  this->toolBarMenu->addWidget(spacer);
  this->toolBarMenu->addAction(this->action_Menu);

  m_wid = new MainWid(this);
  setCentralWidget(m_wid);

  createActions();


  m_display = new DisplayWid(this);
  toolBarDisplay->addWidget(m_display);
  m_wid->setDisplay(m_display);

  setWindowTitle(QString("%1 %2").arg(APP_NAME).arg(APP_VERSION));

  connect(m_wid, SIGNAL(running(bool)), this, SLOT(runningSLOT(bool)));

  connectSLOT(false);

  m_error = new QLabel(statusBar());
  m_error->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  statusBar()->addWidget(m_error, 20);
  m_error->setLineWidth(1);

  m_info = new QLabel(statusBar());
  m_info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  statusBar()->addWidget(m_info, 10);
  m_info->setLineWidth(1);

  connect(m_wid, SIGNAL(error(const QString &)), m_error, SLOT(setText(const QString &)));
  connect(m_wid, SIGNAL(info(const QString &)), m_info, SLOT(setText(const QString &)));
  connect(m_wid, SIGNAL(useTextLabel(bool)), this, SLOT(setUseTextLabel(bool)));
  connect(m_wid, SIGNAL(setConnect(bool)), this, SLOT(setConnectSLOT(bool)));
  connect(m_wid, SIGNAL(toolbarVisibility(bool, bool, bool, bool, bool)),
          this, SLOT(toolbarVisibilitySLOT(bool, bool, bool, bool, bool)));

  connect(m_wid, SIGNAL(connectDMM(bool)), action_Connect, SLOT(setChecked(bool)));

  QRect winRect = m_wid->winRect();

  m_wid->applySLOT();


  if (!winRect.isEmpty())
  {
    if (m_wid->saveWindowPosition())
      move(winRect.x(), winRect.y());
    if (m_wid->saveWindowSize())
      resize(winRect.width(), winRect.height());
    else
      resize(640, 480);
  }
  QTimer::singleShot(1000, action_Connect, &QAction::trigger);
}

void MainWin::setConsoleLogging(bool on)
{
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
  connect(action_Start, SIGNAL(triggered()), m_wid, SLOT(startSLOT()));
  connect(action_Stop, SIGNAL(triggered()), m_wid, SLOT(stopSLOT()));
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

  connect(toolBarDisplay, SIGNAL(visibilityChanged(bool)), this, SLOT(setToolbarVisibilitySLOT()));
  connect(toolBarMenu, SIGNAL(visibilityChanged(bool)),  this, SLOT(setToolbarVisibilitySLOT()));
  connect(toolBarFile, SIGNAL(visibilityChanged(bool)), this, SLOT(setToolbarVisibilitySLOT()));
  connect(toolBarRecorder, SIGNAL(visibilityChanged(bool)), this, SLOT(setToolbarVisibilitySLOT()));
  connect(toolBarDMM, SIGNAL(visibilityChanged(bool)), this, SLOT(setToolbarVisibilitySLOT()));

  connect(new QShortcut(action_Configure->shortcut(), this), SIGNAL(activated()), action_Configure, SLOT(trigger()));
  connect(new QShortcut(action_Direct_help->shortcut(), this), SIGNAL(activated()), action_Direct_help, SLOT(trigger()));
  connect(new QShortcut(action_About->shortcut(), this), SIGNAL(activated()), action_About, SLOT(trigger()));
  connect(new QShortcut(action_Quit->shortcut(), this), SIGNAL(activated()), action_Quit, SLOT(trigger()));
}

void MainWin::runningSLOT(bool on)
{
  m_running = on;

  action_Stop->setEnabled(!on);
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

void MainWin::on_action_About_triggered()
{
  QMessageBox::about(this, tr("QtDMM: Welcome!"), tr("<h1>QtDMM %1</h1><hr>"\
                     "<div align=right><i>A simple recorder for DMM's</i></div><p>"\
                     "<div align=justify>A simple display software for a variety of digital multimeter. Currently confirmed are:"\
                     "<table>%2</table>Other compatible models may work also.<p>"\
                     "QtDMM features min/max memory and a configurable "\
                     "recorder with import/export and printing function. Sampling may"\
                     " be started manually, at a given time or triggered by a measured threshold. "\
                     "Additionally an external program may be started when given thresholds are reached.</div>"\
                     "<div align=justify><b>QtDMM</b> uses the platform independent toolkit "\
                     "<b>Qt</b> version %3 and is licensed under <b>GPL 3</b> (Versions prior to v0.9.0 where licensed under GPL 2)</div><br>"\
                     "&copy; 2001-2014 Matthias Toussaint &nbsp;-&nbsp;&nbsp;<font color=blue><u><a href='mailto:qtdmm@mtoussaint.de'>qtdmm@mtoussaint.de</a></u></font>"\
                     "<p><br>The icons (except the DMM icon) have been taken from the KDE project.<p>")
                     .arg(APP_VERSION).arg(m_wid->deviceListText()).arg(qVersion()));
}

void MainWin::on_action_Menu_triggered()
{
  if (!m_menu)
  {
    m_menu = new QMenu(this);
    m_menu->addAction(action_Configure);
    m_menu->addSeparator();
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

  if (m_wid->closeWin())
    ev->accept();
  else
    ev->ignore();
}

void MainWin::setToolbarVisibilitySLOT()
{
  m_wid->setToolbarVisibility(toolBarDisplay->isVisible(),
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
  toolBarDisplay->setVisible(disp);
}

void MainWin::setupIcons()
{
  QIcon iconConnectOn = QIcon::fromTheme("network-connect");
  QIcon iconConnectOff = QIcon::fromTheme("network-disconnect");

  this->action_Connect->setIcon(iconConnectOff);
  connect(this->action_Connect, &QAction::toggled, this, [ = ](bool checked)
  {
    this->action_Connect->setIcon(checked ? iconConnectOn : iconConnectOff);
  });
}
