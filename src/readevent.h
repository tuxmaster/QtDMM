//======================================================================
// File:		readevent.h
// Author:	Matthias Toussaint
// Created:	Sat Apr 14 13:01:28 CEST 2001
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

#pragma once

#include <QtCore>
// moved from readerthread, so this is accessibly in dmmdriver.
// should be solved in another way ...
#define FIFO_LENGTH 100

class ReadEvent
{
public:
  enum DataFormat
  {
    Invalid = -1,
    Metex14 = 0,
    PeakTech10,
    Voltcraft14Continuous,
    Voltcraft15Continuous,
    M9803RContinuous,
    VC820Continuous,
    CyrustekES51986,
    VC940Continuous,
    QM1537Continuous,
    RS22812Continuous,
    VC870Continuous,
    DO3122Continuous,
    CyrustekES51922,
    DTM0660,
    CyrustekES51962,
    Sigrok,
    EndOfList              // new stuff always before!
  };

  static QString toString(DataFormat format)
  {
    const auto it = formatToStringMap().find(format);
    if (it != formatToStringMap().end())
      return it.value();
    return "Invalid";
  }

  static DataFormat fromString(const QString& str)
  {
    for (auto it = formatToStringMap().cbegin(); it != formatToStringMap().cend(); ++it)
    {
      if (it.value() == str)
        return it.key();
    }
    return Invalid;
  }

private:
  static const QMap<DataFormat, QString>& formatToStringMap()
  {
    static const QMap<DataFormat, QString> map = {
      { Metex14,               "Metex14" },
      { PeakTech10,            "PeakTech10" },
      { Voltcraft14Continuous, "Voltcraft14Continuous" },
      { Voltcraft15Continuous, "Voltcraft15Continuous" },
      { M9803RContinuous,      "M9803RContinuous" },
      { VC820Continuous,       "VC820Continuous" },
      { CyrustekES51986,       "CyrustekES51986" },
      { VC940Continuous,       "VC940Continuous" },
      { QM1537Continuous,      "QM1537Continuous" },
      { RS22812Continuous,     "RS22812Continuous" },
      { VC870Continuous,       "VC870Continuous" },
      { DO3122Continuous,      "DO3122Continuous" },
      { CyrustekES51922,       "CyrustekES51922" },
      { DTM0660,               "DTM0660" },
      { CyrustekES51962,       "CyrustekES51962" }
    };
    return map;
  }
};
