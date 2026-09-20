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
/// Size of the byte ring buffer ReaderThread collects frames in. Lives here
/// (not in readerthread.h) because DmmDecoder needs it too.
#define FIFO_LENGTH 100

/// Names the wire protocols QtDMM can decode.
///
/// The enum is the key that ties a DmmDecoder::DMMInfo entry, the decoder
/// class registered for it (DmmDecoder::getInstance) and the settings file
/// together. Values are persisted by name (toString/fromString), never by
/// number, so reordering is harmless as long as EndOfList stays last.
class ReadEvent
{
public:
  /// One entry per decoder; see docs/protocols for the frame formats.
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

  /// Name of a format as used in the settings file and in DMMInfo::protocol.
  static QString toString(DataFormat format)
  {
    const auto it = formatToStringMap().find(format);
    if (it != formatToStringMap().end())
      return it.value();
    return "Invalid";
  }

  /// Inverse of toString(); unknown names give Invalid.
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
