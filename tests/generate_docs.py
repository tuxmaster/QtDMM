#!/usr/bin/env python3
"""Generate the documents that must not drift from their source.

1. docs/user/supported-devices.md - the device table, read off the
   DmmDecoder::addConfig({...}) registrations in src/decoders/*.cpp. Those calls
   are the only thing that decides what the settings dialog offers, so the
   documentation is derived from them rather than maintained by hand.

2. README.md - the GitHub landing page, assembled from docs/ so the website,
   the in-app handbook and the README share one source. Relative links are
   rewritten to paths that work from the repository root.

Usage:
  generate_docs.py            write both files
  generate_docs.py --check    regenerate in memory and compare; exit 1 on drift
"""

import argparse
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DECODER_DIR = REPO / "src" / "decoders"
DOCS = REPO / "docs"
DEVICES_MD = DOCS / "user" / "supported-devices.md"
README = REPO / "README.md"

# {"Vendor", "Model", "", baud, ReadEvent::Proto, bits, stopBits, numValues,
#  parity, display, externalSetup, rts, dtr}
ADD_CONFIG = re.compile(
    r'addConfig\(\s*\{\s*"(?P<vendor>[^"]*)"\s*,\s*"(?P<model>[^"]*)"\s*,\s*"[^"]*"\s*,'
    r'\s*(?P<baud>\d+)\s*,\s*ReadEvent::(?P<protocol>\w+)\s*,\s*(?P<bits>\d)\s*,'
    r'\s*(?P<stop>\d)\s*,\s*(?P<values>\d+)\s*,\s*(?P<parity>\d)\s*,\s*(?P<counts>\d+)\s*,'
    r'\s*(?P<ext>\d)\s*,\s*(?P<rts>\d)\s*,\s*(?P<dtr>\d)')
PARITY = {"0": "N", "1": "E", "2": "O"}

# The chip behind a protocol, where it is known: lets a user match an
# unlisted meter to a protocol. Read from the protocol table in
# src/protocols.cpp - one row per line:
#   { ReadEvent::<Id>, "<Name>", QT_TRANSLATE_NOOP("Protocols", "<text>"), "<chip>", make<...> },
PROTOCOL_ROW = re.compile(
    r'\{\s*ReadEvent::(?P<id>\w+)\s*,\s*"(?P<name>\w+)"\s*,\s*QT_TRANSLATE_NOOP\("Protocols",\s*"(?P<text>[^"]*)"\)'
    r'\s*,\s*"(?P<chip>[^"]*)"\s*,\s*make<(?P<decoder>\w+)>')


def protocol_table():
    """{name: {"id", "text", "chip", "decoder"}} from src/protocols.cpp."""
    text = (REPO / "src" / "protocols.cpp").read_text(encoding="utf-8")
    rows = {}
    for m in PROTOCOL_ROW.finditer(text):
        rows[m.group("name")] = {"id": m.group("id"), "text": m.group("text"),
                                 "chip": m.group("chip"), "decoder": m.group("decoder")}
    if not rows:
        sys.exit("no protocol rows found in src/protocols.cpp - format changed?")
    return rows


CHIP = {name: row["chip"] for name, row in protocol_table().items() if row["chip"]}


def devices():
    rows = []
    for src in sorted(DECODER_DIR.glob("*.cpp")):
        for m in ADD_CONFIG.finditer(src.read_text(encoding="utf-8")):
            d = m.groupdict()
            lines = "DTR" if d["dtr"] == "1" else ""
            if d["rts"] == "1":
                lines = (lines + " RTS").strip()
            model = d["model"]
            unconfirmed = model.endswith("*")
            rows.append({
                "vendor": d["vendor"], "model": model.rstrip("* ").strip() + (" ¹" if unconfirmed else ""),
                "protocol": d["protocol"], "chip": CHIP.get(d["protocol"], "-"),
                # baud 0: not a serial device (Bluetooth LE, or sigrok-cli talks to the meter)
                "serial": f'{d["baud"]} {d["bits"]}{PARITY[d["parity"]]}{d["stop"]}' if d["baud"] != "0"
                          else ("sigrok-cli" if d["protocol"] == "Sigrok" else "Bluetooth LE"),
                "counts": d["counts"], "lines": lines or "-",
                "decoder": src.name,
            })
    rows.sort(key=lambda r: (r["vendor"].lower(), r["model"].lower()))
    return rows


def render_devices(rows):
    out = [
        "# Supported devices",
        "",
        "Every meter QtDMM can decode, taken from the decoder registrations in",
        "`src/decoders/` (this page is generated from them by",
        "`tests/generate_docs.py`). Choosing one of these models on the Multimeter",
        "settings page fills in the serial parameters below; meters not listed can",
        "often be used with *Manual settings* if they speak one of the listed",
        "protocols - the *Chip* column helps: a meter built around the same chip",
        "(named in its manual or on the sigrok wiki) usually speaks the same",
        "protocol. See [Connecting a meter](connecting.md).",
        "",
        "*Serial* is baud rate, data bits, parity (N/E/O) and stop bits. *Lines* are",
        "the control lines the cable needs driven. *Counts* is the display",
        "resolution. Not every entry has been confirmed on hardware recently; models",
        "marked ¹ were added from chip datasheets and protocol documentation and have not",
        "been tried with QtDMM at all. If you can confirm one, or get an unlisted",
        "meter working, please report it on the",
        "[project page](https://github.com/tuxmaster/QtDMM/issues).",
        "",
        "| Vendor | Model | Chip | Protocol | Serial | Lines | Counts |",
        "|---|---|---|---|---|---|---|",
    ]
    for r in rows:
        out.append(f'| {r["vendor"]} | {r["model"]} | {r["chip"]} | `{r["protocol"]}` | {r["serial"]} '
                   f'| {r["lines"]} | {r["counts"]} |')
    out.append("")
    out.append("¹ settings taken from the chip, not yet confirmed on hardware with QtDMM.")
    out.append("")
    out.append(f"{len(rows)} devices across {len({r['vendor'] for r in rows})} vendors.")
    return "\n".join(out) + "\n"


