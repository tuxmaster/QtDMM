# Development

Documentation for people working on QtDMM itself. Planned pages:

- **Building and testing** — `compile.sh`, the CMake targets, the CTest suites
  (`test_decoder`, `test_graph`) and how the decoder fixtures in
  `tests/data/decoder/` are generated from the protocol specifications.
- **Architecture** — the path from `PortHandler` and the `QIODevice` backends
  through `ReaderThread` and the `DmmDecoder` subclasses to `DMM`, `MainWid`,
  `DisplayWid` and `DMMGraph`; the `DmmResponse` value contract; the
  `SharedStateManager` used for multiple instances.
- **Adding a protocol** — every place a new meter protocol has to be registered:
  the `DmmDecoder` subclass, the `ReadEvent::DataFormat` enum, the
  `DmmDecoder::getInstance()` factory, `addConfig()` for each supported device,
  the protocol entry in `src/ui/uidmmprefs.ui` (whose order must currently match
  the enum), and the protocol specification and test vectors under
  `docs/protocols/spec/`.

None of these pages exist yet.

## Translations

The build compiles `assets/translations/qtdmm_*.ts` with lrelease only. After
adding or changing `tr()` strings, refresh the sources explicitly:

```
cmake --build build --target update_translations
```

then fill in the new entries (Qt Linguist or a text editor) and commit the
`.ts` files. lupdate is kept out of the normal build on purpose: it rewrites
files in the source tree, and two targets running it in parallel corrupted
them.

## Platforms

Linux, macOS and FreeBSD are built and tested by the GitHub workflows with
the distribution's Qt and hidapi (FreeBSD runs inside a VM on an Ubuntu runner
via `vmactions/freebsd-vm`, since GitHub has no FreeBSD runner). Windows is built on `windows-latest` with Qt from
`install-qt-action` and hidapi compiled from source via CMake `FetchContent`
(see `CMakeLists.txt`); the workflow uploads a portable ZIP and an Inno Setup
installer, both produced by CPack from the same install tree
(`cmake/deploy.cmake`). Platform-specific code is limited to the permission
hint in `src/dmm.cpp`, the process liveness check in
`src/sharedstatemanager.cpp`, the serial port naming in
`src/portdevices/serial.cpp` and the console attach in `src/main.cpp`.

Longer-term idea from the original README: split measuring and recording into
a separate background daemon.
