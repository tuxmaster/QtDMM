# Troubleshooting

## "No permission to access /dev/ttyUSB0"

On Linux, serial ports belong to a group, and your user has to be in it. QtDMM
names the group in the error message; it is `dialout` on Debian, Ubuntu and
their relatives and `uucp` on Arch Linux and CachyOS. Add yourself with

```
sudo usermod -aG dialout $USER
```

(or `uucp`), then **log out and back in** — group membership is only read at
login.

## "Timeout on device" although the meter is on

- Many meters only send data after their RS-232 or USB function is switched on
  — look for a button labelled RS232, PC, USB or a serial symbol.
- Some cables draw their power from the DTR or RTS line. Compare the DTR/RTS
  boxes on the DMM settings page with the device table in
  [Supported devices](supported-devices.md); with manual settings, try DTR on
  and RTS off first.
- With manual settings, double-check baud rate, data bits and parity. Most
  meters in the device table use 7 data bits with odd or no parity.

## USB-HID cables

Meters with a USB-HID cable (many Uni-Trend models) appear in the port list as
`HID 0x1a86:0xe008 /dev/hidrawN`. If the entry is missing or cannot be opened,
the `/dev/hidraw*` device is usually only readable by root. A udev rule grants
access to your user; check the cable's vendor:product id with `lsusb` and match
it in the rule. QtDMM sets the cable's speed from the device table when it
opens the port.

## Windows

Serial ports appear as `COM3`, `COM4` … and need no permissions. USB-serial
cables need the chip vendor's driver once (CH340/CH341, FTDI, Prolific); until
it is installed the port is missing from the list. HID cables use the Windows
HID class driver and work without any driver installation. Debug output
(`qtdmm --debug`) is shown when QtDMM is started from a command prompt.

## "Another instance is running"

QtDMM instances coordinate through shared memory so that several meters can be
recorded side by side (see [Command line](command-line.md)). If an instance
crashed or was killed, its entry could be left behind; QtDMM now checks whether
that process still exists and replaces a stale entry automatically. Should the
message appear anyway, no other QtDMM is running, and it persists across
restarts, remove the shared memory segment with `ipcs -m` / `ipcrm` or simply
reboot.

## Readings look right on the meter but wrong in QtDMM

Check the model chosen on the DMM page — several meters share a protocol and
differ only in range or resolution, and a near match can decode with a wrong
factor. If the model is right and a specific range or function still decodes
wrongly, please report it with the meter model and, if possible, the debug
output (`qtdmm --debug` prints every frame as hex) on the project page.
