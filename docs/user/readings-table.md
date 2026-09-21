# Readings table

Next to the display, the analog meter and the recorder graph, QtDMM can
list every reading the meter sends: one row per value, with the time it
arrived, the value as the meter displayed it, unit, mode and range. Where
the recorder samples the reading on a fixed grid (once a second by
default) and keeps only the number, the table is the raw protocol of the
session - each of the meter's two to four readings per second, including
mode changes, `OL` and HOLD.

Switch it on with **Readings table** in the toolbar or the menu (Ctrl+4).
It opens as a panel docked below the graph; like the other panels it can
be dragged to another side or pulled out as a separate window, and QtDMM
remembers where you left it.

## The columns

| Column | Content |
|---|---|
| Time | when the reading arrived, to the millisecond |
| Value | the value as shown on the meter, `OL` for overload |
| Unit | with its SI prefix, as on the display (`mV`, `kOhm`) |
| Mode | `DC`, `AC`, `AC+DC`, `Resistance`, `Diode`, `Continuity`, `Capacitance`, `Frequency`, `Temperature` |
| Range | `AUTO` or `MANU`, when the meter reports it |
| Hold | `HOLD` while the meter's hold function is on |

Meters that send a second value with each reading (a frequency next to the
voltage, say) get an extra row for it, marked `2nd` in the Mode column.

## Following, keeping, clearing

The table **follows** the newest reading: it scrolls to the bottom as rows
arrive. Scroll up to look at older rows and following pauses; scroll back
to the end, or tick **Follow**, to resume.

The table keeps the newest **N rows** (10 000 by default, set it next to
**Keep**) and drops the oldest beyond that - at four readings a second
that is about forty minutes. **Clear** empties it; the recorder's graph is
not affected, and clearing the graph (Ctrl+Del) leaves the table alone.

Below the table a line sums up what it holds: the number of rows and the
minimum, maximum, mean and span of the numeric main readings (overloads
and second values are left out), in the unit of the newest row.

## Getting the data out

- **Copy** (Ctrl+C while the table has the focus, or the context menu) puts
  the selected rows - all rows when nothing is selected - on the clipboard
  as tab-separated text with a header line, ready to paste into a
  spreadsheet. Ctrl+A selects everything.
- **Export...** writes the whole table as CSV: `timestamp;value;unit;mode;range;hold`
  with ISO 8601 timestamps (`2026-09-21T14:03:05,250`), the value and unit
  exactly as displayed. The recorder's own export
  ([The recorder](recorder.md)) remains the one to use for the time-gridded
  series the graph shows.
