# Workflows (Make Targets)

This doc groups the Make targets into common, independent workflows so we
don’t have to remember long command lines.

## Conventions
- Run all commands from `ESP32Port/pickplazESP32Port/`.
- Override variables at invocation time, e.g. `PORT=/dev/ttyACM0 make monitor-usb`.
- QEMU workflows assume the Espressif QEMU fork is installed at repo root.

## Coupling and terminals
Some workflows are coupled (build → merge → run) and/or require two terminals.

Key rules:
- QEMU serial monitor **must** attach after QEMU starts, otherwise the socket
  is not yet listening and the monitor exits.
- If you want a single terminal, use `make qemu-run-monitor` (it waits for
  the socket before attaching).
- For QEMU, you always need a **build** and **merge** before running QEMU
  unless you already have a fresh `flash_image.bin`.

## Workflow A: Hardware build (devkit)
Build firmware for the ESP32‑C3 devkit without flashing.
```bash
make build-devkit
```

## Workflow B: Hardware flash + monitor
Flash the board and attach the monitor.
```bash
make flash
make monitor-usb
```

If you need a specific port:
```bash
PORT=/dev/ttyACM0 make flash-port
PORT=/dev/ttyACM0 make monitor-usb
```

## Workflow C: QEMU smoke test (QEMU install only)
Verify the QEMU binaries exist and can list machines.
```bash
make qemu-smoke
```

## Workflow D: QEMU run with monitor (recommended)
Build, merge, then run QEMU with the monitor attached in one command.
This avoids the socket timing race.
```bash
make build-qemu
make merge-qemu
make qemu-run-monitor
```

Or use the composite target:
```bash
make qemu-all
```

To run without the self-test:
```bash
make qemu-all-noselftest
```

## Workflow E: QEMU run (two terminals)
Start QEMU in one terminal, then attach a monitor in another.
Order matters: start QEMU first.
```bash
# terminal 1
make qemu-run

# terminal 2
make monitor-qemu
```

## Workflow F: QEMU run (stdio only)
Print the serial output directly in the QEMU terminal.
```bash
make build-qemu
make merge-qemu
make qemu-run-stdio
```

## Workflow G: QEMU reset
Print the reset sequence and execute it manually in a telnet session.
```bash
make qemu-reset
```
Then run:
```text
system_reset
```

## Workflow H: Clean builds
Clean build artifacts for a specific environment.
```bash
make clean-devkit
make clean-qemu
```

## Workflow I: Fast QEMU rerun
Skip the build step when binaries are unchanged.
```bash
make qemu-run-fast
```

For the no-selftest environment:
```bash
make qemu-run-fast-noselftest
```

## Workflow J: Flash + monitor (hardware)
Convenience wrapper to flash then attach the USB monitor.
```bash
make devkit-flash-monitor
```

## Workflow K: PICSimLab integration
Use PICSimLab as a GUI to observe the QEMU run.

External QEMU (recommended if you want our custom QEMU build):
```bash
make picsimlab-external
```

No-selftest variant:
```bash
make picsimlab-external-noselftest
```

Or run the steps manually:
```bash
make build-qemu
make merge-qemu
make qemu-picsimlab
make picsimlab
```

Standalone PICSimLab (uses its built-in QEMU):
```bash
make picsimlab-standalone
```

No-selftest variant:
```bash
make picsimlab-standalone-noselftest
```

Or run the steps manually:
```bash
make build-qemu
make merge-qemu
make picsimlab
```

See `docs/PICSIMLAB_INTEGRATION.md` for details.

## Variable overrides
Defaults:
- `PORT=/dev/ttyACM0`
- `BAUD=115200`
- `QEMU_SERIAL_PORT=4444`
- `QEMU_MONITOR_PORT=5555`

Examples:
```bash
PORT=/dev/ttyACM1 make monitor-usb
QEMU_SERIAL_PORT=4445 QEMU_MONITOR_PORT=5556 make qemu-run-monitor
```
