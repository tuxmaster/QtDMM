//======================================================================
// File:		prefwidget.h
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 14:22:16 CEST 2002
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
#include <QtWidgets>

//class SimpleCfg;
class Settings;

/// Base class of the pages in the settings dialog (ConfigDlg).
///
/// A page owns a group of settings keys. It loads them into its widgets in
/// defaultsSLOT(), writes the widgets back with applySLOT() and resets to the
/// built-in values with factoryDefaultsSLOT(). ConfigDlg lists the pages by
/// label() and pixmap() and calls the three slots for all pages at once.
class PrefWidget : public QWidget
{
  Q_OBJECT
public:
  PrefWidget(QWidget *parent = Q_NULLPTR);
  /// Category name shown in the dialog's list.
  QString label() const { return m_label; }
  QString description() const { return m_description; }
  QPixmap pixmap() const { return *m_pixmap; }
  /// Page id = ConfigDlg::PageType, also the index in the page stack.
  void    setId(int id) { m_id = id; }
  int     id() const { return m_id; }
  void    setCfg(Settings *cfg) { m_cfg = cfg; }

public Q_SLOTS:
  /// Loads the stored settings (or their defaults) into the widgets.
  virtual void defaultsSLOT() = 0;
  /// Resets the widgets to the built-in defaults.
  virtual void factoryDefaultsSLOT() = 0;
  /// Writes the widgets' values to the Settings (staged until save()).
  virtual void applySLOT() = 0;

protected:
  Settings *m_cfg;
  QString   m_label;
  QString   m_description;
  QPixmap  *m_pixmap;
  int       m_id;
};

