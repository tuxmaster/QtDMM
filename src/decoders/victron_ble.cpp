// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "victron_ble.h"

#include "siprefix.h"
#include "victronble.h"

// Bluetooth devices: baud/bits/parity are meaningless, the port string
// carries the address, the key and the chosen fields. Verified live with a
// SmartShunt 500A/50mV, a SmartSolar MPPT 100/20 48V and a Phoenix Inverter
// 12V 500VA (2026-09-21). 6000 counts: the analog meter's full scale then
// fits 12/24/48 V systems (13.23 V -> 60 V); the chargers show whole watts,
// 1000 counts = a 1 kW scale.
static const bool registered = []() {
  DmmDecoder::addConfig({"Victron", "SmartShunt", "", 0, ReadEvent::VictronBLE, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Victron", "BMV-712 Smart *", "", 0, ReadEvent::VictronBLE, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Victron", "SmartSolar MPPT", "", 0, ReadEvent::VictronBLE, 8, 1, 1, 0, 1000, 0, 0, 0});
  DmmDecoder::addConfig({"Victron", "BlueSolar MPPT *", "", 0, ReadEvent::VictronBLE, 8, 1, 1, 0, 1000, 0, 0, 0});
  DmmDecoder::addConfig({"Victron", "Phoenix Inverter Smart", "", 0, ReadEvent::VictronBLE, 8, 1, 1, 0, 6000, 0, 0, 0});
  return true;
}();

namespace
{
using Value = DecoderVictronBLE::Value;

Value value(const char *id, double dval, int decimals, const QString &unit, const QString &special, bool valid = true)
{
  Value v;
  v.id = id;
  v.dval = dval;
  v.val = QString::number(dval, 'f', decimals);
  v.unit = unit;
  v.special = special;
  v.valid = valid;
  return v;
}

Value invalid(const char *id, const QString &unit, const QString &special)
{
  return value(id, 0, 0, unit, special, false);
}
}

std::optional<DmmDecoder::DmmResponse> DecoderVictronBLE::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id = id;
  m_result.showBar = true;

  // "<type hex><plaintext hex> [<main> <second>]\n", see VictronBle::frame()
  const QList<QByteArray> parts = data.trimmed().split(' ');
  const QByteArray bytes = QByteArray::fromHex(parts.value(0));
  if (bytes.size() < 2)
    return std::nullopt;
  const quint8 type = quint8(bytes[0]);
  const QList<Value> all = values(type, bytes.mid(1));
  if (all.isEmpty())
    return std::nullopt;

  auto pick = [&](const QString &wanted, int fallback) -> const Value *
  {
    for (const Value &v : all)
      if (!wanted.isEmpty() && v.id == wanted)
        return &v;
    return wanted.isEmpty() && fallback < all.size() ? &all[fallback] : nullptr;
  };
  const Value *main = pick(QString::fromLatin1(parts.value(1)), 0);
  const QString secondId = QString::fromLatin1(parts.value(2));
  const Value *second = secondId == "-" ? nullptr : pick(secondId, 1);
  if (!main || !main->valid)
    return std::nullopt;   // unknown field, or no reading for it yet

  m_result.dval = main->dval;
  m_result.val = main->val;
  m_result.unit = main->unit;
  m_result.special = main->special;
  if (second && second->valid)
  {
    m_result.dval2 = second->dval;
    m_result.val2 = second->val;
    m_result.unit2 = second->unit;
    m_result.id2 = 1;
  }
  return m_result;
}

QList<Value> DecoderVictronBLE::values(quint8 readoutType, const QByteArray &plain)
{
  switch (readoutType)
  {
    case VictronBle::BatteryMonitor: return batteryMonitor(plain);
    case VictronBle::SolarCharger:   return solarCharger(plain);
    case VictronBle::Inverter:       return inverter(plain);
    default:                         return {};
  }
}

