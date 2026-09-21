# Meters over the network: qtdmm-bridge

The meters sit at the bench, QtDMM runs at the desk — or on a laptop in
another room. **qtdmm-bridge** is a small program for the box next to the
meters (a Raspberry Pi is enough) that hands them to QtDMM over the network.
QtDMM connects to it with the port type *RFC2217* and sends the meter's line
settings itself, so the bridge needs no knowledge of meters or protocols:
every meter QtDMM supports works through it unchanged.

It is one Python file, `tools/qtdmm-bridge/qtdmm_bridge.py` in the QtDMM
sources, and needs Python 3.11 and the `pyserial` package — nothing else.

## Setting up the bridge

On the box with the meters:

```
sudo apt install python3-serial          # Debian, Raspberry Pi OS
./qtdmm_bridge.py --list                 # what is connected?
./qtdmm_bridge.py --port 4000=/dev/ttyUSB0 --port 4001=/dev/ttyUSB1
```

Each meter gets its own TCP port. `--list` also prints the stable
`/dev/serial/by-id/…` names, which survive re-plugging in another order — use
those for anything permanent.

**USB-HID cables** (UT-D04 type cables, UT803, UT61B/C/D, the two UT-D09
revisions, Brymen's BU-86X) are served the same way on Linux; the address is
the cable's `hid:VID:PID` or `hid:/dev/hidrawN`, as shown by `--list`:

```
./qtdmm_bridge.py --port 4002=hid:1a86:e008
```

The user running the bridge needs access to the devices: group `dialout` for
serial ports, and a udev rule for the HID cables (in the bridge's README).

## Connecting from QtDMM

In QtDMM open **Settings → Special ports**, choose type *RFC2217* and enter
`host:port`, e.g. `raspberry:4000` or `192.168.1.20:4000`. Then, on the
*Multimeter* page, pick that port and the meter model as usual. When QtDMM
connects, the bridge applies the model's baud rate, data bits, parity and
DTR/RTS to the device; the status line shows *Connected 192.168.1.20:4000*
once readings arrive.

Several QtDMM instances can use several bridge ports at once — one meter per
instance, as always — and [calculated values](calculated-values.md) work
across them just like with local meters.

## Running it as a service

For a permanent setup write a configuration file and let systemd start the
bridge at boot. `--print-config` writes a starting point with one entry per
detected device:

```
./qtdmm_bridge.py --print-config | sudo tee /etc/qtdmm-bridge.toml
```

```toml
[bridge]
bind = "0.0.0.0"
mdns = false

[[port]]
name = "UT61E"
tcp = 4000
device = "/dev/serial/by-id/usb-Prolific_Technology_Inc._USB-Serial_Controller-if00-port0"

[[port]]
name = "UT803"
tcp = 4001
device = "hid:1a86:e008"
```

The unit file `qtdmm-bridge.service` next to the script has the installation
steps in its header (copy the script to `/usr/local/bin`, create the service
user, `systemctl enable --now qtdmm-bridge`). `journalctl -u qtdmm-bridge -f`
shows the log.

## What to expect

- **Unplugging a cable** does not drop the connection: the bridge waits for
  the device to come back and QtDMM shows *Timeout* meanwhile. `--strict`
  changes that to dropping the client.
- **One client per port.** A second QtDMM connecting to the same port takes
  over; the first one sees the connection close.
- **Finding the bridge**: with `mdns = true` (or `--mdns`) and the optional
  `zeroconf` package the ports are announced as `_qtdmm-bridge._tcp`, so
  `avahi-browse -rt _qtdmm-bridge._tcp` lists them with address and port.
- **Security**: none. Anyone who can reach the port can read the meter and
  change its line settings. Outside a trusted network bind to `127.0.0.1`
  and reach the bridge through an SSH tunnel
  (`ssh -L 4000:127.0.0.1:4000 pi@raspberry`, then connect QtDMM to
  `localhost:4000`).
- Any other RFC 2217 server (`ser2net`, ESP-Link, …) works with QtDMM as
  well, as long as it implements the remote port setup commands.
