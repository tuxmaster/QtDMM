// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QByteArray>
#include <QString>
#include <optional>

/// Victron Energy "Instant Readout" over Bluetooth LE: the SmartShunt,
/// SmartSolar MPPT and friends broadcast their readings in the manufacturer
/// data of their advertisements (company id 0x02E1), AES-128-CTR encrypted
/// with a per-device key that VictronConnect shows under "Instant readout
/// via Bluetooth". No connection is needed, QtDMM just listens.
///
/// This is the pure part - parsing and decrypting one advertisement - so it
/// can be tested with captured advertisements. BleAdvertisementDevice scans,
/// DecoderVictronBLE turns the plaintext into readings.
///
/// Layout (victron-ble by keshavdv, Victron's "Extra manufacturer data"):
/// `10 02` record type, model id (u16 LE), readout type, iv (u16 LE), then
/// the encrypted bytes whose first byte is the key's first byte (key check).
namespace VictronBle
{
  constexpr quint16 CompanyId = 0x02E1;

  enum ReadoutType : quint8
  {
    SolarCharger = 0x01,
    BatteryMonitor = 0x02,
    Inverter = 0x03,
    DcDcConverter = 0x04,
    SmartLithium = 0x05,
    InverterRs = 0x06,
    AcCharger = 0x08,
    SmartBatteryProtect = 0x09,
    LynxSmartBms = 0x0A,
    MultiRs = 0x0B,
    VeBus = 0x0C,
    DcEnergyMeter = 0x0D,
  };

  struct Advertisement
  {
    quint16 model = 0;
    quint8 readoutType = 0;
    quint16 iv = 0;
    QByteArray encrypted;   ///< key-check byte followed by the ciphertext
  };

  /// Splits the manufacturer data (without the company id) of an Instant
  /// Readout advertisement; nothing for other Victron records.
  std::optional<Advertisement> parse(const QByteArray &manufacturerData);

  /// The plaintext of @p adv, or nothing when @p key (16 bytes) does not
  /// match the key-check byte. AES-128-CTR, counter = iv, little endian.
  std::optional<QByteArray> decrypt(const Advertisement &adv, const QByteArray &key);

  /// "2ac4..." (32 hex digits, blanks and colons ignored) -> 16 bytes; empty
  /// when it is not a key.
  QByteArray keyFromHex(const QString &hex);

  /// Product name for a model id ("SmartShunt 500A/50mV"), or the id in hex.
  QString modelName(quint16 model);

  /// The frame BleAdvertisementDevice hands to the reader, one line: the
  /// readout type and the plaintext as hex, then the ids of the fields
  /// wanted as main and second value ("-" for none). DecoderVictronBLE
  /// reads it back.
  QByteArray frame(quint8 readoutType, const QByteArray &plaintext,
                   const QString &mainField = QString(), const QString &secondField = QString());

  /// One value a readout type carries, as the settings offer it.
  struct Field
  {
    const char *id;      ///< persistent, in the port string ("V", "SOC")
    const char *label;   ///< QT_TRANSLATE_NOOP("VictronBle", ...)
  };
  /// The fields of a readout type, the first one is the default main
  /// value, the second the default second value. Empty for unknown types.
  QList<Field> fields(quint8 readoutType);
  /// Which readout type a model in the device table speaks, from its name
  /// ("SmartShunt" -> BatteryMonitor); 0 when unknown.
  quint8 readoutTypeForModel(const QString &model);

  /// Reads bit fields from a little-endian packed record, LSB first.
  class BitReader
  {
  public:
    explicit BitReader(const QByteArray &data) : m_data(data) {}
    /// The next @p bits bits as an unsigned value; all ones past the end.
    quint32 unsignedBits(int bits);
    /// The next @p bits bits as a two's complement value.
    qint32 signedBits(int bits);
  private:
    QByteArray m_data;
    int m_pos = 0;
  };
}
