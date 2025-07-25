//======================================================================
// File:		configitem.cpp
// Author:	Matthias Toussaint
// Created:	Sat Oct 19 17:38:54 CEST 2002
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


#include <QtGui>
#include <QtWidgets>

#include "configitem.h"

ConfigItem::ConfigItem(int id, const QPixmap &pixmap, const QString &label, QListWidget *parent) :
  QListWidgetItem(label, parent), m_id(id)
{
  setIcon(pixmap);
}


