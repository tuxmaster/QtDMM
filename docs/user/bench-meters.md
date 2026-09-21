# Bench meters through sigrok-cli

Keysight, Agilent, HP and Siglent bench multimeters speak SCPI over USB or
LAN. QtDMM does not implement SCPI itself; it lets
[sigrok-cli](https://sigrok.org/wiki/Sigrok-cli) talk to the meter and reads
the values sigrok-cli prints - the same path as the `Sigrok` entries under
[Special ports](connecting.md#port-types), just with the model in the list
and the settings in one group.

## Which meters

The models libsigrok's `scpi-dmm` driver recognises by their `*IDN?` reply:
Keysight 34465A, Agilent 34405A / 34410A / 34460A, HP 34401A, Siglent
SDM3055 - and **SCPI DMM (any scpi-dmm model)** for the others the driver
knows (OWON XDM1041/2041, GW Instek GDM8251A/8255A). None of them has been
tried with QtDMM yet; sigrok's own [device pages](https://sigrok.org/wiki/Supported_hardware#Multimeters)
say how well each one works. Other libsigrok multimeter drivers can be
entered in the **Driver** field.

## Setting it up

1. Install sigrok-cli (Debian/Ubuntu: `apt install sigrok-cli`; Windows and
   macOS builds are on the sigrok download page). If it is not on your
   `PATH`, set its location under **Settings → Special ports**.
2. **Settings → Multimeter**: choose the vendor and the model with
   *(sigrok)*. The port and serial settings make way for a **sigrok-cli**
   group:
    - **Driver** - the libsigrok driver, filled in from the model.
    - **Connection** - how sigrok-cli reaches the meter, its `conn=`
      option: a serial port (`/dev/ttyUSB0`, `COM3`; the list offers the
      ones found), `usbtmc/<vid>.<pid>` for USB-TMC (the meter's USB ids
      in hex, e.g. `usbtmc/2a8d.0101`), or `tcp-raw/<host>/5025` for LAN.
    - **Options** - further driver options, colon separated, such as
      `serialcomm=9600/8n1` for a serial connection.
    - The line underneath says whether sigrok-cli was found and whether it
      has the driver; **Test** runs `sigrok-cli --scan` with these settings
      and shows the meter's answer. A serial port without a SCPI meter keeps
      sigrok-cli waiting, so the test gives up after 30 s.
3. **Connect**. sigrok-cli runs in the background for as long as the meter
   is connected and QtDMM shows its readings like any other meter's.

On Linux, USB-TMC devices need read/write permission for your user, typically
a udev rule such as
`SUBSYSTEM=="usbmisc", KERNEL=="usbtmc*", MODE="0666"` (or the rules file
that comes with libsigrok, `60-libsigrok.rules`).

## Notes

- The meter keeps the function and range set on its front panel; QtDMM
  shows whatever it measures (`VOLT:DC`, `RES`, ...). sigrok-cli switches
  the meter to remote control while it runs.
- Reading rate is what sigrok-cli delivers, usually a few readings per
  second.
- The status line reports *Lost connection* when sigrok-cli exits (meter
  switched off, cable pulled) and QtDMM tries again every few seconds.
