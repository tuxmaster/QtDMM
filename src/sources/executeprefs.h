//======================================================================
// File:		executeprefs.h
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:26:02 CEST 2002
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
// Copyright (c) 2002 Matthias Toussaint
//======================================================================

#ifndef EXECUTEPREFS_HH
#define EXECUTEPREFS_HH

#include <uiexecuteprefs.h>

class ExecutePrefs : public UIExecutePrefs
{
  Q_OBJECT
public:
  ExecutePrefs( QWidget *parent=0, const char *name=0 );
  virtual ~ExecutePrefs();

  bool startExternal() const;
  bool externalFalling() const;
  double externalThreshold() const;
  bool disconnectExternal() const;
  QString externalCommand() const;
  void setThreshold( double );
  
public slots:
  virtual void defaultsSLOT();
  virtual void factoryDefaultsSLOT();
  virtual void applySLOT();
  
protected slots:
  void browseExecSLOT();

};

#endif // EXECUTEPREFS_HH

