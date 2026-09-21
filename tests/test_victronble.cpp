// Copyright (c) 2026 The QtDMM developers
// SPDX-License-Identifier: GPL-3.0-or-later
//
// VictronBle: parsing and decrypting Instant Readout advertisements, then
// the decoder on the plaintext. Vectors are the public ones from
// victron-ble (keshavdv) - a SmartShunt 500A/50mV and a BlueSolar MPPT
// 75/15 with their keys - plus the bit reader on hand-made bytes.

#include <QtCore>

#include "decoders/victron_ble.h"
#include "victronble.h"

static int failed = 0;

static void check(bool cond, const QString &what)
{
  if (!cond)
  {
    qWarning() << "FAIL:" << what;
    ++failed;
  }
}

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  // --- 1. bit reader: LSB first, little endian, signed ---
  {
    VictronBle::BitReader r(QByteArray::fromHex("ffff0000ff7f"));   // 0xFFFF, 0x0000, 0x7FFF
    check(r.unsignedBits(16) == 0xFFFF, "u16 all ones");
    check(r.signedBits(16) == 0, "s16 zero");
    check(r.signedBits(16) == 0x7FFF, "s16 max");
    check(r.unsignedBits(8) == 0xFF, "past the end reads ones");
    VictronBle::BitReader s(QByteArray::fromHex("fe"));              // 0b11111110
    check(s.unsignedBits(1) == 0 && s.signedBits(7) == -1, "bit split and sign");
    VictronBle::BitReader t(QByteArray::fromHex("ffffff3f"));         // 22 bits of ones = -1
    check(t.signedBits(22) == -1, "s22 minus one");
  }

  // --- 2. key parsing ---
  check(VictronBle::keyFromHex("aff4d0995b7d1e176c0c33ecb9e70dcd").size() == 16, "key from hex");
  check(VictronBle::keyFromHex("AF F4 D0 99 5B 7D 1E 17 6C 0C 33 EC B9 E7 0D CD").size() == 16, "key with blanks");
  check(VictronBle::keyFromHex("aff4d0995b7d1e176c0c33ecb9e70d").isEmpty(), "short key rejected");
  check(VictronBle::keyFromHex("zzf4d0995b7d1e176c0c33ecb9e70dcd").isEmpty(), "non-hex rejected");

  // --- 3. SmartShunt advertisement (victron-ble test vector) ---
  DecoderVictronBLE decoder(ReadEvent::VictronBLE);
  {
    const QByteArray adv = QByteArray::fromHex("100289a302b040af925d09a4d89aa0128bdef48c6298a9");
    const QByteArray key = VictronBle::keyFromHex("aff4d0995b7d1e176c0c33ecb9e70dcd");
    const auto parsed = VictronBle::parse(adv);
    check(parsed.has_value(), "shunt advertisement parses");
    if (parsed)
    {
      check(parsed->model == 0xA389 && parsed->readoutType == VictronBle::BatteryMonitor, "model and readout type");
      check(VictronBle::modelName(parsed->model) == "SmartShunt 500A/50mV", "model name");
      check(parsed->iv == 0x40B0, QString("iv %1").arg(parsed->iv, 0, 16));
      check(!VictronBle::decrypt(*parsed, VictronBle::keyFromHex("00f4d0995b7d1e176c0c33ecb9e70dcd")).has_value(),
            "wrong key fails the key check");
      const auto plain = VictronBle::decrypt(*parsed, key);
      check(plain.has_value(), "decrypts with the right key");
      if (plain)
      {
        const auto r = decoder.decode(VictronBle::frame(parsed->readoutType, *plain), 0);
        check(r.has_value(), "battery monitor decodes");
        if (r)
        {
          check(qFuzzyCompare(r->dval, 12.53) && r->val == "12.53" && r->unit == "V" && r->special == "DC",
                QString("voltage 12.53 V: %1 %2").arg(r->val, r->unit));
          check(r->id2 == 1 && qFuzzyCompare(r->dval2 + 1, 1.0) && r->unit2 == "A", "current 0 A as second value");
        }
      }
    }
  }

  // --- 4. BlueSolar MPPT advertisement (victron-ble test vector) ---
  {
    const QByteArray adv = QByteArray::fromHex("100242a0016207adceb37b605d7e0ee21b24df5c");
    const QByteArray key = VictronBle::keyFromHex("adeccb947395801a4dd45a2eaa44bf17");
    const auto parsed = VictronBle::parse(adv);
    check(parsed && parsed->model == 0xA042 && parsed->readoutType == VictronBle::SolarCharger, "mppt parses");
    const auto plain = parsed ? VictronBle::decrypt(*parsed, key) : std::nullopt;
    check(plain.has_value(), "mppt decrypts");
    if (plain)
    {
      const auto r = decoder.decode(VictronBle::frame(VictronBle::SolarCharger, *plain), 0);
      check(r.has_value(), "solar charger decodes");
      if (r)
      {
        check(r->dval == 19.0 && r->val == "19" && r->unit == "W", QString("PV power 19 W: %1 %2").arg(r->val, r->unit));
        check(r->id2 == 1 && qFuzzyCompare(r->dval2, 13.88) && r->val2 == "13.88" && r->unit2 == "V",
              QString("battery 13.88 V as second value: %1 %2").arg(r->val2, r->unit2));
      }
    }
  }

  // --- 5. other records and junk ---
  check(!VictronBle::parse(QByteArray::fromHex("0269b907109a")).has_value(), "non-readout record ignored");
  check(!VictronBle::parse(QByteArray()).has_value(), "empty ignored");
  check(!decoder.decode("03" + QByteArray(16, 'a') + "\n", 0).has_value(), "unknown readout type gives nothing");
  check(!decoder.decode("zz\n", 0).has_value(), "junk line gives nothing");
  // a battery monitor with no voltage yet
  check(!decoder.decode(VictronBle::frame(VictronBle::BatteryMonitor, QByteArray::fromHex("ffffff7fffffffffffffffffffffff")), 0).has_value(),
        "n/a voltage gives nothing");

  if (failed == 0)
    qInfo() << "All Victron BLE tests passed.";
  else
    qWarning() << failed << "Victron BLE test(s) failed.";
  return failed == 0 ? 0 : 1;
}
