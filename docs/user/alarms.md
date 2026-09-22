# Alarms

An alarm watches the reading and tells you when it leaves the range you
expect - a battery that sags below 12 V, a supply that drifts high, a
sensor that stops answering. Each QtDMM instance has its own list of
alarms under **Settings → Alarms**.

## Conditions

| Condition | Raises when |
|---|---|
| Reading below | the reading is under the threshold |
| Reading above | the reading is over the threshold |
| Reading outside a range | it is under the lower or over the upper bound |
| Reading inside a range | it is between the bounds |
| Overload (OL) | the meter shows an overload |
| No readings for a while | nothing arrived for the given number of seconds. Counted from the moment the meter is connected, so it does not raise while QtDMM is not connected |

Thresholds are in the reading's base unit and take SI suffixes (`4.7m`,
`12k`). Two settings keep an alarm from chattering:

- **For at least** - the condition must hold this long before the alarm
  raises; a spike shorter than that is ignored.
- **Hysteresis** - once raised, the alarm clears only when the reading has
  come back by this much (below 12 V with 0.2 V hysteresis clears at
  12.2 V).

An unticked alarm in the list is kept but never fires.

## Actions

What happens when an alarm raises; tick any of them:

- **Banner** over the graph in the alarm's colour with its message (or the
  condition in words). **Acknowledge** hides it; the alarm stays raised
  until the reading is back and raises again only after that.
- **Beep**.
- **Popup window** with message, value and time.
- **Bring the QtDMM window to the front**.
- **Mark in the recorder graph** - a dashed vertical line in the alarm's
  colour at the moment it raised, kept with the recording; and **in the
  readings table** - the row gets the colour.
- **Recorder** - start or stop the recording.
- **Run program** - a command line; `%v` is the value, `%u` its unit,
  `%n` the alarm's name. That is the hook for anything else: a
  notification (`notify-send "%n" "%v%u"`), a mail, a webhook with `curl`,
  a relay.

The status line reports every raise and clear too.

Alarms judge the main reading only. For a condition on several meters -
power over a limit, say - set up a [calculated value](calculated-values.md)
and put the alarm on that instance.
