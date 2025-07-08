//======================================================================
// File:		dmmbar.h
// Author:	Matthias Toussaint
// Created:	Tue Apr 10 17:42:19 CEST 2001
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
// Copyright (c) 2001 Matthias Toussaint
//======================================================================

#ifndef DMMBAR_HH
#define DMMBAR_HH

#include <QtGui>
#include <QtWidgets>

class DMMBar : public QWidget
{
  Q_OBJECT
public:
  DMMBar(QWidget *parent);

public Q_SLOTS:
  void		setValue(double);

protected:
  double	m_value;
};

#endif // DMMBAR_HH
