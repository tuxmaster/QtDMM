# QtDMM

QtDMM reads digital multimeters over serial, USB-HID, RFC2217 and sigrok, shows
the live reading and records it in a transient recorder with CSV export, thresholds
and an integration curve.

This documentation is organised in three parts:

- **[User Guide](user/index.md)** — installing, connecting a meter, recording and
  exporting.
- **[Protocols](protocols/index.md)** — the wire protocols of the supported meters:
  which decoder handles which device, the original protocol notes and captures,
  and the test vectors derived from them.
- **[Development](dev/index.md)** — building, testing, and extending QtDMM,
  including how to add support for a new meter.

Build this site locally with `pip install mkdocs` and `mkdocs serve` from the
repository root.
