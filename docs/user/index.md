# QtDMM Handbook

QtDMM reads a digital multimeter over a serial, USB-HID, RFC2217 or sigrok
connection, shows the live reading on an LCD-style display and an analog
meter, and records it in a transient recorder with thresholds, an
integration curve, CSV export and printing. Several meters run side by
side, one window each, and can be combined into calculated values.

## Contents

- [Connecting a meter](connecting.md) — choosing the device, port types,
  manual settings, the status line.
- [Analog meter](analog-meter.md) — the moving-coil style instrument: scale,
  red zone, readouts, styles.
- [The recorder](recorder.md) — starting manually, at a time or on a threshold;
  zoom, pan and cursor; integration; CSV export and import.
- [Readings table](readings-table.md) — every reading the meter sent, with
  time, mode and range; copy to a spreadsheet or export as CSV.
- [Alarms](alarms.md) — a banner, beep, popup, program or the recorder when
  the reading leaves its range or stops coming.
- [Supported devices](supported-devices.md) — every meter QtDMM knows, with
  protocol and serial settings.
- [Calculated values](calculated-values.md) — power from a voltage and a
  current instance: formulas over the readings of other instances; the
  virtual meter for demos without hardware.
- [Bench meters through sigrok-cli](bench-meters.md) — Keysight, Agilent, HP
  and Siglent SCPI meters over USB-TMC, LAN or serial, with sigrok-cli doing
  the talking.
- [Bluetooth LE](bluetooth.md) — Victron SmartShunt and MPPT chargers over
  their encrypted broadcasts: key from VictronConnect, scan, what is shown.
- [Meters over the network](remote-bridge.md) — qtdmm-bridge on a Raspberry Pi,
  serial and HID meters over RFC 2217, running it as a service.
- [Command line](command-line.md) — options, multiple instances, debug output.
- [Keyboard and mouse](keyboard.md) — shortcuts in the main window and the
  graph.
- [Troubleshooting](troubleshooting.md) — permission errors, "another instance
  is running", HID devices.

Press **Shift+F1** in QtDMM and click any control to get a short explanation
of it right where it is.
