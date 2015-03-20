//======================================================================
// File:		guiprefs.h
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:29:05 CEST 2002
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

#ifndef GUIPREFS_HH
#define GUIPREFS_HH

#include <uiguiprefs.h>

class GuiPrefs : public UIGuiPrefs
{
  Q_OBJECT
public:
  GuiPrefs( QWidget *parent=0, const char *name=0 );
  virtual ~GuiPrefs();
  
  bool showTip() const;
  bool showBar() const;
  bool showMinMax() const;
  bool alertUnsavedData() const;
  bool useTextLabel() const;
  QColor displayBgColor() const;
  QColor displayTextColor() const;
  bool saveWindowPosition() const;
  bool saveWindowSize() const;
  void setShowTipsSLOT( bool on );
  bool showDmmToolbar() const;
  bool showGraphToolbar() const;
  bool showFileToolbar() const;
  bool showHelpToolbar() const;
  bool showDisplay() const;
  void setToolbarVisibility( bool, bool, bool, bool, bool );
  
signals:
  void showTips( bool );
  
public slots:
  virtual void defaultsSLOT();
  virtual void factoryDefaultsSLOT();
  virtual void applySLOT();
  
};

#endif // GUIPREFS_HH

