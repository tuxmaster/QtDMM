//======================================================================
// File:		engnumbervalidator.h
// Author:	Matthias Toussaint
// Created:	Fri Oct 11 20:29:37 CEST 2002
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

#ifndef ENGNUMBERVALIDATOR_HH
#define ENGNUMBERVALIDATOR_HH

#include <QtCore>
#include <QtGui>

class EngNumberValidator : public QValidator
{
	public:
	  EngNumberValidator( QObject *parent=0);
	  QValidator::State	validate( QString &, int & ) const Q_DECL_OVERRIDE;

	  static double		value( const QString & );
	  static QString	engValue( double );

};

#endif // ENGNUMBERVALIDATOR_HH
