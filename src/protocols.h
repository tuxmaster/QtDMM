// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QString>
#include <memory>
#include <vector>

#include "readevent.h"

class DmmDecoder;

/// Everything QtDMM knows about one wire protocol, in one place: the enum
/// value, its persistent name, the text shown in the protocol combo, the
/// meter chip behind it (for the device table) and the decoder factory.
///
/// The combo in the settings, ReadEvent::toString()/fromString(),
/// DmmDecoder::getInstance() and tests/generate_docs.py (which parses this
/// table) all read from here, so a new protocol is one new row.
struct ProtocolInfo
{
  ReadEvent::DataFormat id;
  const char *name;          ///< persistent name, e.g. "CyrustekES51922"
  const char *description;   ///< combo text; translated with QCoreApplication::translate("Protocols", ...)
  const char *chip;          ///< meter chip, "" when unknown
  /// How the meter is reached when it is not a serial line: "Bluetooth LE",
  /// "USB-HID (BU-86X)", "sigrok-cli". "" means serial, and then the
  /// devices carry a baud rate. Used by the device table, and by the
  /// settings page of a build without Bluetooth to recognise these models.
  const char *transport;
  std::shared_ptr<DmmDecoder> (*create)(ReadEvent::DataFormat);
};

/// All protocols, in the order they are shown.
const std::vector<ProtocolInfo> &protocols();
/// The row for @p id, or null.
const ProtocolInfo *protocolInfo(ReadEvent::DataFormat id);
