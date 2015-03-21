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

#ifndef PREFWIDGET_HH
#define PREFWIDGET_HH

#include <QtGui>

class SimpleCfg;

class PrefWidget : public QWidget
{
  Q_OBJECT
	public:
	  PrefWidget( QWidget *parent=0, const char *name=0 );
	  QString		label() const { return m_label; }
	  QString		description() const { return m_description; }
	  QPixmap		pixmap() const { return *m_pixmap; }

	  void			setId( int id ) { m_id = id; }
	  int			id() const { return m_id; }

	  void			setCfg( SimpleCfg *cfg ) { m_cfg = cfg; }

	public Q_SLOTS:
	  virtual void	defaultsSLOT() = 0;
	  virtual void	factoryDefaultsSLOT() = 0;
	  virtual void	applySLOT() = 0;

	protected:
	  SimpleCfg		*m_cfg;
	  QString		m_label;
	  QString		m_description;
	  QPixmap		*m_pixmap;
	  int			m_id;

};

#endif // PREFWIDGET_HH
