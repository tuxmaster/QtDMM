# Protocols

Every supported meter speaks one of the wire protocols below, each handled by one
decoder class in `src/decoders/`. This section collects what is known about
those protocols, where that knowledge comes from, and how it is turned into
regression tests.

## Layout

- `sources/` — the original protocol notes, captures and datasheets, kept
  unchanged. The `*.log` files pair raw frame bytes with the reading the meter
  displayed at that moment, which makes them ground truth for tests.
- `spec/` — one YAML file per protocol with curated test vectors derived from
  those sources. `tests/generate_fixtures.py` turns them into the fixtures under
  `tests/data/decoder/` that `test_decoder` runs.

## Decoders, devices and sources

| Decoder | Protocol (`ReadEvent`) | Devices | Captures in `sources/` | Test vectors |
|---|---|---|---|---|
| `ascii.cpp` | `Metex14`, `PeakTech10`, `Voltcraft14Continuous`, `Voltcraft15Continuous`, `Sigrok` | 41 devices: Metex M-3xxx/M-46xx/ME-xx, Voltcraft M-36xx/M-46xx/ME-xx/VC 6xx, PeakTech 4xxx/451, Mastech MAS-34x, Radioshack 22-805/RS22-168A, Digitech QM1350, McVoice M-345pro, Sinometer MAS-343 | `metex-22t.log` (Voltcraft ME-22T) | Metex14: yes |
| `brymen_bm52x.cpp` | `BrymenBM52x`, `BrymenBM82x` | Brymen BM525s, BM829s over the BU-86X adapter (unconfirmed) | — (layout from libsigrok's `bm52x.c`, live readings only) | synthetic |
| `brymen_bm86x.cpp` | `BrymenBM86x` | Brymen BM867s, BM869s over the BU-86X adapter (unconfirmed) | — (layout from libsigrok's `bm86x.c`) | synthetic |
| `brymen_bm25x.cpp` | `BrymenBM25x` | Brymen BM250/251/252/257 (unconfirmed) | — (layout from libsigrok's `bm25x.c`) | synthetic |
| `cyrustek_es51922.cpp` | `CyrustekES51922` | Uni-Trend UT61E, Wintex TD2200 | `UT61E.log` | yes |
| `cyrustek_es51962.cpp` | `CyrustekES51962` | PeakTech 3315, Uni-Trend UT70B | `UT70B.log` | yes |
| `cyrustek_es51986.cpp` | `CyrustekES51986` | Iso-Tech IDM 73, Tenma 72-1016, Uni-Trend UT803 | `UT803.log` | yes |
| `do3122.cpp` | `DO3122Continuous` | Duratool DO3122 | — | none |
| `fluke_qm.cpp` | `FlukeQM` | Fluke 87-IV/89-IV/187/189 (9600 Bd) and 287/289 (115200 Bd) over the IR serial adapter (unconfirmed) | `fluke_18x_remote.pdf`, `fluke_28x_remote.pdf` (Fluke's Remote Interface Specifications; no capture) | yes, from the specifications' examples |
| `gdm703.cpp` | `GDM703Continuous` | Voltcraft GDM 703/704/705 (unconfirmed; chip WENS98A per the [sigrok IC list](https://sigrok.org/wiki/Multimeter_ICs)) | — (layout from Toussaint's CDMM, `ablage/CDMM`) | synthetic |
| `dtm0660.cpp` | `DTM0660` | Generic DTM0660 (4000/6000/8000 count) | — | none |
| `m9803r.cpp` | `M9803RContinuous` | ELV M9803R, MASTECH M9803R, McVoice M-980T | — | none |
| `qm1537.cpp` | `QM1537Continuous` | Digitek DT4000ZC, Digitech QM1537, PeakTech 3430, TekPower TP4000ZC, Uni-Trend UT61B/C/D, Vichy VC99 | `UT61BCD.log` | yes |
| `rs22812.cpp` | `RS22812Continuous` | Radioshack 22-812 | — | none |
| `vc820.cpp` | `VC820Continuous` | Digitek DT-9062/INO2513, Digitech QM1462/QM1538, HoldPeak HP-90EPC, PeakTech 3330, Tenma 72-7745, Uni-Trend UT60A/UT60E, Voltcraft VC 820/840 | `UT60AE.log` | yes |
| `vc870.cpp` | `VC870Continuous` | Voltcraft VC 870 | — | none |
| `vc940.cpp` | `VC940Continuous` | Tenma 72-7732, Uni-Trend UT71B/UT71CDE/UT804, Voltcraft VC 920/940/960 | `UT71BCDE.log`, `UT804.log` | yes |

`FlukeQM` is the one polled protocol with variable-length frames: QtDMM sends
`QM\r` once a second, the meter answers with a CMD_ACK line (`0\r`) and one
reading line. The reader hands everything since the previous frame to the
decoder (`getPacketLength()` 0), which takes the last line.

Two protocol families transmit every telegram twice in a row (UT803, UT70B, and
the VC940 per the original implementation); their decoders deliberately accept
only the second copy so one measurement yields one reading. The specs record
this as `frames_per_reading: 2`.

## Datasheets and protocol documents in `sources/`

- `ES51962.PDF`, `ES51981.pdf`, `ES51986.pdf` — Cyrustek chip datasheets for
  the corresponding decoders.
- `UT803+command+instructions.pdf` — Uni-Trend's protocol notes for the UT803.
- `vc870_protocol.pdf` — the VC870 protocol description.
- `dtm0660-brochure.pdf`, `duratool.pdf` — material for the DTM0660 and DO3122
  decoders.
- `fluke_18x_remote.pdf`, `fluke_28x_remote.pdf` — Fluke's Remote Interface
  Specifications for the 189/187/89-IV/87-IV and the 289/287; the `QM`
  examples in them are the test vectors of `spec/fluke_qm.yaml`.
- `ES232.pdf`, `FS9922-DMM4-DS-15_EN.pdf`, `BM250-BM250s-6000-count-digital-multimeters-r1.pdf`
  — chip and meter documents whose mapping to a decoder has not been confirmed
  here; do not assume one without checking.

## Captures for meters without a decoder

`UT109.log`, `UT233.log`, `UT321.log`, `UT325.log`, `UT60FG.log`, `UT70D.log`
and `UT81B.log` document Uni-Trend meters that QtDMM does not support yet.
They are not usable as tests today, but are the starting material for adding
those devices.

## Provenance

The decoders largely originate with QtDMM's original author, Matthias
Toussaint (<http://www.mtoussaint.de/qtdmm.html>), and were extended from
several external sources. The Uni-Trend protocol logs (`UT*.log`) are the work
of heha, author of the Windows read-out tool DMM.EXE, published at
<https://heha.fwh.is/hs/UNI-T/>; the source of that tool is an independent
implementation of the same protocols and was used to cross-check several of
the specs here. `metex-22t.log` was contributed by Simon Hartmann, as noted in
the file itself. Further protocol knowledge was taken from the sigrok DMM
protocol parsers (<https://sigrok.org/>, `libsigrok/src/dmm/`), which served as
a third reference wherever the logs and heha's tool left room for doubt. The
datasheets are the respective manufacturers' documents. When a decoder looks wrong, consult these sources
first — several apparent bugs (such as the frame deduplication above) turned
out to be deliberate once the notes were read.
