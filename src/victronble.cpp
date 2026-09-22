// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "victronble.h"

#include <QCoreApplication>
#include <QMap>
#include <QRegularExpression>

extern "C" {
#include "3rdparty/tiny-aes/aes.h"
}

std::optional<VictronBle::Advertisement> VictronBle::parse(const QByteArray &d)
{
  // 10 02 | model lo hi | readout type | iv lo hi | key check + ciphertext
  if (d.size() < 9 || quint8(d[0]) != 0x10)
    return std::nullopt;
  Advertisement adv;
  adv.model = quint8(d[2]) | (quint8(d[3]) << 8);
  adv.readoutType = quint8(d[4]);
  adv.iv = quint8(d[5]) | (quint8(d[6]) << 8);
  adv.encrypted = d.mid(7);
  return adv;
}

std::optional<QByteArray> VictronBle::decrypt(const Advertisement &adv, const QByteArray &key)
{
  if (key.size() != 16 || adv.encrypted.isEmpty() || adv.encrypted[0] != key[0])
    return std::nullopt;
  // CTR mode: the 16-byte counter block is the iv as a little-endian number
  quint8 counter[16] = {};
  counter[0] = adv.iv & 0xff;
  counter[1] = adv.iv >> 8;
  AES_ctx ctx;
  AES_init_ctx_iv(&ctx, reinterpret_cast<const quint8 *>(key.constData()), counter);
  QByteArray plain = adv.encrypted.mid(1);
  AES_CTR_xcrypt_buffer(&ctx, reinterpret_cast<quint8 *>(plain.data()), plain.size());
  return plain;
}

QByteArray VictronBle::keyFromHex(const QString &hex)
{
  QString clean = hex;
  clean.remove(QRegularExpression("[\\s:]"));
  if (clean.size() != 32 || !QRegularExpression("^[0-9A-Fa-f]{32}$").match(clean).hasMatch())
    return QByteArray();
  return QByteArray::fromHex(clean.toLatin1());
}

