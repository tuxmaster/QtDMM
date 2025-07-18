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

#include <QtGui>
#include <iostream>

#include "mainwin.h"


void myMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
  switch (type)
  {
    case QtDebugMsg:
#ifdef QT_DEBUG
      qDebug() << "Debug: " << msg;
#endif
      break;
    case QtWarningMsg:
      if (msg.contains("Absolute index"))
        abort();
      else
        qWarning() << "Warning: " << msg;
      break;
    case QtFatalMsg:
      qFatal("Fatal: %s", msg.toUtf8().constData());
    case QtCriticalMsg:
      qCritical() << "Critial: " << msg;
      break;
    case QtInfoMsg:
      qInfo() << "Info: " << msg;
      break;
  }
}

void printHelp()
{
  std::cerr << "Usage: qtdmm [--help|--debug|--print]" << std::endl;
  std::cerr << "  --help  : print this message" << std::endl;
  std::cerr << "  --debug : protocoll debugging information" << std::endl;
  std::cerr << "  --print : send DMM reading to standard output" << std::endl;
  exit(0);
}


int main(int argc, char **argv)
{
  qInstallMessageHandler(myMessageOutput);
  QApplication app(argc, argv);

  app.setApplicationName(APP_NAME);
  app.setApplicationVersion(APP_VERSION);
  app.setOrganizationName(APP_ORGANIZATION);

  // translation
  QTranslator QtTranslation;
  if (QtTranslation.load(QString("qt_%1").arg(QLocale::system().name()), QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
    app.installTranslator(&QtTranslation);
  else
    qWarning() << "Could not load Qt translation!";

  QTranslator AppTranslation;
  if (AppTranslation.load(QString("%1").arg(QLocale::system().name()), ":/Translations")) //
    app.installTranslator(&AppTranslation);
  else
    qWarning() << "Could not load App translation!";

  MainWin mainWin;

  QCommandLineParser parser;
  parser.addOption(QCommandLineOption("debug"));
  parser.addOption(QCommandLineOption("help"));
  parser.addOption(QCommandLineOption("print"));
  parser.process(app);

  if (parser.isSet("debug"))
    mainWin.setConsoleLogging(true);
  mainWin.show();
  mainWin.move(100, 100);

  return app.exec();
}
