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
  std::shared_ptr<DmmDecoder> (*create)(ReadEvent::DataFormat);
};

/// All protocols, in the order they are shown.
const std::vector<ProtocolInfo> &protocols();
/// The row for @p id, or null.
const ProtocolInfo *protocolInfo(ReadEvent::DataFormat id);