QString VictronBle::modelName(quint16 model)
{
  // victron-ble's product table, the ones one meets on BLE
  static const QMap<quint16, QString> names = {
    { 0xA389, "SmartShunt 500A/50mV" }, { 0xA38A, "SmartShunt 1000A/50mV" }, { 0xA38B, "SmartShunt 2000A/50mV" },
    { 0xA3A4, "Smart Battery Sense" }, { 0xA3A5, "Smart Battery Sense" },
    { 0xA042, "BlueSolar MPPT 75/15" }, { 0xA043, "BlueSolar MPPT 100/15" }, { 0xA044, "BlueSolar MPPT 100/30" },
    { 0xA046, "BlueSolar MPPT 150/70" }, { 0xA047, "BlueSolar MPPT 150/100" }, { 0xA049, "BlueSolar MPPT 100/50 rev2" },
    { 0xA050, "SmartSolar MPPT 250/100" }, { 0xA051, "SmartSolar MPPT 150/100" }, { 0xA052, "SmartSolar MPPT 150/85" },
    { 0xA053, "SmartSolar MPPT 75/15" }, { 0xA054, "SmartSolar MPPT 75/10" }, { 0xA055, "SmartSolar MPPT 100/15" },
    { 0xA056, "SmartSolar MPPT 100/30" }, { 0xA057, "SmartSolar MPPT 100/50" }, { 0xA058, "SmartSolar MPPT 150/35" },
    { 0xA059, "SmartSolar MPPT 150/100 rev2" }, { 0xA05A, "SmartSolar MPPT 150/85 rev2" }, { 0xA05B, "SmartSolar MPPT 250/70" },
    { 0xA05C, "SmartSolar MPPT 250/85" }, { 0xA05D, "SmartSolar MPPT 250/60" }, { 0xA05E, "SmartSolar MPPT 250/45" },
    { 0xA05F, "SmartSolar MPPT 100/20" }, { 0xA060, "SmartSolar MPPT 100/20 48V" }, { 0xA061, "SmartSolar MPPT 150/45" },
    { 0xA062, "SmartSolar MPPT 150/60" }, { 0xA063, "SmartSolar MPPT 150/70" }, { 0xA064, "SmartSolar MPPT 250/85 rev2" },
    { 0xA065, "SmartSolar MPPT 250/100 rev2" }, { 0xA066, "BlueSolar MPPT 100/20" }, { 0xA067, "BlueSolar MPPT 100/20 48V" },
    { 0xA068, "SmartSolar MPPT 250/60 rev2" }, { 0xA069, "SmartSolar MPPT 250/70 rev2" }, { 0xA06A, "SmartSolar MPPT 150/45 rev2" },
    { 0xA06B, "SmartSolar MPPT 150/60 rev2" }, { 0xA06C, "SmartSolar MPPT 150/70 rev2" }, { 0xA06D, "SmartSolar MPPT 150/85 rev3" },
    { 0xA06E, "SmartSolar MPPT 150/100 rev3" }, { 0xA06F, "BlueSolar MPPT 150/45 rev2" }, { 0xA070, "BlueSolar MPPT 150/60 rev2" },
    { 0xA071, "BlueSolar MPPT 150/70 rev2" }, { 0xA075, "SmartSolar MPPT 75/15 rev2" }, { 0xA076, "BlueSolar MPPT 100/30 rev3" },
    { 0xA077, "BlueSolar MPPT 100/50 rev3" }, { 0xA078, "SmartSolar MPPT 100/30 rev2" }, { 0xA079, "SmartSolar MPPT 100/50 rev2" },
    { 0xA102, "SmartSolar MPPT VE.Can 150/70" }, { 0xA103, "SmartSolar MPPT VE.Can 150/45" }, { 0xA104, "SmartSolar MPPT VE.Can 150/60" },
    { 0xA105, "SmartSolar MPPT VE.Can 150/85" }, { 0xA106, "SmartSolar MPPT VE.Can 150/100" }, { 0xA107, "SmartSolar MPPT VE.Can 250/45" },
    { 0xA108, "SmartSolar MPPT VE.Can 250/60" }, { 0xA109, "SmartSolar MPPT VE.Can 250/70" }, { 0xA10A, "SmartSolar MPPT VE.Can 250/85" },
    { 0xA10B, "SmartSolar MPPT VE.Can 250/100" }, { 0xA381, "BMV-712 Smart" }, { 0xA382, "BMV-710H Smart" }, { 0xA383, "BMV-712 Smart Rev2" },
    { 0xA3F0, "Smart BuckBoost 12V/12V-50A" },
    { 0xA200, "Phoenix Inverter" },
    { 0xA201, "Phoenix Inverter 12V 250VA 230V" }, { 0xA202, "Phoenix Inverter 24V 250VA 230V" }, { 0xA204, "Phoenix Inverter 48V 250VA 230V" },
    { 0xA211, "Phoenix Inverter 12V 375VA 230V" }, { 0xA212, "Phoenix Inverter 24V 375VA 230V" }, { 0xA214, "Phoenix Inverter 48V 375VA 230V" },
    { 0xA221, "Phoenix Inverter 12V 500VA 230V" }, { 0xA222, "Phoenix Inverter 24V 500VA 230V" }, { 0xA224, "Phoenix Inverter 48V 500VA 230V" },
    { 0xA231, "Phoenix Inverter 12V 250VA 230V" }, { 0xA232, "Phoenix Inverter 24V 250VA 230V" }, { 0xA234, "Phoenix Inverter 48V 250VA 230V" },
    { 0xA239, "Phoenix Inverter 12V 250VA 120V" }, { 0xA23A, "Phoenix Inverter 24V 250VA 120V" }, { 0xA23C, "Phoenix Inverter 48V 250VA 120V" },
    { 0xA241, "Phoenix Inverter 12V 375VA 230V" }, { 0xA242, "Phoenix Inverter 24V 375VA 230V" }, { 0xA244, "Phoenix Inverter 48V 375VA 230V" },
    { 0xA249, "Phoenix Inverter 12V 375VA 120V" }, { 0xA24A, "Phoenix Inverter 24V 375VA 120V" }, { 0xA24C, "Phoenix Inverter 48V 375VA 120V" },
    { 0xA251, "Phoenix Inverter 12V 500VA 230V" }, { 0xA252, "Phoenix Inverter 24V 500VA 230V" }, { 0xA254, "Phoenix Inverter 48V 500VA 230V" },
    { 0xA259, "Phoenix Inverter 12V 500VA 120V" }, { 0xA25A, "Phoenix Inverter 24V 500VA 120V" }, { 0xA25C, "Phoenix Inverter 48V 500VA 120V" },
    { 0xA261, "Phoenix Inverter 12V 800VA 230V" }, { 0xA262, "Phoenix Inverter 24V 800VA 230V" }, { 0xA264, "Phoenix Inverter 48V 800VA 230V" },
    { 0xA269, "Phoenix Inverter 12V 800VA 120V" }, { 0xA26A, "Phoenix Inverter 24V 800VA 120V" }, { 0xA26C, "Phoenix Inverter 48V 800VA 120V" },
    { 0xA271, "Phoenix Inverter 12V 1200VA 230V" }, { 0xA272, "Phoenix Inverter 24V 1200VA 230V" }, { 0xA274, "Phoenix Inverter 48V 1200VA 230V" },
    { 0xA279, "Phoenix Inverter 12V 1200VA 120V" }, { 0xA27A, "Phoenix Inverter 24V 1200VA 120V" }, { 0xA27C, "Phoenix Inverter 48V 1200VA 120V" },
  };
  return names.value(model, QString("0x%1").arg(model, 4, 16, QLatin1Char('0')).toUpper());
}

