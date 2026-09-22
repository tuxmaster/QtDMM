# The recorder

The recorder graph — shown with the **Graph** toolbar button, hidden by
default and brought up automatically when a recording starts — records one
reading per sampling interval. What it
records is the meter's primary value in base units — a reading of 12.3 mV is
stored as 0.0123 V — so the curve stays continuous when the meter changes
range.

## Sampling

**Settings → Recording** (Ctrl+F2) sets:

- **Sample every** — the interval, in tenths of a second, seconds, minutes,
  hours or days. With an interval longer than the meter's own rate you get the average
  of the readings in that interval.
- **Sample time** — how long to record; the recorder stops by itself when it
  is reached. Leave it at zero to record until stopped.

**Settings → Scales** sets the visible window and the vertical scale
(automatic, or a fixed minimum and maximum).

## Starting and stopping

Three start modes, chosen on the Recording page:

- **Manual** — *Start* (Ctrl+S) and *Stop* (Ctrl+X) in the toolbar, or the
  graph's right-click menu.
- **Predefined time** — recording begins at the given time of day.
- **Trigger** — recording begins when the reading crosses a threshold, on its
  raising or falling edge. The threshold is drawn into the graph as a line you
  can drag. A **pre-trigger** time keeps the readings from just before the
  crossing.

*Clear* (Ctrl+Del) empties the recording. QtDMM warns before you lose unsaved data
by clearing, importing or quitting; the warning can be switched off under
**Settings → Appearance**.

[Alarms](alarms.md) can start and stop the recorder as well, on any of
their conditions.

## External command

**Settings → External application** runs a program when the reading crosses a threshold
(raising or falling edge), for example to switch something off. Optionally
QtDMM disconnects from the meter first so the command can use the serial
port.

## Looking at the data

- **Mouse wheel** zooms the time axis; the **middle button** drags it. On the
  keyboard: Ctrl++ / Ctrl+- zoom, Ctrl+0 shows the whole recording, after a
  click into the graph also `+`, `-`, `0`, the arrow keys, Home and End (see
  [Keyboard and mouse](keyboard.md)).
- **Copy image** (right-click menu or Ctrl+Shift+C) puts a picture of the graph
  on the clipboard.
- Hovering shows a crosshair with time and value at the cursor.
- **Integration** (Settings → Integration curve) draws a second curve: the running
  sum of the readings above a threshold, scaled and offset as configured — for
  charge or energy over time.
- **Print** (Ctrl+P) prints the graph with a title and comment.

## CSV export and import

*Export* (Ctrl+E) writes a semicolon-separated file:

```
timestamp;time (s);value;unit
2026-09-19T10:00:00,000;0;12.3;mV
2026-09-19T10:00:01,000;1;12.4;mV
```

The value is written with an SI prefix and the matching unit, exactly as a
meter would show it. *Import* (Ctrl+I) reads such files back, scaling the
values into base units again, and also accepts the tab-separated format of
QtDMM versions before 0.9.5. The unit of the first row becomes the graph's
unit.
