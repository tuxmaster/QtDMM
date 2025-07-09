//======================================================================
// File:		recorderprefs.h
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 14:34:19 CEST 2002
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

#include "ui_uirecorderprefs.h"
#include "dmmgraph.h"

class RecorderPrefs : public PrefWidget, private Ui::UIRecorderPrefs
{
  Q_OBJECT
public:
  RecorderPrefs(QWidget *parent = Q_NULLPTR);
  ~RecorderPrefs();
  DMMGraph::SampleMode sampleMode() const;
  int         sampleStep() const;
  int         sampleLength() const;
  double      fallingThreshold() const;
  double      raisingThreshold() const;
  QTime       startTime() const;
  void        setThreshold(double);

public Q_SLOTS:
  virtual void  defaultsSLOT()Q_DECL_OVERRIDE;
  virtual void  factoryDefaultsSLOT()Q_DECL_OVERRIDE;
  virtual void  applySLOT()Q_DECL_OVERRIDE;
  void          setSampleTimeSLOT(int sampleTime);
};


