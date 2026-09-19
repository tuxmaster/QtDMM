# Analog meter

Besides the digital display, QtDMM can show the reading on a moving-coil style
instrument: a dial with a scale on an arc, a needle that swings with the
inertia of a real meter, a red zone at the top end, boxes with the current
value and the maximum, and an overload lamp.

Switch it on with **Analog meter** in the display toolbar or in the menu. It
opens as a panel docked to the right of the graph; drag its title bar to dock
it on another side or to pull it out as a separate window, and resize it
freely — the dial, scale and lettering scale with the window.

## The scale

The scale is drawn in the unit the multimeter shows, prefix included, and its
full scale follows the meter's range: a 4000-count meter reading `3.856 V`
gets a 0 … 4 V scale, `385.6 mV` a 0 … 400 mV scale. When the meter changes
range the scale relabels itself. The inner 0 … 100 arc is the same reading as
a percentage of full scale, like the bar graph on the meter itself.

Negative readings push the needle into the short stub left of zero and show
their sign in the **CURRENT** box. On the **GUI** settings page you can choose
how the scale is laid out:

- **Automatic** — zero at the left; as soon as a clearly negative reading
  arrives the scale switches to centre zero (−FS … 0 … +FS) and stays there
  until you reset the min/max memory (Ctrl+R).
- **Zero left** and **Centre zero** fix one layout.

The **red zone** starts at 90 % of full scale by default; the same settings
page lets you move it.

## Readouts and lamp

**CURRENT** shows the reading exactly as the multimeter sends it, **MAX** the
maximum since the last reset in the same unit. The **OL** lamp lights and the
needle rests against the right stop while the meter reports overload; **HOLD**
appears while the meter's hold function is active.

## Style

Two colour schemes are available on the GUI settings page: a dark studio dial
with a white scale and needle, and a classic ivory dial with black lettering.
**Needle inertia** can be switched off to make the needle jump straight to each
new reading.
