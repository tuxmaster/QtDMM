# Bluetooth LE: Victron Instant Readout

Victron Energy's SmartShunt, BMV-712 Smart and the SmartSolar / BlueSolar
MPPT chargers with Bluetooth broadcast their readings in their Bluetooth LE
advertisements ("Instant Readout"). QtDMM listens to those broadcasts - no
connection, no pairing - and shows them like a meter's reading:

| Device | Main value | Second value |
|---|---|---|
| SmartShunt, BMV-712 | battery voltage (V DC) | battery current (A, negative when discharging) |
| SmartSolar, BlueSolar MPPT | PV power (W) | battery voltage (V) |

The second value goes to the display's second line and to the
[readings table](readings-table.md) (as a `2nd` row); the main value drives
the recorder and the analog meter. About one reading per second arrives.

## Setting it up

The broadcasts are encrypted with a key that only the device's owner has.
In the VictronConnect app open the device, then **Settings → Product info →
Instant readout via Bluetooth** and tap **Show** under *Instant readout
details* (or *Encryption data*): it shows the **MAC address** and the
**encryption key** (32 hex digits). Instant readout must be enabled there.

In QtDMM, **Settings → Multimeter**: choose vendor **Victron** and the
model. The port and serial settings make way for a **Bluetooth** group:

- **Device** - press **Scan** to list the Victron devices in range (five
  seconds; their names as in VictronConnect), or type the address.
- **Key** - the 32-digit key. It is stored in QtDMM's settings file and
  never shown in the status line.

Then **Connect** as usual. The status line says *Connected* once the first
advertisement decrypted. With the wrong key it reports *The encryption key
does not match* after a few advertisements; when the device goes out of
range or is switched off the usual timeout appears and QtDMM keeps
listening.

## Requirements

A Bluetooth LE adapter and a QtDMM built with Qt's Bluetooth module
(`QTDMM_WITH_BLE`, on by default when Qt6 Bluetooth is found - on Debian /
Ubuntu that is the `qt6-connectivity-dev` package at build time and
`libqt6bluetooth6` at run time). Linux uses BlueZ over D-Bus; QtDMM does
not need root. Without the module the Victron models are listed but cannot
be connected.

## What is not there yet

Only the main and second value are shown; state of charge, consumed Ah,
time to go, yield and the aux input (starter voltage / temperature) are
decoded but not displayed. Other Victron products (inverters, DC-DC
converters, Smart Lithium) broadcast in the same way and can follow once
someone has one to test with.
