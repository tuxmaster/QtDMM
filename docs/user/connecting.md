# Connecting a meter

## Choosing the device

Open the settings with **F2** (or the *Configure* button) and go to the **DMM**
page.

1. Pick the **vendor** in the first box. The second box then lists only that
   vendor's models. *All vendors* shows the complete list; *Manual settings*
   lets you enter the serial parameters yourself.
2. Pick the **model**. Baud rate, data bits, parity, stop bits, protocol and
   display resolution are filled in from QtDMM's device table and locked. You
   can also start typing into the model box; the search covers every vendor
   and switches the vendor box for you.
3. Choose the **port** (see below).
4. Set **DTR** / **RTS** if the meter's cable needs them powered — the
   defaults come from the device table.

Manual settings are for meters that are not in the table yet. If you find a
combination that works, please report it on the project page so it can be
added — see [Supported devices](supported-devices.md).

## Port types

The port box lists everything QtDMM found, prefixed with its type:

| Prefix | Backend | Typical entry |
|---|---|---|
| `Serial` | RS-232 and USB-serial adapters | `/dev/ttyUSB0`, `/dev/ttyS0` (Linux), `/dev/cuaU0` (FreeBSD), `COM3` (Windows) |
| `HID` | USB-HID cables (Hoitek HE2325U and compatible, as used by many Uni-Trend meters) | `HID 0x1a86:0xe008 /dev/hidraw2` (Linux), `HID 0x1a86:0xe008 \\?\hid#…` (Windows) |
| `RFC2217` | serial port on another machine, via an RFC 2217 server | `localhost:4000` |
| `Sigrok` | any meter that `sigrok-cli` supports | `scpi-dmm:conn=/dev/ttyUSB0` |

RFC2217 and sigrok entries are not detected automatically. Add them under
**Settings → Custom ports**: choose the type and type the host:port or the
`sigrok-cli` driver string. For sigrok, `sigrok-cli --help` and the
[sigrok hardware list](https://sigrok.org/wiki/Supported_hardware#Multimeters)
tell you the driver string; the path to `sigrok-cli` can be set on the same
page if it is not in your `PATH`.

Any program that fully implements RFC 2217 (including remote port setup) works
as the server. `assets/rfc2217_server.py` from the QtDMM sources is a
convenient one: `rfc2217_server.py -p 4000 /dev/ttyUSB0`.

## Connecting

Click **Connect** (Ctrl+C) in the toolbar. The status line at the bottom shows
what is happening:

- *Connecting …* — the port is open, waiting for the first frame.
- *Connected /dev/ttyUSB0* — readings arrive; the display and the recorder are
  live.
- *Timeout on device …* — the port is open but nothing arrives. Check that the
  meter is switched on, that its RS-232 output is enabled (many meters have a
  button or menu entry for it), and DTR/RTS.
- *No permission to access …* — see [Troubleshooting](troubleshooting.md).

Clicking **Connect** again disconnects and frees the port for other programs.

## The display

The LCD-style display mirrors the meter: value, unit, the annunciators HOLD,
AUTO, MANU, AC, DC, diode and continuity (unlit ones stay faintly visible,
like on the meter itself), the bar graph and, below the value, the minimum
and maximum since the last **Reset** (Ctrl+R). Min/max reset automatically
when the meter switches to a different unit.

The display is a panel like the [analog meter](analog-meter.md): drag its
title bar to dock it on another side of the window or to pull it out as a
separate window, and resize it — digits and lettering scale with it.
**Display** in the toolbar or the menu hides and shows it. Bar graph, min/max
and the LCD tint are set on the GUI settings page.
