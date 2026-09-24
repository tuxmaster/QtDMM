// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QBluetoothUuid>
#include <QIODevice>
#include <QLowEnergyCharacteristic>
#include <QStringList>

#include "dmmdecoder.h"

class QLowEnergyController;
class QLowEnergyService;
class QTimer;

/// A Bluetooth LE meter reached over a GATT connection - the counterpart of
/// BleAdvertisementDevice, which only listens to broadcasts. It connects to
/// the address, finds the meter's serial-like service, subscribes to its
/// notify characteristic and hands every notification on as received bytes;
/// what the reader thread writes (the decoder's poll request) goes to the
/// write characteristic. The protocol stays in the decoder.
///
/// Which service and characteristics a meter uses comes from the protocol
/// (profile()); the UNI-T iDMM meters (UT60BT, UT161) use the Microchip/ISSC
/// "Transparent UART" service. Port string: "<address>". open() returns at
/// once and the connection is set up in the background; a lost link emits
/// finished() and DMM reconnects as for the other port types.
///
/// Polled meters answer one request with one frame. The reader asks once a
/// second; to follow the meter's own update rate the device repeats the last
/// request as soon as an answer has come in, at most every kRepollMs.
class BleGattDevice : public QIODevice
{
  Q_OBJECT
public:
  /// The GATT layout of one protocol family.
  struct Profile
  {
    QBluetoothUuid service;
    QBluetoothUuid notify;
    QList<QBluetoothUuid> write;   ///< in order of preference
    QStringList namePrefixes;      ///< for scan(): advertised names that belong to it
  };
  /// The profile for @p format, or nullopt when it is not a GATT protocol.
  static std::optional<Profile> profile(ReadEvent::DataFormat format);

  explicit BleGattDevice(const DmmDecoder::DMMInfo &info, const QString &device, QObject *parent = nullptr);
  ~BleGattDevice() override;

  bool open(OpenMode mode) override;
  void close() override;
  qint64 bytesAvailable() const override;
  bool isSequential() const override { return true; }

  /// True once the meter has confirmed the notification subscription and the
  /// write characteristic is known; until then open() has returned but the
  /// link is still being set up (up to 20 s). The last poll written before
  /// is sent on becoming ready.
  bool isReady() const { return m_ready; }

  /// Shortest time between two repeated polls.
  static constexpr int kRepollMs = 300;

  /// Meters of @p format seen in a short scan, as "<address> <name>". Blocks
  /// for @p ms.
  static QStringList scan(ReadEvent::DataFormat format, int ms = 6000);

Q_SIGNALS:
  /// The link is gone or could not be set up; the text says why.
  void finished(const QString &reason);

private:
  void onServiceDiscovered(const QBluetoothUuid &uuid);
  void onDiscoveryFinished();
  void onServiceState();
  void setReady();
  void fail(const QString &reason);
  qint64 readData(char *data, qint64 maxSize) override;
  qint64 writeData(const char *data, qint64 len) override;

  QString m_address;
  std::optional<Profile> m_profile;
  QLowEnergyController *m_controller = nullptr;
  QLowEnergyService *m_service = nullptr;
  QLowEnergyCharacteristic m_writeChar;
  QTimer *m_connectTimeout = nullptr;
  QTimer *m_repoll = nullptr;
  QByteArray m_lastPoll;
  void sendPoll();
  QByteArray m_rx;
  bool m_serviceFound = false;
  bool m_ready = false;
};
