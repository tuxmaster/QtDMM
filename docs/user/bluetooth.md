# Bluetooth LE

QtDMM reads two kinds of Bluetooth LE meters:

- **UNI-T UT60BT** (and, untested, the UT161B/D/E and the UT61B+/D+/E+
  with the UT-D07B adapter) - QtDMM *connects* to the meter and asks it for
  readings, like a serial meter on a cable.
- **Victron** SmartShunt, BMV-712, MPPT chargers and Phoenix inverters - QtDMM
  *listens* to the readings they broadcast, without a connection.

## UNI-T UT60BT

The UT60BT has Bluetooth built in. Switch it on with the meter's Bluetooth
key (the Bluetooth symbol appears on the LCD); the meter switches it off
again after a while without a connection.

In QtDMM, **Settings → Multimeter**: choose vendor **Uni-Trend** and the
model **UT60BT**. Instead of the port box a **Bluetooth** group appears:

- **Device** - press **Scan** to list the UT60BT meters in range (six
  seconds), or type the address. The meter advertises as *UT60BT*.

Then **Connect**. QtDMM connects to the meter, asks it for a reading about
three times a second and shows what the LCD shows - value, unit and range,
AC/DC, HOLD, manual/auto range, the diode and continuity symbols and OL.
When the meter goes out of range or switches Bluetooth off, the status line
says so and QtDMM connects again as soon as the meter is back.

Only one program can be connected to the meter at a time: close the UNI-T
app or any other tool first.

Like the meter itself, QtDMM shows 0 in the dead bands the UT60BT keeps
around zero (about ±10 µV in the mV position, ±5 mA on the current
ranges). The rotary switch is mechanical; changing it produces one frame
with the new function and the old number, which QtDMM passes on like the
meter's own display does.

## UNI-T UT61B+ / UT61D+ / UT61E+ and UT161B / D / E

These meters speak the same protocol as the UT60BT. The **UT161** have
Bluetooth built in: choose the model and set it up exactly like the UT60BT.
The UT61+ have no Bluetooth of their own; either use the **UT-D09 USB cable** that comes with them -
choose the model *UT61E+* (or B+/D+) and the cable's HID port, as for any
[USB-HID cable](connecting.md) - or the **UT-D07B Bluetooth adapter**:
choose *UT61E+ (UT-D07B Bluetooth)* and set it up like the UT60BT above
(the adapter advertises as *UT-D07B*).

In the DC V position with AC shown alongside (the meter's DC+AC mode), the
meter sends DC and AC readings in turn: QtDMM records the DC reading and
shows the AC one on the display's second line and in the readings table.

Not tried with a real UT61+ or UT161 yet. Their ranges come from the tables
of UNI-T's own app (preserved by the
[unit_ut61eplus](https://github.com/ljakob/unit_ut61eplus) project) and agree
with [ut61xpy](https://github.com/olegv142/ut61xpy); the same tables confirm
what a UT60BT sends. Please report how it works for you.

## Victron Instant Readout

Victron Energy's SmartShunt, BMV-712 Smart, the SmartSolar / BlueSolar
MPPT chargers and the Phoenix Inverter Smart broadcast their readings in their Bluetooth LE
advertisements ("Instant Readout"). QtDMM listens to those broadcasts - no
connection, no pairing - and shows them like a meter's reading:

Each device broadcasts several values; you choose which one QtDMM treats
as the reading (**Main value**: display, analog meter, recorder) and which
goes to the display's second line and the [readings table](readings-table.md)
as a `2nd` row (**Second value**, or none):

| Device | Values |
|---|---|
| SmartShunt, BMV-712 | battery voltage (V), battery current (A, negative when discharging), battery power (W, computed), state of charge (%), consumed Ah, time to go (min), aux input (starter or midpoint voltage in V, or temperature in °C - as configured on the device) |
| SmartSolar, BlueSolar MPPT | PV power (W), battery voltage (V), battery charging current (A), yield today (Wh), load current (A) |
| Phoenix Inverter Smart | AC apparent power (VA), battery voltage (V), AC voltage (V), AC current (A) |

The defaults are the first two of each row. About one reading per second
arrives. Run several QtDMM instances on the same device to record more
than two of its values at once ([instances](command-line.md)).

### Setting it up

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
- **Main value** / **Second value** - which of the device's values to show
  (table above).

Then **Connect** as usual. The status line says *Connected* once the first
advertisement decrypted. With the wrong key it reports *The encryption key
does not match* after a few advertisements; when the device goes out of
range or is switched off the usual timeout appears and QtDMM keeps
listening.

### What is not there yet

The device state (eco mode, bulk/absorption/float), alarms and charger
errors are decoded but not shown. Other Victron products (DC-DC converters,
Smart Lithium, Multi RS, AC chargers) broadcast in the same way and can
follow once someone has one to test with.

## Requirements

A Bluetooth LE adapter and a QtDMM built with Qt's Bluetooth module
(`QTDMM_WITH_BLE`, on by default when Qt6 Bluetooth is found - on Debian /
Ubuntu that is the `qt6-connectivity-dev` package at build time and
`libqt6bluetooth6` at run time). Linux uses BlueZ over D-Bus; QtDMM does
not need root. Windows builds include Bluetooth as well. Without the module
the Bluetooth models are listed but cannot be connected; the settings page
says so.

**macOS:** not supported yet. macOS does not tell programs the Bluetooth
address of a device, and QtDMM finds and addresses the meters by that
address, so the macOS build leaves Bluetooth out (`QTDMM_WITH_BLE` is off by
default there).
