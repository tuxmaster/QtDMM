//======================================================================
// File:		dmmprefs.h
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:08:57 CEST 2002
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

#include <QtSerialPort>

#include "ui_uidmmprefs.h"
#include "readevent.h"

class DmmPrefs : public PrefWidget, private Ui::UIDmmPrefs
{
  Q_OBJECT
public:
  DmmPrefs(QWidget *parent = Q_NULLPTR);
  ~DmmPrefs();
  QSerialPort::Parity parity() const;
  QSerialPort::DataBits bits() const;
  QSerialPort::StopBits stopBits() const;
  int            speed() const;
  int            numValues() const;
  bool           externalSetup() const;
  bool           rts() const;
  bool           cts() const;
  bool           dsr() const;
  bool           dtr() const;
  ReadEvent::DataFormat format() const;
  int            display() const;
  QString        dmmName() const;
  QString        device() const;
  QString        deviceListText() const;

public Q_SLOTS:
  virtual void   defaultsSLOT() Q_DECL_OVERRIDE;
  virtual void   factoryDefaultsSLOT() Q_DECL_OVERRIDE;
  virtual void   applySLOT() Q_DECL_OVERRIDE;

protected Q_SLOTS:
  void           on_ui_model_activated(int);
  void           on_ui_load_clicked();
  void           on_ui_save_clicked();
  void           on_ui_externalSetup_toggled();

protected:
  QString        m_path;

private:
  QStringListModel *m_portlist;
};
