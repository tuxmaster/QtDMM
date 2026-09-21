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

# a meter on a USB-HID cable (UT-D04/UT803 type, UT-D09, Brymen BU-86X), Linux
./qtdmm_bridge.py --port 4001=hid:1a86:e008
```

## USB-HID cables

Many Uni-T meters (UT-D04 cable, UT803, UT61B/C/D, ...), the two UT-D09
revisions and Brymen's BU-86X adapter are USB-HID devices, not serial ports.
The bridge serves them like serial ports: it reads the HID reports directly
from `/dev/hidrawN` (no extra library), unpacks the UART bytes and, where the
chip takes one, turns the line settings QtDMM sends into the cable's feature
report. In QtDMM the port is again an RFC2217 entry; the meter model is chosen
as usual. Chips: CH9325/HE2325U (receive only, baud rate set by feature
report), CP2110 (full UART), CH9329 (fixed 9600 8N1), BU-86X (fixed).

Address a cable by `hid:VID:PID` (the first one attached) or by its node
`hid:/dev/hidraw2`; `--list` prints both. Linux only.

The user needs read/write access to the hidraw node. A udev rule does it:

```
# /etc/udev/rules.d/60-qtdmm-bridge.rules
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="e008", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="04fa", ATTRS{idProduct}=="2490", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea80", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="1a86", ATTRS{idProduct}=="e429", MODE="0660", GROUP="plugdev"
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="0820", ATTRS{idProduct}=="0001", MODE="0660", GROUP="plugdev"
```

then `sudo udevadm control --reload && sudo udevadm trigger` and re-plug the cable.

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
./qtdmm_bridge.py --print-config > qtdmm-bridge.toml   # one entry per detected device, then edit
./qtdmm_bridge.py -c /etc/qtdmm-bridge.toml
```

### As a service (systemd)

`qtdmm-bridge.service` has the steps in its header; in short:

```bash
sudo cp qtdmm_bridge.py /usr/local/bin/qtdmm-bridge && sudo chmod +x /usr/local/bin/qtdmm-bridge
qtdmm-bridge --print-config | sudo tee /etc/qtdmm-bridge.toml     # then edit names/ports
sudo useradd --system --no-create-home --groups dialout,plugdev qtdmm-bridge
sudo cp qtdmm-bridge.service /etc/systemd/system/
sudo systemctl daemon-reload && sudo systemctl enable --now qtdmm-bridge
journalctl -u qtdmm-bridge -f
```

The unit runs unprivileged with access to the serial and hidraw device nodes
only; for a UART on other pins add a `DeviceAllow=` line.

### Finding the bridge (mDNS)

With `mdns = true` in the configuration (or `--mdns`) each port is announced
as a `_qtdmm-bridge._tcp` service - address, port, device and name in the TXT
record. This needs the `zeroconf` package (`pip install zeroconf` or
`apt install python3-zeroconf`); without it the bridge logs a warning and
carries on. `avahi-browse -rt _qtdmm-bridge._tcp` shows what is announced.

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

- HID cables on Windows/macOS (hidraw is Linux); serial ports work everywhere.
- QtDMM does not browse mDNS itself yet; the address is typed in.

## Tests

```bash
python3 -m unittest discover -s tests      # in tools/qtdmm-bridge, no hardware needed
```

The QtDMM test suite (`ctest`) runs these too, as `qtdmm_bridge`, and checks
the QtDMM side with `rfc2217_client`.