QByteArray VictronBle::frame(quint8 readoutType, const QByteArray &plaintext,
                             const QString &mainField, const QString &secondField)
{
  QByteArray line = QByteArray(1, char(readoutType)).toHex() + plaintext.toHex();
  if (!mainField.isEmpty())
    line += ' ' + mainField.toLatin1() + ' ' + (secondField.isEmpty() ? QByteArray("-") : secondField.toLatin1());
  return line + '\n';
}

QList<VictronBle::Field> VictronBle::fields(quint8 readoutType)
{
  switch (readoutType)
  {
    case BatteryMonitor:
      return { { "V", QT_TRANSLATE_NOOP("VictronBle", "Battery voltage") },
               { "I", QT_TRANSLATE_NOOP("VictronBle", "Battery current") },
               { "P", QT_TRANSLATE_NOOP("VictronBle", "Battery power") },
               { "SOC", QT_TRANSLATE_NOOP("VictronBle", "State of charge") },
               { "AH", QT_TRANSLATE_NOOP("VictronBle", "Consumed Ah") },
               { "TTG", QT_TRANSLATE_NOOP("VictronBle", "Time to go") },
               { "AUX", QT_TRANSLATE_NOOP("VictronBle", "Aux input (starter voltage, midpoint or temperature)") } };
    case SolarCharger:
      return { { "PV", QT_TRANSLATE_NOOP("VictronBle", "PV power") },
               { "V", QT_TRANSLATE_NOOP("VictronBle", "Battery voltage") },
               { "I", QT_TRANSLATE_NOOP("VictronBle", "Battery charging current") },
               { "YIELD", QT_TRANSLATE_NOOP("VictronBle", "Yield today") },
               { "LOAD", QT_TRANSLATE_NOOP("VictronBle", "Load current") } };
    case Inverter:
      return { { "VA", QT_TRANSLATE_NOOP("VictronBle", "AC apparent power") },
               { "V", QT_TRANSLATE_NOOP("VictronBle", "Battery voltage") },
               { "VAC", QT_TRANSLATE_NOOP("VictronBle", "AC voltage") },
               { "IAC", QT_TRANSLATE_NOOP("VictronBle", "AC current") } };
    default:
      return {};
  }
}

quint8 VictronBle::readoutTypeForModel(const QString &model)
{
  const QString m = model.toLower();
  if (m.contains("shunt") || m.contains("bmv"))
    return BatteryMonitor;
  if (m.contains("mppt") || m.contains("solar"))
    return SolarCharger;
  if (m.contains("inverter"))
    return Inverter;
  return 0;
}

quint32 VictronBle::BitReader::unsignedBits(int bits)
{
  quint32 value = 0;
  for (int i = 0; i < bits; ++i, ++m_pos)
  {
    const int byte = m_pos / 8;
    if (byte >= m_data.size())
      m_ok = false;
    const bool bit = byte < m_data.size() ? (quint8(m_data[byte]) >> (m_pos % 8)) & 1 : true;
    if (bit)
      value |= quint32(1) << i;
  }
  return value;
}

qint32 VictronBle::BitReader::signedBits(int bits)
{
  const quint32 raw = unsignedBits(bits);
  if (bits < 32 && (raw & (quint32(1) << (bits - 1))))
    return qint32(raw) - qint32(quint32(1) << bits);
  return qint32(raw);
}
