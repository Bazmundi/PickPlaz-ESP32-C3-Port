# PickPlaz ESP32-C3 Port

PickPlaz began life on STM32. This repo tells the story of moving that firmware
to the ESP32-C3 while preserving the original behaviors and timing quirks. The
goal is a faithful port that is easier to build, test, and evolve.

If you are new here, start with the docs index and stage overview. They give
context, decisions, and the rationale behind each step.

## Where the ESP32 port lives

At the repo root, the ESP32-C3 port lives under
`ESP32Port/pickplazESP32Port/` (this directory). The STM32 baseline and other
historical assets remain at the repo root.

## Start here

- Docs entry point: [docs/source/index.md](docs/source/index.md)
- Porting context + constraints: [docs/source/design/porting_design.md](docs/source/design/porting_design.md)
- Stage plan and progress: [docs/source/stages/overview.md](docs/source/stages/overview.md)

## Build, run, and tooling

These are intentionally light here; the details live in the docs.

- Environment and dependencies: [PROJECT_DEPENDENCIES.md](PROJECT_DEPENDENCIES.md)
- PlatformIO environments: [platformio.ini](platformio.ini)
- QEMU setup and usage: [docs/source/tooling/qemu_setup.md](docs/source/tooling/qemu_setup.md)
- PICSimLab notes: [docs/source/tooling/picsimlab.md](docs/source/tooling/picsimlab.md)

## Documentation

Docs are authored in [docs/source/](docs/source/) and built with Sphinx + Doxygen. See:

- Docs workflow and outputs: [docs/source/index.md](docs/source/index.md)
- Build commands: [docs/Makefile](docs/Makefile)

## Port layout (this directory)

- `src/`, `include/`, `lib/` - firmware sources
- [docs/source/](docs/source/) - documentation sources
- `scripts/` - helper scripts (QEMU, flash merge, etc.)
- [platformio.ini](platformio.ini) - PlatformIO environments

## License

See [LICENSE](LICENSE) and [NOTICE.md](NOTICE.md).
