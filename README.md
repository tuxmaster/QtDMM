<!-- GENERATED from docs/ by tests/generate_docs.py - edit the pages in docs/, not this file -->

# QtDMM

**QtDMM** is a simple, cross-platform digital multimeter (DMM) readout application
with a built-in, configurable transient recorder. It's especially useful for users
of older multimeters whose original software no longer runs on modern operating
systems.

QtDMM provides a reliable, open-source alternative to outdated or proprietary DMM
software and runs on current **Linux**, **Windows**, **macOS** and **FreeBSD**
systems.

Website with downloads and the online handbook: **<https://qtdmm.de>**,
contact: <hello@qtdmm.de>.
Source code and bug reports: <https://github.com/tuxmaster/QtDMM>.

## Features

- Transient Recorder
  - Manual start
  - Scheduled start at a specific time
  - Automatic start triggered when defined thresholds are reached
- Analog meter display with auto-ranging scale, dockable or as its own window
- Several meters at once, one window each, recording in sync
- Calculated values across meters (power from voltage and current, ...)
- various connection backends
  - RS232 Serial (USB-Serial and native UART)
  - USB HID-serial support (HOITEK HE2325U & compatible)
  - RFC2217 Remote Serial
  - Sigrok support via sigrok-cli application
- supports lots of DMMs, see [Supported devices](docs/user/supported-devices.md)
  - custom serial settings dialog for yet unknown DMMs

## Key Advantages

- Supports many RS232-based/USB-based multimeters
- Recording via network remote with RFC2217 remote serial support
- No proprietary software required
- Fully **open source** and GPL-V3 licensed
- Easy to use and extend

## Documentation

- **[User Guide](docs/user/index.md)** — connecting a meter, the recorder, CSV
  export, troubleshooting, keyboard shortcuts. The same pages are built into
  QtDMM as its handbook (F1).
- **[Protocols](docs/protocols/index.md)** — the wire protocols of the supported
  meters: which decoder handles which device, the original protocol notes and
  captures, and the test vectors derived from them.
- **[Development](docs/dev/index.md)** — building, testing, and extending QtDMM,
  including how to add support for a new meter.

Build this site locally with `pip install mkdocs` and `mkdocs serve` from the
repository root.

## Supported hardware

Every meter QtDMM can decode, taken from the decoder registrations in
`src/decoders/` (this page is generated from them by
`tests/generate_docs.py`). Choosing one of these models on the DMM settings
page fills in the serial parameters below; meters not listed can often be
used with *Manual settings* if they speak one of the listed protocols - see
[Connecting a meter](docs/user/connecting.md).

