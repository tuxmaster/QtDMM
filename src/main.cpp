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

void qtdmmMessageOutput(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
  switch (type)
  {
    case QtDebugMsg:
#ifdef QT_DEBUG
      qDebug() << "Debug: " << msg;
#endif
      break;
    case QtWarningMsg:
#ifdef QT_DEBUG
      if (msg.contains("Absolute index"))
        abort();
#endif
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
