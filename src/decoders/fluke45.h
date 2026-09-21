// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dmmdecoder.h"

/// Fluke 45 bench meter over its RS-232 port (9600 8N1 by default, set on
/// the meter). Polled: QtDMM sends the compound query
/// `FUNC1?;AUTO?;MOD?;VAL1?\r`, the meter answers the four response units in
/// one line separated by semicolons (`VDC;1;0;+1.2345E+0\r\n`) and then its
/// prompt line (`=>`, or `?>`/`!>` on an error). Values are NR3 floats in
/// base units, ±1E+9 is overload. Only the primary display is read.
///
/// Frames are variable-length lines (getPacketLength() 0); prompt lines and
/// a command echo (Echo "On" on the meter) are left in the buffer and skipped
/// by decode(). Sources: Fluke 45 Users Manual chapter 5
/// (docs/protocols/sources/fluke_45_users_manual.pdf), Matthias Toussaint's
/// CDMM fluke45.cpp, libsigrok fluke-45.
class DecoderFluke45 : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderFluke45(ReadEvent::DataFormat df) : DmmDecoder(df) { m_name = "Fluke45"; }

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id) override;
  QByteArray pollRequest() const override { return QByteArrayLiteral("FUNC1?;AUTO?;MOD?;VAL1?\r"); }
  bool checkFormat(const char *data, size_t idx) override;
  size_t getPacketLength() override { return 0; }
};
