//======================================================================
// File:		main.cpp
// Author:	Matthias Toussaint
// Created:	Tue Apr 10 17:36:39 CEST 2001
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
#include <QMessageBox>
#include <QtGui>
#include <iostream>

#include "mainwin.h"

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <stdio.h>

// QtDMM is a GUI-subsystem executable on Windows, so it has no console of its
// own. When started from cmd/PowerShell, reattach to the parent's console so
// --help, --version and the --debug frame dump remain visible there.
static void attachParentConsole()
{
  if (::AttachConsole(ATTACH_PARENT_PROCESS))
  {
    FILE *f = nullptr;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    // Qt would otherwise route qWarning() & co. to the debugger
    qputenv("QT_FORCE_STDERR_LOGGING", "1");
  }
}
#endif

// Everything goes to stderr, prefixed by severity. Written with fprintf on
// purpose: calling qDebug() & co. from inside the handler re-enters it.
// Debug messages only appear for logging categories switched on by --debug
// (see MainWin::setConsoleLogging), which keeps release builds quiet.
void qtdmmMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  const char *prefix = "";
  switch (type)
  {
    case QtDebugMsg:
      prefix = context.category ? context.category : "Debug";
      break;
    case QtInfoMsg:     prefix = "Info"; break;
    case QtWarningMsg:  prefix = "Warning"; break;
    case QtCriticalMsg: prefix = "Critical"; break;
    case QtFatalMsg:    prefix = "Fatal"; break;
  }
  fprintf(stderr, "%s: %s\n", prefix, msg.toLocal8Bit().constData());
  fflush(stderr);
  if (type == QtFatalMsg)
    abort();
}

void initTranslation(QApplication *app,QTranslator *QtTranslation, QTranslator *AppTranslation)
{
  if (QtTranslation->load(QString("qt_%1").arg(QLocale::system().name()), QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    app->installTranslator(QtTranslation);
  else
    qWarning() << "Could not load Qt translation!";

  if (AppTranslation->load(QString("%1").arg(QLocale::system().name()), ":/Translations")) //
    app->installTranslator(AppTranslation);
  else
    qWarning() << "Could not load App translation!";
}

int main(int argc, char **argv)
{
#ifdef Q_OS_WIN
  attachParentConsole();
#endif
  qInstallMessageHandler(qtdmmMessageOutput);
  QApplication app(argc, argv);
  QTranslator QtTranslation;
  QTranslator AppTranslation;
  QCommandLineParser parser;

  app.setApplicationName(APP_NAME);
  app.setApplicationVersion(APP_VERSION);
  app.setOrganizationName(APP_ORGANIZATION);

  initTranslation(&app,&QtTranslation,&AppTranslation);

  parser.addOption({"debug", QObject::tr("protocol debugging information")});
  parser.addOption({"config-dir",QObject::tr("sets directory where config files are located"), "config-dir"});
  parser.addOption({"config-id",QObject::tr("sets <config-id>"), "config-id"});
  parser.addHelpOption();
  parser.addVersionOption();
  parser.process(app);

  MainWin mainWin(parser);

  mainWin.show();
  mainWin.move(100, 100);

  return app.exec();
}
