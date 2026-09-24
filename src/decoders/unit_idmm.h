// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dmmdecoder.h"

/// UNI-T "iDMM" protocol of the Bluetooth meters (UT60BT, UT161A-E) and the
/// UT-D07B adapter of the UT61x+ series: a Microchip/ISSC "Transparent UART"
/// GATT service carrying `AB CD`-framed packets (BleGattDevice).
///
/// QtDMM polls: every request `AB CD 03 5E 01 D9` ('^') is answered by one
/// 19-byte measurement frame
///
///     AB CD 10 | fn | range | 7 x ASCII display | bar hi | bar lo |
///     flags A | flags B | flags C | checksum (16 bit BE sum of bytes 0..16)
///
/// fn selects the function (DCV, OHM, ...), range an ASCII digit that, with
/// fn, gives unit and prefix; the display string carries sign and decimal
/// point exactly as on the LCD. Other frames (the 11-byte name answer) fail
/// the length/checksum test and are skipped.
///
/// Spec with vectors in docs/protocols/spec/unit_idmm.yaml. Sources:
/// ble-multimeter (docs/protocols/uni-t.md, hardware-verified on a UT60BTk)
/// and ut61xpy (adapters/ut61xp.py), both in ablage/.
class DecoderUniTiDMM : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderUniTiDMM(ReadEvent::DataFormat df) : DmmDecoder(df) { m_name = "UniTiDMM"; }

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id) override;
  QByteArray pollRequest() const override;
  bool checkFormat(const char *data, size_t idx) override;
  size_t getPacketLength() override { return kFrameLength; }

  static constexpr int kFrameLength = 19;
  /// True when @p frame (kFrameLength bytes) has the header, the length byte
  /// and a matching checksum.
  static bool frameValid(const unsigned char *frame);
};
