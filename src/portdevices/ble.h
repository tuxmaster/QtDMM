// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QIODevice>
#include <QStringList>
#include "dmmdecoder.h"

class QBluetoothDeviceDiscoveryAgent;
class QBluetoothDeviceInfo;

/// Bluetooth LE port for devices that broadcast their readings in their
/// advertisements - today Victron's Instant Readout (SmartShunt, MPPT). No
/// connection is made: a low-energy scan runs for as long as the port is
/// open, advertisements of the configured address are decrypted with the
/// configured key and each new one becomes one text line for the reader
/// (VictronBle::frame). Port string: "<address> <key> [<main> <second>]"
/// with the field ids the decoder should show, e.g.
/// "CB:09:E4:16:33:DB 2ac4... SOC V". finished() reports a lost adapter or a
/// wrong key; silence is left to DMM's watchdog. Built only with
/// QTDMM_WITH_BLE (Qt6::Bluetooth).
class BleAdvertisementDevice : public QIODevice
{
  Q_OBJECT
public:
  explicit BleAdvertisementDevice(const DmmDecoder::DMMInfo &info, const QString &device, QObject *parent = nullptr);
  ~BleAdvertisementDevice() override;

  bool open(OpenMode mode) override;
  void close() override;
  qint64 bytesAvailable() const override;
  bool isSequential() const override { return true; }

  /// Names of Victron devices seen in a short scan, as "<address> <name>";
  /// the settings dialog fills its combo with them. Blocks for @p ms.
  static QStringList scan(int ms = 5000);

Q_SIGNALS:
  /// The scan cannot go on (adapter gone) or the key does not match; the
  /// text says which.
  void finished(const QString &reason);

private:
  void onDevice(const QBluetoothDeviceInfo &info);
  qint64 readData(char *data, qint64 maxSize) override;
  qint64 writeData(const char *data, qint64 len) override;

  QString m_address;
  QByteArray m_key;
  QString m_mainField, m_secondField;
  QBluetoothDeviceDiscoveryAgent *m_agent = nullptr;
  QByteArray m_rx;
  int m_lastIv = -1;
  int m_keyMismatches = 0;
};