LINK = re.compile(r"\]\((?!https?://|mailto:|#|/)([^)]+)\)")


def rebase_links(text, page_dir):
    """Make links in a page from docs/<page_dir> resolve from the repo root."""
    prefix = "docs/" + (page_dir + "/" if page_dir else "")
    return LINK.sub(lambda m: f"]({prefix}{m.group(1)})", text)


def demote(text):
    """Shift every heading one level down so a page nests under a README H2."""
    return re.sub(r"^(#+) ", lambda m: "#" + m.group(1) + " ", text, flags=re.M)


def render_readme(devices_md):
    index = (DOCS / "index.md").read_text(encoding="utf-8")
    cmdline = (DOCS / "user" / "command-line.md").read_text(encoding="utf-8")

    head, licensing = index.split("\n## Licensing\n", 1)
    devices_body = devices_md.split("\n", 1)[1]        # drop the H1
    cmdline_body = cmdline.split("\n", 1)[1]

    parts = [
        "<!-- GENERATED from docs/ by tests/generate_docs.py - edit the pages in docs/, not this file -->",
        "",
        rebase_links(head.rstrip("\n"), ""),
        "",
        "## Supported hardware",
        rebase_links(devices_body.rstrip("\n"), "user"),
        "",
        "## Command line",
        demote(rebase_links(cmdline_body.rstrip("\n"), "user")),
        "",
        "## Licensing",
        licensing.strip("\n"),
        "",
    ]
    return "\n".join(parts)


def protocol_drift():
    """Every ReadEvent::DataFormat value before EndOfList must have exactly
    one row in src/protocols.cpp, with the enum id as its name."""
    header = (REPO / "src" / "readevent.h").read_text(encoding="utf-8")
    body = header[header.index("enum DataFormat"):header.index("EndOfList")]
    enum_ids = [m.group(1) for m in re.finditer(r"^\s*(\w+)\s*(?:=\s*-?\d+)?\s*,", body, re.M) if m.group(1) != "Invalid"]
    rows = protocol_table()
    problems = []
    for e in enum_ids:
        if e not in rows:
            problems.append(f"ReadEvent::{e} has no row in src/protocols.cpp")
    for name, row in rows.items():
        if row["id"] != name:
            problems.append(f"protocols.cpp row {name}: enum id {row['id']} differs from the name")
        if row["id"] not in enum_ids:
            problems.append(f"protocols.cpp row {name}: ReadEvent::{row['id']} is not in the enum")
    return problems


def hid_cable_drift():
    """The HID cable table exists three times: tests/data/hid_cables.json (the
    truth both test suites read), kCables in src/portdevices/hidserial.cpp and
    HID_CABLES in tools/qtdmm-bridge/qtdmm_bridge.py. Returns the problems."""
    import json

    truth = {(c["vid"].lower(), c["pid"].lower()): c["chip"]
             for c in json.loads((REPO / "tests" / "data" / "hid_cables.json").read_text(encoding="utf-8"))["cables"]}
    cpp = {}
    for m in re.finditer(r"\{\s*0x([0-9a-fA-F]{4}),\s*0x([0-9a-fA-F]{4}),\s*HIDSerialDevice::Chip::(\w+)\s*\}",
                         (REPO / "src" / "portdevices" / "hidserial.cpp").read_text(encoding="utf-8")):
        cpp[(m.group(1).lower(), m.group(2).lower())] = m.group(3)
    py = {}
    for m in re.finditer(r"\(0x([0-9a-fA-F]{4}),\s*0x([0-9a-fA-F]{4})\):\s*\(\"(\w+)\"",
                         (REPO / "tools" / "qtdmm-bridge" / "qtdmm_bridge.py").read_text(encoding="utf-8")):
        py[(m.group(1).lower(), m.group(2).lower())] = m.group(3)
    problems = []
    for name, table in (("hidserial.cpp kCables", cpp), ("qtdmm_bridge.py HID_CABLES", py)):
        if table != truth:
            problems.append(f"{name} differs from tests/data/hid_cables.json: "
                            f"{sorted(set(table.items()) ^ set(truth.items()))}")
    return problems


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--check", action="store_true")
    args = ap.parse_args()

    devices_md = render_devices(devices())
    outputs = {DEVICES_MD: devices_md}
    outputs[README] = render_readme(devices_md)

    stale = 0
    for path, text in outputs.items():
        rel = path.relative_to(REPO)
        if args.check:
            current = path.read_text(encoding="utf-8") if path.exists() else None
            ok = current == text
            stale += not ok
            print(f'{"ok" if ok else "STALE":8s} {rel}')
        else:
            path.write_text(text, encoding="utf-8")
            print(f"wrote    {rel}")

    for problem in protocol_drift() + hid_cable_drift():
        print(f"DRIFT    {problem}", file=sys.stderr)
        stale += 1

    if stale:
        print(f"\n{stale} file(s) differ from their sources - run tests/generate_docs.py",
              file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
