//======================================================================
// File:		displayprefs.h
// Author:	Matthias Toussaint
// Created:	Sun Nov 24 15:08:12 CET 2002
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

#ifndef DISPLAYPREFS_HH
#define DISPLAYPREFS_HH

#include <uidisplayprefs.h>

class DisplayPrefs : public UIDisplayPrefs
{
  Q_OBJECT
public:
  DisplayPrefs( QWidget *parent=0, const char *name=0 );
  virtual ~DisplayPrefs();
  
  bool showBar() const;
  bool showMinMax() const;
  QColor displayBgColor() const;
  QColor displayTextColor() const;
 
public slots:
  virtual void defaultsSLOT();
  virtual void factoryDefaultsSLOT();
  virtual void applySLOT();
  
};

#endif // DISPLAYPREFS_HH

