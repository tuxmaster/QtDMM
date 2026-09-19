# Supported devices

Every meter QtDMM can decode, taken from the decoder registrations in
`src/decoders/` (this page is generated from them by
`tests/generate_docs.py`). Choosing one of these models on the DMM settings
page fills in the serial parameters below; meters not listed can often be
used with *Manual settings* if they speak one of the listed protocols - see
[Connecting a meter](connecting.md).

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