// Remaining time u16 min | voltage s16 /100 V | alarm u16 | aux u16 |
// aux mode 2 | current s22 /1000 A | consumed Ah u20 /10 | SOC u10 /10 %
QList<Value> DecoderVictronBLE::batteryMonitor(const QByteArray &plain)
{
  VictronBle::BitReader r(plain);
  const quint32 ttg = r.unsignedBits(16);
  const qint32 voltage = r.signedBits(16);
  r.unsignedBits(16);                            // alarm reason
  const quint32 aux = r.unsignedBits(16);
  const quint32 auxMode = r.unsignedBits(2);
  const qint32 current = r.signedBits(22);
  const quint32 consumed = r.unsignedBits(20);
  const quint32 soc = r.unsignedBits(10);

  QList<Value> v;
  const bool vOk = voltage != 0x7FFF, iOk = current != 0x1FFFFF;
  v << (vOk ? value("V", voltage / 100.0, 2, "V", "DC") : invalid("V", "V", "DC"));
  v << (iOk ? value("I", current / 1000.0, 3, "A", "DC") : invalid("I", "A", "DC"));
  v << (vOk && iOk ? value("P", voltage / 100.0 * current / 1000.0, 1, "W", "DC") : invalid("P", "W", "DC"));
  v << (soc != 0x3FF ? value("SOC", soc / 10.0, 1, "%", "DC") : invalid("SOC", "%", "DC"));
  v << (consumed != 0xFFFFF ? value("AH", -(consumed / 10.0), 1, "Ah", "DC") : invalid("AH", "Ah", "DC"));
  v << (ttg != 0xFFFF ? value("TTG", ttg, 0, "min", "DC") : invalid("TTG", "min", "DC"));
  switch (auxMode)
  {
    case 0: v << value("AUX", qint16(aux) / 100.0, 2, "V", "DC"); break;            // starter battery
    case 1: v << value("AUX", aux / 100.0, 2, "V", "DC"); break;                    // midpoint
    case 2: v << (aux != 0xFFFF ? value("AUX", aux / 100.0 - 273.15, 1, "C", "TE") // temperature, K -> °C
                                : invalid("AUX", "C", "TE")); break;
    default: v << invalid("AUX", "", "DC"); break;                                  // disabled
  }
  return v;
}

// Charge state u8 | error u8 | battery voltage s16 /100 V | battery current
// s16 /10 A | yield today u16 *10 Wh | PV power u16 W | load current u9 /10 A
QList<Value> DecoderVictronBLE::solarCharger(const QByteArray &plain)
{
  VictronBle::BitReader r(plain);
  r.unsignedBits(8);                             // charge state
  r.unsignedBits(8);                             // charger error
  const qint32 voltage = r.signedBits(16);
  const qint32 current = r.signedBits(16);
  const quint32 yield = r.unsignedBits(16);
  const quint32 power = r.unsignedBits(16);
  const quint32 load = r.unsignedBits(9);

  QList<Value> v;
  v << (power != 0xFFFF ? value("PV", power, 0, "W", "DC") : invalid("PV", "W", "DC"));
  v << (voltage != 0x7FFF ? value("V", voltage / 100.0, 2, "V", "DC") : invalid("V", "V", "DC"));
  v << (current != 0x7FFF ? value("I", current / 10.0, 1, "A", "DC") : invalid("I", "A", "DC"));
  v << (yield != 0xFFFF ? value("YIELD", yield * 10.0, 0, "Wh", "DC") : invalid("YIELD", "Wh", "DC"));
  v << (load != 0x1FF ? value("LOAD", load / 10.0, 1, "A", "DC") : invalid("LOAD", "A", "DC"));
  return v;
}

// Device state u8 | alarm u16 | battery voltage s16 /100 V | AC apparent
// power u16 VA | AC voltage u15 /100 V | AC current u11 /10 A
QList<Value> DecoderVictronBLE::inverter(const QByteArray &plain)
{
  VictronBle::BitReader r(plain);
  r.unsignedBits(8);                             // device state
  r.unsignedBits(16);                            // alarm reason
  const qint32 battery = r.signedBits(16);
  const quint32 apparent = r.unsignedBits(16);
  const quint32 acVoltage = r.unsignedBits(15);
  const quint32 acCurrent = r.unsignedBits(11);

  QList<Value> v;
  v << (apparent != 0xFFFF ? value("VA", apparent, 0, "VA", "AC") : invalid("VA", "VA", "AC"));
  v << (battery != 0x7FFF ? value("V", battery / 100.0, 2, "V", "DC") : invalid("V", "V", "DC"));
  v << (acVoltage != 0x7FFF ? value("VAC", acVoltage / 100.0, 2, "V", "AC") : invalid("VAC", "V", "AC"));
  v << (acCurrent != 0x7FF ? value("IAC", acCurrent / 10.0, 1, "A", "AC") : invalid("IAC", "A", "AC"));
  return v;
}
