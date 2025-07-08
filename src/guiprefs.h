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

#pragma once

#include <QtGui>
#include "ui_uiguiprefs.h"

class GuiPrefs : public PrefWidget, private Ui::UIGuiPrefs
{
  Q_OBJECT
public:
  GuiPrefs(QWidget *parent = Q_NULLPTR);
  ~GuiPrefs();
  bool			showTip() const;
  bool			showBar() const;
  bool			showMinMax() const;
  bool			alertUnsavedData() const;
  bool			useTextLabel() const;
  QColor		displayBgColor() const;
  QColor		displayTextColor() const;
  bool			saveWindowPosition() const;
  bool			saveWindowSize() const;
  void			on_ui_tipOfTheDay_toggled(bool on);
  bool			showDmmToolbar() const;
  bool			showGraphToolbar() const;
  bool			showFileToolbar() const;
  bool			showHelpToolbar() const;
  bool			showDisplay() const;
  void			setToolbarVisibility(bool, bool, bool, bool, bool);

Q_SIGNALS:
  void			showTips(bool);

public Q_SLOTS:
  virtual void	defaultsSLOT();
  virtual void	factoryDefaultsSLOT();
  virtual void	applySLOT();

};


