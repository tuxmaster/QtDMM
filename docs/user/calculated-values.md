# Calculated values

One QtDMM window shows one quantity. To see a quantity nobody measures
directly - the power a load draws, the resistance from a voltage and a
current, the difference between two channels - let one instance measure the
voltage, another the current, and a third *calculate* from both. The third
instance is an ordinary QtDMM window: display, analog meter, recorder,
export and printing all work on the calculated value exactly as on a
measured one.

## Setting it up

1. Start one instance per meter and give them short names: **Instances**
   (Ctrl+N), *Add*, e.g. `u` for the voltmeter and `i` for the ammeter.
   Names are used as variable names in formulas, so they may contain
   letters, digits and underscores only.
2. Connect each of them to its meter as usual.
3. Add a third instance, e.g. `p`. The quick way: the **ƒ** button in
   *Instances* asks for the name, the unit and the formula, writes the
   configuration and starts the instance already connected. The long way:
   add a plain instance, open its settings and on the *Multimeter* page
   choose the vendor **QtDMM** and the model **Calculated value**. Instead
   of the port and protocol settings a *Formula* group appears:
    - **Unit** - the unit of the result without SI prefix: `W`, `V`, `A`,
      `Ohm`, ... QtDMM adds `m`, `k` and so on itself.
    - **Formula** - e.g. `u * i`. Below it QtDMM lists the variables with
      the values the instances currently deliver, and which instances are
      running; a formula that does not parse is shown in red with the
      position of the error.
4. *OK*, then *Connect*. The status line reads *Calculating u * i* and the
   display shows the power.

Values are combined in their base units (volts, amperes, ohms), whatever
range the meters happen to be in, and the result is shown with a fitting
prefix: 12 V times 0.5 mA is `6.00 mW`.

## Formulas

- Operators `+ - * / ^`, parentheses. `^` binds tightest, so `-2^2` is `-4`
  and `u^2/r` is what you expect.
- Numbers may have an exponent or an SI suffix: `1e-3`, `1.5k`, `22u`, `4.7n`;
  `pi` is a constant.
- Functions: `sqrt(x)`, `abs(x)`, `log10(x)`, `sin(x)`, `cos(x)`, `exp(x)`,
  `floor(x)`, `min(a, b)`, `max(a, b)` and `rand()` (a new value in [0, 1)
  every time).
- Variables are the names of running instances. A name with a hyphen, such
  as `uni-t_803`, is written with an underscore in the formula
  (`uni_t_803`). A calculated instance can itself be an input for another
  one. `t` is reserved: the seconds since the instance connected.

Examples:

| What | Unit | Formula |
|---|---|---|
| Power | `W` | `u * i` |
| Resistance of a load | `Ohm` | `u / i` |
| Voltage drop across a shunt | `V` | `u_in - u_out` |
| Apparent power from RMS readings | `VA` | `u * i` |
| Magnitude of two components | `V` | `sqrt(a^2 + b^2)` |
| A reading scaled by a probe factor | `V` | `probe * 10` |

## The virtual meter

For trying QtDMM without a multimeter, for demonstrations or for feeding a
known signal into a calculation there is a second built-in model,
**QtDMM / Virtual meter**. It is the calculated instance with the formula
built for you from a few fields:

- **Waveform** — *Constant* (the Max value), *Random* (a new value between
  Min and Max each time), *Sine*, *Triangle*, *Square*, *Sawtooth* (between
  Min and Max, once per Period), *Discharge* (from Max towards Min with the
  time constant Period, like a battery under load) or *Custom formula*.
- **Unit** and **Coupling** (DC or AC) — what the display and the graph
  label the value with.
- **Min**, **Max**, **Period (s)**, **Noise** — noise is the peak-to-peak
  amplitude of uniform noise added on top, in the unit above.
- **Formula** — what the fields expand to; with *Custom formula* you edit
  it directly and may use everything from the section above, including the
  readings of other instances.

The virtual meter needs no other instance and publishes its value like a
real one, so `u * i` works just as well with a virtual `u`.

## When an input is missing

The calculated instance polls the others four times a second. While a
variable has no running instance, that instance has not delivered a value
for three seconds, or it shows *OL*, the calculated display shows **OL** and
the status line says which input is the problem, for example *Waiting for
instance 'i'*. The recorder keeps running; the gap is recorded like an
overload from a real meter.

Readings arrive at the pace of the slowest meter and the poll interval, so
a calculated value can lag its inputs by up to a quarter of a second plus
one meter update. For synchronised recordings start all instances together
with *Start* (see [The recorder](recorder.md)); every CSV export carries
timestamps.
