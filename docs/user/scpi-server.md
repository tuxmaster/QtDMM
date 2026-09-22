# SCPI server: the meter as a network instrument

Bench multimeters have a LAN port and answer SCPI queries - `*IDN?`,
`READ?` - from any program that can open a TCP socket. QtDMM can do the
same for the meter it reads: switch on the **SCPI server** under
**Settings → SCPI server** and a consumer multimeter on a USB cable, a
Victron device over Bluetooth or the virtual meter becomes an instrument
that [lxi-tools](https://github.com/lxi-tools/lxi-tools), LabVIEW,
a Python script or a second QtDMM can query.

The server only reports. It never sends anything to the meter; the meter
keeps deciding what it measures.

## Switching it on

| Setting | Meaning |
|---|---|
| Enable the SCPI server | off by default |
| Port | 5025 is the usual raw-socket SCPI port. When it is taken - a second QtDMM instance, say - the next free one is used; the status bar shows the actual port |
| Listen on | *This computer only* binds to `localhost`; *All network interfaces* opens the port to the network |
| Announce by mDNS | with the port open to the network, QtDMM registers as `_scpi-raw._tcp` so that `lxi discover -m` and every DNS-SD browser list it as "QtDMM *host*" (or "QtDMM *instance*" with `--config-id`) |

The status bar shows *SCPI host:port (n clients)* while the server runs.
There is **no authentication**: whoever can reach the port can read the
meter, start the recorder, connect or disconnect - and fetch a picture of
the QtDMM window (`HCOPy:SDUMp:DATA?`), which shows the readings, the
window title and whatever the readings table holds. Keep *This computer
only* unless the network is yours; from elsewhere use an SSH tunnel
(`ssh -L 5025:localhost:5025 bench`).

## Talking to it

```
$ lxi scpi -a bench.local -p 5025 -r "*IDN?"
QtDMM,Uni-Trend UT61E,0,2026.9.22
$ lxi scpi -a bench.local -p 5025 -r "READ?;UNIT?;CONF?"
+1.234500E+00;"V";"VOLT:DC AUTO"
$ printf 'READ?\n' | nc bench.local 5025
```

```python
import socket
s = socket.create_connection(("bench.local", 5025))
s.sendall(b"READ?\n")
print(float(s.recv(64)))      # 1.2345
```

One program message per line; several commands go on one line separated
by `;` and their answers come back the same way. Mnemonics take the
long or the short form (`MEASure?`, `MEAS?`), case does not matter.

| Command | Answer |
|---|---|
| `*IDN?` | `QtDMM,<meter>,0,<version>` |
| `READ?` `FETCh?` `MEASure?` `MEASure:VOLTage:DC?` `[SENSe:]DATA?` `VALue1?` | the reading in SI base units as `+1.234500E+00`; `9.9E+37` for overload, `9.91E+37` (not a number) when nothing arrived in the last 5 s |
| `READ2?` `FETCh2?` `VALue2?` | the meter's second value (Victron, Fluke 45), same rules |
| `UNIT?` `UNIT2?` | the base unit, `"V"` |
| `CONFigure?` `CONFigure2?` | function and range, `"VOLT:DC AUTO"` - functions `VOLT:DC`, `VOLT:AC`, `CURR:DC`, `CURR:AC`, `RES`, `CONT`, `CAP`, `FREQ`, `TEMP`, `DIOD`, `PER`, `POW` |
| `STATus:QUEStionable?` | bit 0 overload, bit 1 hold, bit 2 meter not connected, bit 3 no current reading |
| `STATus:OPERation?` | 16 while the recorder runs |
| `INITiate` / `ABORt` | starts / stops the recorder |
| `INPut ON|OFF`, `INPut?` | connects / disconnects the meter, asks whether it is |
| `HCOPy:SDUMp:DATA?` | a screenshot of the QtDMM window as PNG (IEEE 488.2 block), like a bench instrument's screen dump |
| `SYSTem:ERRor?` `SYSTem:ERRor:COUNt?` | the error queue: `0,"No error"`, `-113,"Undefined header"`, `-230,"Data corrupt or stale"`, ... |
| `*RST` `*CLS` `*OPC?` `*TST?` `*ESR?` `*STB?` `SYSTem:VERSion?` | the usual IEEE 488.2 replies |

Unknown headers answer nothing and leave `-113,"Undefined header"` in the
queue; a query that could not be answered is skipped in the `;` list. The
complete list of accepted forms, answers and error codes is the
[SCPI command reference](../dev/scpi-commands.md).

## Notes

- Values are in base units: a meter showing `12.34 mV` answers
  `+1.234000E-02`, `UNIT?` says `"V"`.
- The reading is the last one the meter sent; `READ?` does not trigger a
  measurement. Meters deliver two to four readings a second, so polling
  faster than that repeats values.
- Every instance of QtDMM (`--config-id`) has its own server; with the
  same port configured they line up on 5025, 5026, ...
- sigrok's `scpi-dmm` driver identifies instruments by `*IDN?` and does not
  know QtDMM, so it will not talk to this server. lxi-tools, PyVISA and
  plain sockets work; the PyVISA resource string is

  ```
  TCPIP::bench.local::5025::SOCKET
  ```
