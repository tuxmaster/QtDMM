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

#include <QtCore>

#include "ui_uiexecuteprefs.h"

class ExecutePrefs : public PrefWidget, private Ui::UIExecutePrefs
{
  Q_OBJECT
	public:
	  ExecutePrefs(QWidget *parent=0);
	  bool			startExternal() const;
	  bool			externalFalling() const;
	  double		externalThreshold() const;
	  bool			disconnectExternal() const;
	  QString		externalCommand() const;
	  void			setThreshold( double );

	public Q_SLOTS:
	  virtual void	defaultsSLOT()Q_DECL_OVERRIDE;
	  virtual void	factoryDefaultsSLOT()Q_DECL_OVERRIDE;
	  virtual void	applySLOT()Q_DECL_OVERRIDE;

	protected Q_SLOTS:
	  void			browseExecSLOT();

};

#endif // EXECUTEPREFS_HH

