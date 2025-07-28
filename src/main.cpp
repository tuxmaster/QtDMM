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

#ifdef Q_OS_LINUX
bool userInGroup(const char* groupname) {
  struct passwd* pw = getpwuid(geteuid());
  if (!pw) return false;

  gid_t* groups;
  int ngroups = 0;

  // Erste Abfrage, um die Anzahl der Gruppen zu bekommen
  getgrouplist(pw->pw_name, pw->pw_gid, nullptr, &ngroups);

  groups = new gid_t[ngroups];
  if (getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups) == -1)
  {
    delete[] groups;
    return false;
  }

  for (int i = 0; i < ngroups; ++i)
  {
    struct group* gr = getgrgid(groups[i]);
    if (gr && strcmp(gr->gr_name, groupname) == 0)
    {
      delete[] groups;
      return true;
    }
  }

  delete[] groups;
  return false;
}
#endif

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

#ifdef Q_OS_UNIX
  if (!userInGroup("dialout")) {
      QMessageBox::critical(nullptr, QObject::tr("Missing Permission"),
                          QObject::tr("The current user is not a member of the 'dialout' group.\n"
                          "Please add the user with the following command:\n\n"
                          "sudo usermod -aG dialout $USER\n\n"
                          "You need to log out and back in for the changes to take effect."));
  }
#endif

  return app.exec();
}
