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

Longer-term idea from the original README: split measuring and recording into
a separate background daemon.
