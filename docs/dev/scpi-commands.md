# SCPI command reference

Everything the [SCPI server](../user/scpi-server.md) understands, as
implemented in `src/scpiserver.cpp` (`ScpiServer::handle()`) and checked by
`tests/test_scpi.cpp`. The server is an instrument that only *reports*: no
command changes what the meter measures.

## Message format

- Transport: raw TCP, one **program message per line** (`\n`, `\r\n` also
  accepted), no VXI-11, no HiSLIP.
- Several commands per message separated by `;`. Their query answers come
  back in one line, separated by `;`, terminated by `\n`. A message without
  a query gets no answer at all.
- **Header prefix rule** (SCPI-99 7.2): after `SYST:ERR?` a following
  `VERS?` means `SYST:VERS?`; a header starting with `:` or `*` is absolute.
- Mnemonics in **long or short form**, case-insensitive: `MEASure?`,
  `MEAS?`, `meas?`. The short form is the upper-case part of the long form.
- A **numeric suffix** on the last mnemonic selects the value: `READ?` =
  `READ1?` = main value, `READ2?` = the meter's second value (Victron,
  Fluke 45). Other suffixes answer `-114,"Header suffix out of range"`.
- Numbers are **NR3** in SI base units: `+1.234500E+00`, six decimals.
  `9.9E+37` is +INF (overload), `-9.9E+37` −INF, `9.91E+37` is NaN (no
  usable reading: not connected, nothing received for 5 s, or no second
  value).
- Strings are double-quoted: `"V"`, `"VOLT:DC AUTO"`.

## IEEE 488.2 common commands

| Command | Answer | Notes |
|---|---|---|
| `*IDN?` | `QtDMM,<meter>,0,<version>` | `<meter>` is the model chosen in the settings (`UNI-T UT61E`), `<version>` the QtDMM version. Manufacturer field is `QtDMM`, so clients can tell it from a real bench meter |
| `*RST` | – | clears the error queue; nothing else to reset |
| `*CLS` | – | clears the error queue |
| `*OPC?` | `1` | always complete |
| `*OPC`, `*WAI` | – | accepted, no effect |
| `*TST?` | `0` | self-test passed |
| `*ESR?`, `*STB?`, `*ESE?`, `*SRE?` | `0` | no status registers; `*ESE n` / `*SRE n` are accepted and ignored |

## Readings

| Command | Answer | Notes |
|---|---|---|
| `READ?`, `FETCh?`, `MEASure?`, `MEASure:SCALar?` | NR3 | the last reading the meter sent; `READ?` does not trigger a measurement |
| `MEASure:<function>[:<type>]?` (e.g. `MEAS:VOLT:DC?`, `MEAS:RES?`) | NR3 | accepted for compatibility with instrument-oriented clients; the function is **ignored** – the meter decides what it measures. Check `CONF?` |
| `[SENSe:]DATA?` | NR3 | same as `READ?` |
| `VALue1?`, `VALue2?` | NR3 | Fluke 45 style names for main and second value |
| `READ2?`, `FETCh2?`, `MEASure2?`, `DATA2?` | NR3 | the second value |
| `UNIT?`, `UNIT2?` | `"V"`, `"A"`, `"Ohm"`, `"F"`, `"Hz"`, `"°C"`, `"%"`, … | base unit without SI prefix |
| `CONFigure?`, `CONFigure2?` | `"<function> <range>"` | see below |

`CONFigure?` derives the function from the meter's mode and unit:

| Function | When |
|---|---|
| `VOLT:DC` / `VOLT:AC` | unit V, mode DC / AC |
| `CURR:DC` / `CURR:AC` | unit A |
| `RES` | unit Ω |
| `CONT` | continuity (beeper mode) |
| `CAP` | unit F |
| `FREQ` | unit Hz |
| `TEMP` | temperature |
| `DIOD` | diode test |
| `PER` | unit % (duty cycle, Victron SoC) |
| `POW` | unit W |
| `PWID` | unit s |
| other | the unit in upper case; `NONE` when nothing is known |

