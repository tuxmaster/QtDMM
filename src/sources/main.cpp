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

#include <qapplication.h>
#include <mainwin.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>

void myMessageOutput( QtMsgType type, const char *msg )
{
  QString txt = msg;
  
  switch ( type ) {
    case QtDebugMsg:
      fprintf( stderr, "Debug: %s\n", msg );
      //abort();
      break;
    case QtWarningMsg:     
      if (txt.contains( "Absolute index" )) abort();
      //fprintf( stderr, "Warning: %s\n", msg );
      //abort();
      break;
    case QtFatalMsg:
      fprintf( stderr, "Fatal: %s\n", msg );
      //abort();                    // deliberately core dump
  }
}

int
main( int argc, char **argv )
{
  qInstallMsgHandler( myMessageOutput );
  QApplication app( argc, argv );
  
  MainWin mainWin;
  
  // very simple parsing (tm)
  for (int i=0; i<argc; ++i)
  {
    if (QString(argv[i]) == "--console")
    {
      mainWin.setConsoleLogging( true );
      break;
    }
  }
  
  app.setMainWidget( &mainWin );
  mainWin.show();
  mainWin.move( 100, 100 );
  
  return app.exec();
}
