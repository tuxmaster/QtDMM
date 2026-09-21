// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dmmdecoder.h"

/// Victron Energy devices over Bluetooth LE "Instant Readout": SmartShunt
/// / BMV-712 (battery monitor) and SmartSolar / BlueSolar MPPT (solar
/// charger). BleAdvertisementDevice listens to the advertisements and
/// decrypts them (src/victronble.h); this decoder gets one line per
/// advertisement - the readout type and the plaintext as hex - and turns
/// the packed bit fields into readings:
///
///  - battery monitor: battery voltage (main), battery current (second value)
///  - solar charger: PV power (main), battery voltage (second value)
///
/// Bit layouts from victron-ble (keshavdv); spec in
/// docs/protocols/spec/victron_ble.yaml.
class DecoderVictronBLE : public DmmDecoder
{
  Q_OBJECT
public:
  DecoderVictronBLE(ReadEvent::DataFormat df) : DmmDecoder(df) { m_name = "VictronBLE"; }

  std::optional<DmmDecoder::DmmResponse> decode(const QByteArray &data, int id) override;
  bool checkFormat(const char *data, size_t idx) override { return data[idx] == '\n'; }
  size_t getPacketLength() override { return 0; }

private:
  bool decodeBatteryMonitor(const QByteArray &plain);
  bool decodeSolarCharger(const QByteArray &plain);
};
