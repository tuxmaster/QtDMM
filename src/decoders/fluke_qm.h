// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dmmdecoder.h"

/// Fluke handheld meters with the IR serial adapter, polled with "QM\r":
///
///  - 87-IV / 89-IV / 187 / 189 (9600 8N1) answer `0\r` + `QM,+47.66 KOhms\r`
///    (a display string: sign, digits, SI prefix and a unit word such as
///    "V DC", "mV AC+DC", "Ohms", "Farads", "Deg C"; "Out of Range" for OL),
///  - 287 / 289 (115200 8N1) answer `0\r` + `-0.023E-3,VDC,NORMAL,NONE\r`
///    (value in base units, unit token, state, attribute).
///
/// Both families are handled here; the line tells them apart. The frames are
/// variable-length lines (getPacketLength() 0), and the CMD_ACK line ("0",
/// "1", "5") in front of the reading is skipped inside decode(). Spec in
/// docs/protocols/spec/fluke_qm.yaml; sources: the two Fluke Remote Interface
/// Specifications under docs/protocols/sources/ and libsigrok's fluke-dmm.
class DecoderFlukeQM : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderFlukeQM(ReadEvent::DataFormat df) : DmmDecoder(df) { m_name = "FlukeQM"; }

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id) override;
  QByteArray pollRequest() const override { return QByteArrayLiteral("QM\r"); }
  bool checkFormat(const char *data, size_t idx) override;
  size_t getPacketLength() override { return 0; }

private:
  bool decode18x(const QString &reading);
  bool decode28x(const QStringList &fields);
  /// Maps the 18x unit words onto the QtDMM unit/special pair; false for
  /// an unknown word.
  bool unit18x(const QString &word, QString &prefix, QString &unit, QString &special);
};
