//======================================================================
// File:		scaleprefs.h
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 15:33:41 CEST 2002
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

#ifndef SCALEPREFS_HH
#define SCALEPREFS_HH

#include <QtGui>

#include "ui_uiscaleprefs.h"

class ScalePrefs : public UIScalePrefs
{
  Q_OBJECT
	public:
	  ScalePrefs(QWidget *parent=0);

	  bool			automaticScale() const;
	  bool			includeZero() const;
	  double		scaleMin() const;
	  double		scaleMax() const;
	  int			windowSeconds() const;
	  int			totalSeconds() const;

	public Q_SLOTS:
	  virtual void	defaultsSLOT();
	  virtual void	factoryDefaultsSLOT();
	  virtual void	applySLOT();

	  void			setAutoScaleSLOT( bool autoScale );
	  void			zoomInSLOT( double fac );
	  void			zoomOutSLOT( double fac );
	  void			setGraphSizeSLOT( int size, int length );

};

#endif // SCALEPREFS_HH

