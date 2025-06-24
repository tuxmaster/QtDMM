//======================================================================
// File:		engnumbervalidator.cpp
// Author:	Matthias Toussaint
// Created:	Fri Oct 11 20:32:01 CEST 2002
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

#include <QRegExp>

#include <math.h>
#include "engnumbervalidator.h"

EngNumberValidator::EngNumberValidator(QObject *parent) : QValidator(parent)
{
}

// TODO: RegExp is still a simple hack
//
QValidator::State EngNumberValidator::validate(QString &input, int &pos) const
{
  static QRegExp doubleRe("-?\\d+\\.?\\d*");
  static QRegExp fullRe("-?\\d+\\.?\\d*[munpkMGT]?");

  input = input.trimmed();
  pos = qMin(static_cast<int>(input.length()), pos);

  // mt: changed
  if (fullRe.exactMatch(input))
    return Acceptable;
  if (doubleRe.exactMatch(input))
    return Intermediate;

  return Invalid;
}

double EngNumberValidator::value(const QString &string)
{
  double factor = 1.;

  // mt: added toAscii
  switch (string[string.length() - 1].toLatin1())
  {
    case 'm':
      factor = 1e-3;
      break;
    case 'u':
      factor = 1e-6;
      break;
    case 'n':
      factor = 1e-9;
      break;
    case 'p':
      factor = 1e-12;
      break;
    case 'k':
      factor = 1e3;
      break;
    case 'M':
      factor = 1e6;
      break;
    case 'G':
      factor = 1e9;
      break;
    case 'T':
      factor = 1e12;
      break;
  }

  return string.toDouble() * factor;
}

QString EngNumberValidator::engValue(double value)
{
  QString suffix = "";

  if (fabs(value) < 1.)
  {
    value *= 1000;
    suffix = "m";
  }
  if (fabs(value) < 1.)
  {
    value *= 1000;
    suffix = "µ";
  }
  if (fabs(value) < 1.)
  {
    value *= 1000;
    suffix = "n";
  }
  if (fabs(value) < 1.)
  {
    value *= 1000;
    suffix = "p";
  }
  if (fabs(value) >= 1000.)
  {
    value /= 1000;
    suffix = "k";
  }
  if (fabs(value) >= 1000.)
  {
    value /= 1000;
    suffix = "M";
  }
  if (fabs(value) >= 1000.)
  {
    value /= 1000;
    suffix = "G";
  }
  if (fabs(value) >= 1000.)
  {
    value /= 1000;
    suffix = "T";
  }

  QString str;
  str.setNum((static_cast<int>(qRound(value * 10.))) / 10.);
  str += suffix;

  return str;
}
