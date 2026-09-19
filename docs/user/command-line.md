# Command line

```
qtdmm [options]
```

| Option | Meaning |
|---|---|
| `--config-id <id>` | Use the named configuration instead of `default`. Each id has its own settings file, so one meter can be set up per id. |
| `--config-dir <dir>` | Directory for the configuration files (default: the platform's user config location). |
| `--debug` | Print every frame received from the meter as hex to the console. Useful when a meter is not decoded correctly — include this output in a bug report. |
| `-h`, `--help` | Show the options. |
| `-v`, `--version` | Show the version. |

## Several meters at once

Every QtDMM window is one *instance*, identified by its `--config-id`. Start
further instances from **Instances** (Ctrl+N): *Add* asks for a name and
launches a new QtDMM with that id; the list shows which instances are
configured and which are running, and lets you open or remove them. Running
instances know about each other through shared memory — the same mechanism
that stops two windows from using the `default` id at once.

## Debug output

`--debug` writes each received frame as a line of hex bytes, e.g.

```
30 30 30 30 30 31 3B 30 30 30 3A 30 0D 0A
```

These lines are exactly what the decoder test fixtures are made of, so a short
capture together with what the meter displayed at the time is the most useful
thing to attach when reporting a decoding problem.
