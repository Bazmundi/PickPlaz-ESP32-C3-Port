# Project Dependencies

This document captures the build and documentation dependencies we have
observed in this repo and nearby tooling. It is meant to help a remote
developer reproduce the environment for devkit, QEMU, and docs workflows.

## Environment overview

- Project Python version: 3.12 (see `pyproject.toml`)
- PlatformIO is installed in the project venv (`.venv/bin/pio`)
- ESP-IDF toolchain comes from PlatformIO's `espressif32` platform
- QEMU binaries are expected at the repo root: `qemu-install/usr/local/bin`
- Docs use Doxygen + Sphinx (Breathe/Exhale)

## Python environment (.venv)

This venv appears to be created by `uv` and does not include `pip` by default.
Use `uv pip` to install packages or use `python -m pip` after installing pip.

Installed packages observed in `.venv`:

- ajsonrpc==1.2.0
- anyio==4.12.1
- bottle==0.13.4
- certifi==2026.1.4
- charset-normalizer==3.4.4
- click==8.1.7
- colorama==0.4.6
- h11==0.16.0
- idna==3.11
- marshmallow==3.26.2
- packaging==25.0
- platformio==6.1.18
- pyelftools==0.32
- pyserial==3.5
- requests==2.32.5
- semantic-version==2.10.0
- starlette==0.46.2
- tabulate==0.9.0
- typing_extensions==4.15.0
- urllib3==2.6.3
- uvicorn==0.34.3
- wsproto==1.3.2

Python dev extras declared in `pyproject.toml` (not installed by default):

- black
- pyright
- pytest
- ruff

Docs-related Python packages are not declared in `pyproject.toml`.
They should be installed in `docs/.venv` using `docs/requirements.txt`.
On this machine they are also present in user Python, which is why the docs
build works without a docs venv.

- sphinx
- myst-parser
- breathe
- exhale
- sphinx-rtd-theme

## ESP32-C3 devkit build (PlatformIO + ESP-IDF)

Required:

- PlatformIO Core (installed in `.venv`)
- PlatformIO `espressif32` platform (auto-installed by `pio run`)
- ESP-IDF toolchain + `riscv32-esp-elf` (PlatformIO packages)
- CMake >= 3.16
- `make`, `git`, `bash`
- USB serial access (Linux: add user to `dialout` group if needed)

Typical commands (see `platformio.ini` for details of -e for env):

- `pio run -e esp32-c3-devkitm-1`
- `pio run -e esp32-c3-devkitm-1 -t upload`

## ESP32-C3 QEMU build and run

Build side (PlatformIO):

- Same as devkit build, but uses `sdkconfig.qemu`
- Merge script depends on PlatformIO's Python and `esptool.py`

QEMU runtime:

- `qemu-system-riscv32` installed at repo root:
  `qemu-install/usr/local/bin/qemu-system-riscv32`
- `telnet` (monitor and serial)
- Optional: `riscv32-esp-elf-gdb` for debugging
- Optional: `picsimlab` GUI integration

QEMU build deps (for Espressif QEMU fork on Ubuntu/Debian):

- `libslirp-dev`
- `libglib2.0-dev`
- `libpixman-1-dev`
- `gdb-multiarch`
- `telnet`
- `make`, `gcc`, `pkg-config`, `git`

Typical commands (see `platformio.ini` for details of -e for env):

- `pio run -e esp32-c3-qemu`
- `scripts/merge_flash.sh`
- `scripts/run_qemu_with_monitor.sh`

## Documentation build (Doxygen + Sphinx)

Required system packages:

- `doxygen`
- `graphviz` (for `dot`)
- `make`

Required Python packages (docs venv; see `docs/requirements.txt`):

- sphinx
- myst-parser
- breathe
- exhale
- sphinx-rtd-theme

The `docs/Makefile` prefers `docs/.venv/bin/sphinx-build` when present and
falls back to whatever `sphinx-build` is on PATH.

Typical commands (from `docs/`):

- `make docs`
- `make docs-internal`

Internal API build (manual fallback, equivalent to `make docs-internal`):

- `doxygen Doxyfile.internal`
- `BREATHE_XML_DIR="$PWD/doxygen-internal/xml" make html BUILDDIR=build-internal`

## Docs publishing (gh-pages branch)

Local publish helper:

- `scripts/publish_gh_pages.sh`

This script expects:

- `doxygen` and `dot` on PATH (system installs are fine)
- Docs venv at `docs/.venv` with Sphinx dependencies installed

It builds docs and writes `docs/build/html` into a temporary `gh-pages` worktree,
then creates a commit. Push with `git push origin gh-pages`.

## Remote developer setup (suggested baseline)

1) Install system packages (Ubuntu/Debian example):
   - `git`, `make`, `cmake`, `python3.12`, `python3.12-venv`
   - `doxygen`, `graphviz`
   - QEMU build deps if needed: `libslirp-dev`, `libglib2.0-dev`,
     `libpixman-1-dev`, `gdb-multiarch`, `telnet`
2) Install `uv` and create the venv:
   - `uv venv`
   - `source .venv/bin/activate`
3) Install PlatformIO into the venv:
   - `uv pip install platformio`
4) (Optional) Install dev extras (only for py dev):
   - `uv pip install -e ".[dev]"`
5) (Optional) Create a docs venv in `docs/` and install requirements:
   - `cd docs`
   - `uv venv`
   - `source .venv/bin/activate`
   - `uv pip install -r requirements.txt`
6) (Optional) Build Espressif QEMU fork into `qemu-install/` at repo root
7) Verify the expected build commands in `Makefile` and `platformio.ini`

## Known local paths and assumptions

- PlatformIO packages live under `~/.platformio/`
- QEMU binaries are expected at `../qemu-install/usr/local/bin/` relative
  to this project directory (`ESP32Port/pickplazESP32Port`)
