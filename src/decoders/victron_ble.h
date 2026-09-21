// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "dmmdecoder.h"

/// Victron Energy devices over Bluetooth LE "Instant Readout": SmartShunt
/// / BMV-712 (battery monitor), SmartSolar / BlueSolar MPPT (solar
/// charger) and Phoenix Inverter Smart (inverter). BleAdvertisementDevice
/// listens to the advertisements and decrypts them (src/victronble.h);
/// this decoder gets one line per advertisement - readout type and
/// plaintext as hex, then the ids of the fields wanted as main and second
/// value - and turns the packed bit fields into readings. Without field
/// ids the first two fields of the type are taken (VictronBle::fields()).
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

  /// One decoded value of a record.
  struct Value
  {
    QString id;        ///< VictronBle::Field::id
    double dval = 0;   ///< base units
    QString val;       ///< display form
    QString unit;      ///< with prefix
    QString special;   ///< "DC", "AC", "TE", ...
    bool valid = false;
  };
  /// Every field of a record, in VictronBle::fields() order; empty for an
  /// unknown readout type.
  static QList<Value> values(quint8 readoutType, const QByteArray &plain);

private:
  static QList<Value> batteryMonitor(const QByteArray &plain);
  static QList<Value> solarCharger(const QByteArray &plain);
  static QList<Value> inverter(const QByteArray &plain);
};