*Serial* is baud rate, data bits, parity (N/E/O) and stop bits. *Lines* are
the control lines the cable needs driven. *Counts* is the display
resolution. Not every entry has been confirmed on hardware recently; if you
can confirm one, or get an unlisted meter working, please report it on the
[project page](https://github.com/tuxmaster/QtDMM).

| Vendor | Model | Protocol | Serial | Lines | Counts |
|---|---|---|---|---|---|
| Digitech | QM1350 | `Metex14` | 600 7N2 | DTR | 4000 |
| Digitech | QM1462 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Digitech | QM1537 | `QM1537Continuous` | 2400 8N1 | DTR | 4000 |
| Digitech | QM1538 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Digitek | DT-9062 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Digitek | DT4000ZC | `QM1537Continuous` | 2400 8N1 | DTR | 4000 |
| Digitek | INO2513 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Duratool | DO3122 | `DO3122Continuous` | 9600 8N1 | - | 4000 |
| ELV | M9803R | `M9803RContinuous` | 9600 7E1 | DTR | 4000 |
| Generic | DTM0660 4000 count | `DTM0660` | 2400 8N1 | DTR | 4000 |
| Generic | DTM0660 6000 count | `DTM0660` | 2400 8N1 | DTR | 6000 |
| Generic | DTM0660 8000 count | `DTM0660` | 2400 8N1 | DTR | 8000 |
| HoldPeak | HP-90EPC | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Iso-Tech | IDM 73 | `CyrustekES51986` | 19200 7O1 | DTR | 6000 |
| MASTECH | M9803R | `M9803RContinuous` | 9600 7E1 | DTR | 4000 |
| MASTECH | MAS-343 | `Metex14` | 600 7N2 | DTR | 4000 |
| MASTECH | MAS-345 | `Metex14` | 600 7N2 | DTR | 4000 |
| McVoice | M-345pro | `Metex14` | 600 7N2 | DTR | 4000 |
| McVoice | M-980T | `M9803RContinuous` | 9600 7N1 | DTR | 4000 |
| Metex | M-3660D | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3830D | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3840D | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3850D | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3850M | `Metex14` | 9600 7N2 | DTR | 4000 |
| Metex | M-3870D | `Metex14` | 1200 7N1 | DTR | 4000 |
| Metex | M-4650C | `Metex14` | 1200 7N2 | DTR | 20000 |
| Metex | ME-11 | `Metex14` | 600 7N2 | DTR | 4000 |
| Metex | ME-22 | `Metex14` | 2400 7N2 | DTR | 4000 |
| Metex | ME-32 | `Metex14` | 600 7N2 | DTR | 4000 |
| Metex | ME-42 | `Metex14` | 600 7N2 | DTR | 4000 |
| Metex | universal system 9160 | `Metex14` | 1200 7N2 | DTR | 4000 |
| PeakTech | 3315 | `CyrustekES51962` | 2400 7N1 | DTR | 4000 |
| PeakTech | 3330 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| PeakTech | 3430 | `QM1537Continuous` | 19200 7N2 | DTR | 4000 |
| PeakTech | 4010 | `Metex14` | 9600 7N2 | DTR | 4000 |
| PeakTech | 4015A | `Metex14` | 9600 7N2 | DTR | 100000 |
| PeakTech | 4360 | `Metex14` | 600 7N2 | DTR | 4000 |
| PeakTech | 4390 | `Metex14` | 9600 7N2 | DTR | 4000 |
| PeakTech | 451 | `PeakTech10` | 600 7N2 | DTR | 4000 |
| Radioshack | 22-805 DMM | `Metex14` | 600 7N2 | DTR | 4000 |
| Radioshack | 22-812 | `RS22812Continuous` | 4800 8N1 | DTR | 4000 |
| Radioshack | RS22-168A | `Metex14` | 1200 7N2 | DTR | 4000 |
| Sinometer | MAS-343 | `Metex14` | 600 7N2 | DTR | 4000 |
| TekPower | TP4000ZC | `QM1537Continuous` | 2400 8N1 | DTR | 4000 |
| Tenma | 72-1016 | `CyrustekES51986` | 19200 7O1 | DTR | 6000 |
| Tenma | 72-7732 | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Tenma | 72-7745 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Uni-Trend | UT60A | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Uni-Trend | UT60E | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Uni-Trend | UT61B | `QM1537Continuous` | 2400 8N1 | DTR | 4000 |
| Uni-Trend | UT61C | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Uni-Trend | UT61D | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Uni-Trend | UT61E | `CyrustekES51922` | 19200 7O1 | DTR | 22000 |
| Uni-Trend | UT70B | `CyrustekES51962` | 2400 7N1 | DTR | 4000 |
| Uni-Trend | UT71B | `VC940Continuous` | 2400 7O1 | DTR | 200000 |
| Uni-Trend | UT71CDE | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Uni-Trend | UT803 | `CyrustekES51986` | 19200 7O1 | DTR | 6000 |
| Uni-Trend | UT804 | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Vichy | VC99 | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Voltcraft | M-3610D | `Metex14` | 1200 7N2 | DTR | 4000 |
| Voltcraft | M-3650D | `Metex14` | 1200 7N2 | DTR | 4000 |
| Voltcraft | M-3860 | `Metex14` | 9600 7N2 | DTR | 20000 |
| Voltcraft | M-4650CR | `Voltcraft14Continuous` | 1200 7N2 | DTR | 20000 |
| Voltcraft | M-4660 | `Metex14` | 1200 7N2 | DTR | 50000 |
| Voltcraft | M-4660A | `Metex14` | 9600 7N2 | DTR | 50000 |
| Voltcraft | M-4660M | `Metex14` | 9600 7N2 | DTR | 50000 |
| Voltcraft | ME-11 | `Metex14` | 600 7N2 | DTR | 4000 |
| Voltcraft | ME-22T | `Metex14` | 2400 7N2 | DTR | 4000 |
| Voltcraft | ME-32 | `Metex14` | 600 7N2 | DTR | 4000 |
| Voltcraft | ME-42 | `Metex14` | 600 7N2 | DTR | 4000 |
| Voltcraft | MXD-4660A | `Metex14` | 9600 7N2 | DTR | 50000 |
| Voltcraft | VC 630 | `Voltcraft14Continuous` | 4800 7N1 | DTR | 50000 |
| Voltcraft | VC 635 | `Voltcraft15Continuous` | 2400 7N1 | DTR | 50000 |
| Voltcraft | VC 650 | `Voltcraft14Continuous` | 4800 7N1 | DTR | 50000 |
| Voltcraft | VC 655 | `Voltcraft15Continuous` | 2400 7N1 | DTR | 50000 |
| Voltcraft | VC 670 | `Voltcraft14Continuous` | 4800 7N1 | DTR | 50000 |
| Voltcraft | VC 820 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Voltcraft | VC 840 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Voltcraft | VC 870 | `VC870Continuous` | 9600 8N1 | DTR | 40000 |
| Voltcraft | VC 920 | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Voltcraft | VC 940 | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Voltcraft | VC 960 | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Wintex | TD2200 | `CyrustekES51922` | 19200 7N1 | DTR | 22000 |

83 devices across 19 vendors.

## Command line

```
qtdmm [options]
```

| Option | Meaning |
|---|---|
| `--config-id <id>` | Use the named configuration instead of `default`. Each id has its own settings file, so one meter can be set up per id. |
| `--config-dir <dir>` | Directory for the configuration files (default: the platform's user config location). |
| `--debug` | Print every frame received from the meter as hex to the console. Useful when a meter is not decoded correctly — include this output in a bug report. |
| `-h`, `--help` | Show the options. |
| `-v`, `--version` | Show the version. |

### Several meters at once

Every QtDMM window is one *instance*, identified by its `--config-id`. Start
further instances from **Instances** (Ctrl+N): *Add* asks for a name and
launches a new QtDMM with that id; the list shows which instances are
configured and which are running, and lets you open or remove them. Running
instances know about each other through shared memory — the same mechanism
that stops two windows from using the `default` id at once. An instance can
also compute its value from the others' readings, see
[Calculated values](docs/user/calculated-values.md).

### Debug output

`--debug` writes each received frame as a line of hex bytes, e.g.

```
30 30 30 30 30 31 3B 30 30 30 3A 30 0D 0A
```

These lines are exactly what the decoder test fixtures are made of, so a short
capture together with what the meter displayed at the time is the most useful
thing to attach when reporting a decoding problem.

## Licensing
- **0.9.5 until today tuxmaster and various contributors, see AUTHORS file**
- **0.9.3 and before (c) 2001-2016 M.Toussaint <qtdmm@mtoussaint.de>**

QtDMM 0.9.0 and beyond is distributed under the GNU Public License, version 3.
(Prior to 0.9.0 are licensed under GNU GPL 2.0)