The range is `AUTO`, `MANUAL` or the meter's own range text.

## Status

| Command | Answer | Notes |
|---|---|---|
| `STATus:QUEStionable?`, `STATus:QUEStionable:CONDition?`, `…:EVENt?` | integer | bit 0 (1) overload · bit 1 (2) hold · bit 2 (4) meter not connected · bit 3 (8) no current reading (nothing for 5 s) |
| `STATus:OPERation?`, `…:CONDition?`, `…:EVENt?` | integer | bit 4 (16) *measuring*: the recorder is running |
| `INPut?`, `INPut:STATe?` | `1` / `0` | meter connected? |
| `SYSTem:VERSion?` | `1999.0` | SCPI version |

## Control

| Command | Effect |
|---|---|
| `INITiate`, `INITiate:IMMediate` | starts the recorder (like the Start button) |
| `ABORt` | stops the recorder |
| `INPut ON`, `INPut 1`, `INPut:STATe ON` | connects the meter (like the Connect button) |
| `INPut OFF`, `INPut 0` | disconnects the meter |

## Screen dump

| Command | Answer |
|---|---|
| `HCOPy:SDUMp:DATA?`, `DISPlay:DATA?` | the main window as an image in an IEEE 488.2 definite-length block: `#` + number of length digits + length + bytes, e.g. `#676714<PNG…>`, then `\n` |
| `HCOPy:SDUMp:DATA:FORMat PNG|BMP|JPG`, `…:FORMat?` | image format, PNG by default |

The same command Keysight and Rigol instruments use for their screenshots,
so a generic "grab screen" routine works. Read the block, not a line:

```python
s.sendall(b"HCOP:SDUM:DATA?\n")
head = s.recv(2)                       # b"#6"
length = int(s.recv(int(head[1:2])))
png = b""
while len(png) < length:
    png += s.recv(length - len(png))
```

(`lxi screenshot` and the lxi-gui button talk VXI-11 only and pick a
plugin by `*IDN?`; both are on the list for a later round.)

## Error queue

| Command | Answer |
|---|---|
| `SYSTem:ERRor?`, `SYSTem:ERRor:NEXT?` | the oldest entry, `0,"No error"` when empty |
| `SYSTem:ERRor:COUNt?` | number of entries |

The queue holds 20 entries; when full, the last one becomes
`-350,"Queue overflow"`. `*CLS` and `*RST` empty it. Codes used:

| Code | Text | When |
|---|---|---|
| `-100` | Command error | a query used as a command (`READ`) or the other way round, an unparsable header |
| `-109` | Missing parameter | `INPut` without ON/OFF |
| `-113` | Undefined header | unknown command; the query is skipped in the answer line |
| `-114` | Header suffix out of range | suffix other than 1 or 2 |
| `-224` | Illegal parameter value | `INPut MAYBE` |
| `-230` | Data corrupt or stale | a reading query answered `9.91E+37` |
| `-240` | Hardware error | `HCOPy:SDUMp:DATA?` when no window is there to dump |
| `-350` | Queue overflow | more than 20 errors unread |
| `-363` | Input buffer overrun | 64 KB without a line terminator; the buffer was discarded |

## Examples

```
*IDN?                        → QtDMM,Uni-Trend UT61E,0,2026.9.22
READ?;UNIT?;CONF?            → +1.234500E+00;"V";"VOLT:DC AUTO"
READ2?                       → 9.91E+37            (UT61E has no second value)
SYST:ERR?;VERS?              → -230,"Data corrupt or stale";1999.0
INIT;STAT:OPER?              → 16
:ABOR;:STAT:OPER?            → 0
FOO?;READ?                   → +1.234500E+00       (FOO? skipped, -113 queued)
```

Not implemented, on purpose: `TRIGger`, `SAMPle`, `CALCulate`, range and
function setters (`CONF:VOLT:DC 20`) – the meters QtDMM reads have no
remote control – and the `SYSTem:COMMunicate` tree.
