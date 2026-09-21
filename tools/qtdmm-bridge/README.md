# qtdmm-bridge

Serve multimeters on a small box - a Raspberry Pi next to the bench, an old
laptop in the lab - to QtDMM running somewhere else on the network.

QtDMM talks RFC 2217 ("telnet COM port control"): it opens a TCP connection
and sends the meter's line settings itself - baud rate, data bits, parity,
stop bits, DTR and RTS. The bridge therefore knows nothing about meters or
protocols; it is a dumb pipe, and every meter QtDMM supports works through
it unchanged.

Requirements: Python 3.11 or newer, [pyserial](https://pypi.org/project/pyserial/).
A single file, no other dependencies:

```bash
sudo apt install python3-serial        # Debian, Raspberry Pi OS
scp qtdmm_bridge.py pi@raspberry:
```

## Usage

```bash
# what is connected?
./qtdmm_bridge.py --list

# one meter on TCP port 4000, a second one on 4001
./qtdmm_bridge.py --port 4000=/dev/ttyUSB0 --port 4001=/dev/ttyUSB1

# stable names survive re-plugging in another order
./qtdmm_bridge.py --port 4000=/dev/serial/by-id/usb-Prolific_Technology_Inc._USB-Serial_Controller-if00-port0
```

In QtDMM: *Settings → Special ports*, type **RFC2217**, address
`raspberry:4000`; then choose the meter model as usual. The line settings of
the model are applied on the bridge when QtDMM connects.

### Configuration file

For more than a port or two, or for a service, use a TOML file:

```toml
[bridge]
bind = "0.0.0.0"        # 127.0.0.1 to allow local (SSH-tunnelled) clients only

[[port]]
name = "UT61E"
tcp = 4000
device = "/dev/serial/by-id/usb-Prolific_Technology_Inc._USB-Serial_Controller-if00-port0"

[[port]]
name = "bench PSU meter"
tcp = 4001
device = "/dev/ttyUSB1"
# Optional defaults, used until QtDMM sends its own:
# baudrate = 2400
# bytesize = 8
# parity = "N"        # N, E, O
# stopbits = 1
# dtr = true
# rts = false
```

```bash
./qtdmm_bridge.py -c /etc/qtdmm-bridge.toml
./qtdmm_bridge.py --port 4000=/dev/ttyUSB0 --print-config > qtdmm-bridge.toml   # a starting point
```

## Behaviour

- **One client per port.** A new connection replaces the old one, so a QtDMM
  that reconnects after a network hiccup gets in immediately.
- **Hot-plug.** When the USB cable disappears the TCP connection stays up and
  the bridge waits for the device to come back; QtDMM shows *Timeout* until
  then. `--strict` drops the client instead.
- **Line settings** the port refuses (a pty has no 7-bit frames, a bare UART
  has no DTR line) are logged once and otherwise ignored - the meter may work
  anyway.
- `-v` prints the negotiation; `--version`, `--help` as usual.

## Security

There is none. Anyone who can reach the TCP port can read the meter and
change its line settings. On anything but a trusted LAN bind to `127.0.0.1`
and use an SSH tunnel:

```bash
ssh -L 4000:127.0.0.1:4000 pi@raspberry
```

## Permissions (Linux)

The user running the bridge needs access to the serial device, normally
group `dialout`:

```bash
sudo usermod -aG dialout $USER     # log out and in again
```

## Not yet

- USB-HID meter cables (UT-D04/UT803, UT-D09, Brymen BU-86X) - planned; the
  bridge already lists them.
- A systemd unit and mDNS announcement - planned.

## Tests

```bash
python3 -m unittest discover -s tests      # in tools/qtdmm-bridge, no hardware needed
```

The QtDMM test suite (`ctest`) runs these too, as `qtdmm_bridge`, and checks
the QtDMM side with `rfc2217_client`.
