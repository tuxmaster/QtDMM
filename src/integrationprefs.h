//======================================================================
// File:		integrationprefs.h
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:31:10 CEST 2002
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

#ifndef INTEGRATIONPREFS_HH
#define INTEGRATIONPREFS_HH

#include <uiintegrationprefs.h>

class IntegrationPrefs : public UIIntegrationPrefs
{
  Q_OBJECT
public:
  IntegrationPrefs( QWidget *parent=0, const char *name=0 );
  virtual ~IntegrationPrefs();
  
  double intScale() const;
  double intThreshold() const;
  double intOffset() const;
  bool   showIntegration() const;
  QColor intColor() const;
  QColor intThresholdColor() const;
  int    intLineWidth() const;
  int    intLineMode() const;
  int    intPointMode() const;
  void   setThreshold( double );

public slots:
  virtual void defaultsSLOT();
  virtual void factoryDefaultsSLOT();
  virtual void applySLOT();
  
};

#endif // INTEGRATIONPREFS_HH

