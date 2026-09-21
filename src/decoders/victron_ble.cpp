// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
#include "victron_ble.h"

#include "victronble.h"

// Bluetooth devices: baud/bits/parity are meaningless, the port string
// carries the address and the key. 6000 counts: the analog meter's full
// scale then fits 12/24/48 V systems (13.23 V -> 60 V); the chargers show
// whole watts, 1000 counts = a 1 kW scale. Verified live with a SmartShunt
// 500A/50mV and a SmartSolar MPPT 100/20 48V (2026-09-21).
static const bool registered = []() {
  DmmDecoder::addConfig({"Victron", "SmartShunt", "", 0, ReadEvent::VictronBLE, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Victron", "BMV-712 Smart *", "", 0, ReadEvent::VictronBLE, 8, 1, 1, 0, 6000, 0, 0, 0});
  DmmDecoder::addConfig({"Victron", "SmartSolar MPPT", "", 0, ReadEvent::VictronBLE, 8, 1, 1, 0, 1000, 0, 0, 0});
  DmmDecoder::addConfig({"Victron", "BlueSolar MPPT *", "", 0, ReadEvent::VictronBLE, 8, 1, 1, 0, 1000, 0, 0, 0});
  return true;
}();

std::optional<DmmDecoder::DmmResponse> DecoderVictronBLE::decode(const QByteArray &data, int id)
{
  m_result = {};
  m_result.id = id;
  m_result.showBar = true;
  m_result.special = "DC";

  // "<type hex><plaintext hex>\n", see VictronBle::frame()
  const QByteArray bytes = QByteArray::fromHex(data.trimmed());
  if (bytes.size() < 2)
    return std::nullopt;
  const quint8 type = quint8(bytes[0]);
  const QByteArray plain = bytes.mid(1);

  bool ok = false;
  switch (type)
  {
    case VictronBle::BatteryMonitor: ok = decodeBatteryMonitor(plain); break;
    case VictronBle::SolarCharger:   ok = decodeSolarCharger(plain); break;
    default: break;
  }
  if (!ok)
    return std::nullopt;
  return m_result;
}

// Remaining time u16 min | voltage s16 /100 V | alarm u16 | aux u16 |
// aux mode 2 | current s22 /1000 A | consumed Ah u20 /10 | SOC u10 /10 %
bool DecoderVictronBLE::decodeBatteryMonitor(const QByteArray &plain)
{
  VictronBle::BitReader r(plain);
  r.unsignedBits(16);                            // remaining time
  const qint32 voltage = r.signedBits(16);
  r.unsignedBits(16);                            // alarm reason
  r.unsignedBits(16);                            // aux input
  r.unsignedBits(2);                             // aux mode
  const qint32 current = r.signedBits(22);
  if (voltage == 0x7FFF)
    return false;                                // no reading yet
  m_result.dval = voltage / 100.0;
  m_result.val = QString::number(m_result.dval, 'f', 2);
  m_result.unit = "V";
  if (current != 0x1FFFFF)                       // -2^21 .. 2^21-1, all ones = n/a
  {
    m_result.dval2 = current / 1000.0;
    m_result.val2 = QString::number(m_result.dval2, 'f', 3);
    m_result.unit2 = "A";
    m_result.id2 = 1;
  }
  return true;
}

// Charge state u8 | error u8 | battery voltage s16 /100 V | battery current
// s16 /10 A | yield today u16 *10 Wh | PV power u16 W | load current u9 /10 A
bool DecoderVictronBLE::decodeSolarCharger(const QByteArray &plain)
{
  VictronBle::BitReader r(plain);
  r.unsignedBits(8);                             // charge state
  r.unsignedBits(8);                             // charger error
  const qint32 voltage = r.signedBits(16);
  r.signedBits(16);                              // battery current
  r.unsignedBits(16);                            // yield today
  const quint32 power = r.unsignedBits(16);
  if (power == 0xFFFF)
    return false;
  m_result.dval = power;
  m_result.val = QString::number(power);
  m_result.unit = "W";
  if (voltage != 0x7FFF)
  {
    m_result.dval2 = voltage / 100.0;
    m_result.val2 = QString::number(m_result.dval2, 'f', 2);
    m_result.unit2 = "V";
    m_result.id2 = 1;
  }
  return true;
}
