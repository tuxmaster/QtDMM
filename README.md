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
- Virtual meter (sine, square, noise, discharge curve, ...) for demos and testing
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
- Runs several meters side by side and combines them (power from U and I)
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
  including how to add support for a new meter; the Doxygen API
  documentation is at <https://qtdmm.de/api/>.

This site is published at <https://qtdmm.de/docs/>. Build it locally with
`pip install mkdocs` and `mkdocs serve` from the repository root.

## Supported hardware

Every meter QtDMM can decode, taken from the decoder registrations in
`src/decoders/` (this page is generated from them by
`tests/generate_docs.py`). Choosing one of these models on the Multimeter
settings page fills in the serial parameters below; meters not listed can
often be used with *Manual settings* if they speak one of the listed
protocols - the *Chip* column helps: a meter built around the same chip
(named in its manual or on the sigrok wiki) usually speaks the same
protocol. See [Connecting a meter](docs/user/connecting.md).

*Serial* is baud rate, data bits, parity (N/E/O) and stop bits. *Lines* are
the control lines the cable needs driven. *Counts* is the display
resolution. Not every entry has been confirmed on hardware recently; models
marked ¹ were added from chip data (libsigrok, ultradmm.com) and have not
been tried with QtDMM at all. If you can confirm one, or get an unlisted
meter working, please report it on the
[project page](https://github.com/tuxmaster/QtDMM/issues).

| Vendor | Model | Chip | Protocol | Serial | Lines | Counts |
|---|---|---|---|---|---|---|
| APPA | 71 ¹ | ES51986 | `CyrustekES51986` | 19200 7O1 | DTR | 6000 |
| APPA | 73 ¹ | ES51986 | `CyrustekES51986` | 19200 7O1 | DTR | 6000 |
| Brymen | BM250 ¹ | Brymen BM25x | `BrymenBM25x` | 9600 8N1 | DTR RTS | 6000 |
| Brymen | BM251 ¹ | Brymen BM25x | `BrymenBM25x` | 9600 8N1 | DTR RTS | 6000 |
| Brymen | BM252 ¹ | Brymen BM25x | `BrymenBM25x` | 9600 8N1 | DTR RTS | 6000 |
| Brymen | BM257 ¹ | Brymen BM25x | `BrymenBM25x` | 9600 8N1 | DTR RTS | 6000 |
| Digitech | QM1350 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| Digitech | QM1462 | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Digitech | QM1537 | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 4000 |
| Digitech | QM1538 | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Digitek | DT-9062 | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Digitek | DT-9602R+ ¹ | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Digitek | DT4000ZC | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Digitek | INO2513 | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Duratool | DO3122 | - | `DO3122Continuous` | 9600 8N1 | - | 4000 |
| ELV | M9803R | - | `M9803RContinuous` | 9600 7E1 | DTR | 4000 |
| Generic | DTM0660 4000 count | DTM0660 | `DTM0660` | 2400 8N1 | DTR | 4000 |
| Generic | DTM0660 6000 count | DTM0660 | `DTM0660` | 2400 8N1 | DTR | 6000 |
| Generic | DTM0660 8000 count | DTM0660 | `DTM0660` | 2400 8N1 | DTR | 8000 |
| GW Instek | GDM-397 ¹ | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 4000 |
| HoldPeak | HP-90EPC | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Iso-Tech | IDM 73 | ES51986 | `CyrustekES51986` | 19200 7O1 | DTR | 6000 |
| MASTECH | M9803R | - | `M9803RContinuous` | 9600 7E1 | DTR | 4000 |
| MASTECH | MAS-343 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| MASTECH | MAS-345 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| MASTECH | MS8250B ¹ | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| McVoice | M-345pro | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| McVoice | M-980T | - | `M9803RContinuous` | 9600 7N1 | DTR | 4000 |
| Metex | M-3360D ¹ | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3640D ¹ | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3660D | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3830D | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3840D | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3850D | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | M-3850M | Metex KS57C2016 | `Metex14` | 9600 7N2 | DTR | 4000 |
| Metex | M-3870D | Metex KS57C2016 | `Metex14` | 1200 7N1 | DTR | 4000 |
| Metex | M-4650C | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 20000 |
| Metex | ME-11 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| Metex | ME-21 ¹ | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| Metex | ME-22 | Metex KS57C2016 | `Metex14` | 2400 7N2 | DTR | 4000 |
| Metex | ME-32 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| Metex | ME-42 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| Metex | Universal System 9140 ¹ | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metex | universal system 9160 | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Metrel | MD9015 ¹ | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| PCE | PCE-DM32 ¹ | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| PeakTech | 2025 ¹ | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 4000 |
| PeakTech | 3315 | ES51962 | `CyrustekES51962` | 2400 7N1 | DTR | 4000 |
| PeakTech | 3330 | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| PeakTech | 3415 ¹ | DTM0660 | `DTM0660` | 2400 8N1 | DTR | 6000 |
| PeakTech | 3430 | FS9922-DMM4 | `QM1537Continuous` | 19200 7N2 | DTR | 4000 |
| PeakTech | 4010 | Metex KS57C2016 | `Metex14` | 9600 7N2 | DTR | 4000 |
| PeakTech | 4015A | Metex KS57C2016 | `Metex14` | 9600 7N2 | DTR | 100000 |
| PeakTech | 4360 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| PeakTech | 4390 | Metex KS57C2016 | `Metex14` | 9600 7N2 | DTR | 4000 |
| PeakTech | 451 | - | `PeakTech10` | 600 7N2 | DTR | 4000 |
| Pro'sKit | MT-1820 ¹ | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Protek | 504 ¹ | FS9721 LP3 | `VC820Continuous` | 1200 7N2 | DTR RTS | 4000 |
| Protek | 505 ¹ | FS9721 LP3 | `VC820Continuous` | 1200 7N2 | DTR RTS | 4000 |
| Protek | 506 ¹ | FS9721 LP3 | `VC820Continuous` | 1200 7N2 | DTR RTS | 4000 |
| Radioshack | 22-805 DMM | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| Radioshack | 22-812 | - | `RS22812Continuous` | 4800 8N1 | DTR | 4000 |
| Radioshack | RS22-168A | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Sinometer | MAS-343 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| SparkFun | 70C ¹ | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 4000 |
| Tecpel | DMM-8061 ¹ | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| TekPower | TP4000ZC | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Tenma | 72-1016 | ES51986 | `CyrustekES51986` | 19200 7O1 | DTR | 6000 |
| Tenma | 72-6870 ¹ | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Tenma | 72-7730 ¹ | - | `VC940Continuous` | 2400 7O1 | DTR | 20000 |
| Tenma | 72-7732 | - | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Tenma | 72-7745 | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Tenma | 72-9380A ¹ | - | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Uni-Trend | UT30A ¹ | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Uni-Trend | UT30E ¹ | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Uni-Trend | UT60A | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Uni-Trend | UT60D ¹ | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Uni-Trend | UT60E | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Uni-Trend | UT61A ¹ | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Uni-Trend | UT61B | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 4000 |
| Uni-Trend | UT61C | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Uni-Trend | UT61D | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Uni-Trend | UT61E | ES51922 | `CyrustekES51922` | 19200 7O1 | DTR | 22000 |
| Uni-Trend | UT70B | ES51962 | `CyrustekES51962` | 2400 7N1 | DTR | 4000 |
| Uni-Trend | UT71A ¹ | - | `VC940Continuous` | 2400 7O1 | DTR | 20000 |
| Uni-Trend | UT71B | - | `VC940Continuous` | 2400 7O1 | DTR | 200000 |
| Uni-Trend | UT71CDE | - | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Uni-Trend | UT803 | ES51986 | `CyrustekES51986` | 19200 7O1 | DTR | 6000 |
| Uni-Trend | UT804 | - | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| V&A | VA18B ¹ | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| V&A | VA40B ¹ | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Velleman | DVM4100 ¹ | DTM0660 | `DTM0660` | 2400 8N1 | DTR | 6000 |
| Vichy | VC99 | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Voltcraft | GDM 703 ¹ | WENS98A | `GDM703Continuous` | 9600 8N1 | DTR | 4000 |
| Voltcraft | GDM 704 ¹ | WENS98A | `GDM703Continuous` | 9600 8N1 | DTR | 4000 |
| Voltcraft | GDM 705 ¹ | WENS98A | `GDM703Continuous` | 9600 8N1 | DTR | 4000 |
| Voltcraft | M-3610D | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Voltcraft | M-3650D | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Voltcraft | M-3860 | Metex KS57C2016 | `Metex14` | 9600 7N2 | DTR | 20000 |
| Voltcraft | M-4650CR | - | `Voltcraft14Continuous` | 1200 7N2 | DTR | 20000 |
| Voltcraft | M-4660 | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 50000 |
| Voltcraft | M-4660A | Metex KS57C2016 | `Metex14` | 9600 7N2 | DTR | 50000 |
| Voltcraft | M-4660M | Metex KS57C2016 | `Metex14` | 9600 7N2 | DTR | 50000 |
| Voltcraft | ME-11 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| Voltcraft | ME-22T | Metex KS57C2016 | `Metex14` | 2400 7N2 | DTR | 4000 |
| Voltcraft | ME-32 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| Voltcraft | ME-42 | Metex KS57C2016 | `Metex14` | 600 7N2 | DTR | 4000 |
| Voltcraft | MS-9140 ¹ | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Voltcraft | MS-9160 ¹ | Metex KS57C2016 | `Metex14` | 1200 7N2 | DTR | 4000 |
| Voltcraft | MXD-4660A | Metex KS57C2016 | `Metex14` | 9600 7N2 | DTR | 50000 |
| Voltcraft | VC 630 | - | `Voltcraft14Continuous` | 4800 7N1 | DTR | 50000 |
| Voltcraft | VC 635 | - | `Voltcraft15Continuous` | 2400 7N1 | DTR | 50000 |
| Voltcraft | VC 650 | - | `Voltcraft14Continuous` | 4800 7N1 | DTR | 50000 |
| Voltcraft | VC 655 | - | `Voltcraft15Continuous` | 2400 7N1 | DTR | 50000 |
| Voltcraft | VC 670 | - | `Voltcraft14Continuous` | 4800 7N1 | DTR | 50000 |
| Voltcraft | VC 820 | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Voltcraft | VC 830 ¹ | FS9922-DMM4 | `QM1537Continuous` | 2400 8N1 | DTR | 6000 |
| Voltcraft | VC 840 | FS9721 LP3 | `VC820Continuous` | 2400 8N1 | DTR | 4000 |
| Voltcraft | VC 870 | - | `VC870Continuous` | 9600 8N1 | DTR | 40000 |
| Voltcraft | VC 920 | - | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Voltcraft | VC 940 | - | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Voltcraft | VC 960 | - | `VC940Continuous` | 2400 7O1 | DTR | 40000 |
| Wintex | TD2200 | ES51922 | `CyrustekES51922` | 19200 7N1 | DTR | 22000 |

¹ settings taken from the chip, not yet confirmed on hardware with QtDMM.

123 devices across 30 vendors.

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
configured, which are running and what each one currently reads, and lets
you open or remove them. The **ƒ** button creates a calculated instance
from a name, a unit and a formula in one go (see
[Calculated values](docs/user/calculated-values.md)). Running
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
