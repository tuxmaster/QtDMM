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
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include "mainwin.h"

void checkUserInDailOutGroup()
{
#ifdef Q_OS_LINUX
  const char* groupname = "dialout";
  struct passwd* pw = getpwuid(geteuid());
  if (!pw) return; // check not applicable

  gid_t* groups;
  int ngroups = 0;

  getgrouplist(pw->pw_name, pw->pw_gid, nullptr, &ngroups);

  groups = new gid_t[ngroups];
  if (getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups) == -1)
  {
    delete[] groups;
    return; // cannot get groups
  }

  for (int i = 0; i < ngroups; ++i)
  {
    struct group* gr = getgrgid(groups[i]);
    if (gr && strcmp(gr->gr_name, groupname) == 0)
    {
      delete[] groups;
      return; // ok
    }
  }

  delete[] groups;
  QMessageBox::critical(nullptr, QObject::tr("Missing Permission"),
                        QObject::tr("The current user is not a member of the 'dialout' group.\n"
                        "Please add the user with the following command:\n\n"
                        "sudo usermod -aG dialout $USER\n\n"
                        "You need to log out and back in for the changes to take effect."));

#endif
}


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

  checkUserInDailOutGroup();

  return app.exec();
}
