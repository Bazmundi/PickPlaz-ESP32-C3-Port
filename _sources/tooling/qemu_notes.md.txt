# QEMU Smoke Test (Stage 1)

This doc records a lightweight smoke test for the local QEMU setup.
It is **not** a functional ESP32 simulation yet; it only verifies that the
QEMU toolchain exists and can be invoked without errors.

## Why this exists
- Keep QEMU available as an optional tool without tracking its source/build.
- Provide a fast check before Stage 2+ adds more hardware‑dependent logic.
- Record the QEMU setup story used in the earlier esp32version work.

## What is (and is not) tracked
- `qemu/` and `qemu-install/` live at repo root and are **ignored** by git.
- This repo tracks the smoke test script and the QEMU setup docs under `docs/`.

## Smoke test
Run from the repo root:

```bash
ESP32Port/pickplazESP32Port/scripts/qemu_smoke.sh
```

Expected output:
- A list of `qemu-system-*` binaries found under `qemu-install/usr/local/bin`.
- Version info for each binary.
- A confirmation line if `qemu-system-riscv32` can list machines.

## QEMU setup and usage
For the full setup story and ESP32-C3 run scripts, see:
- `ESP32Port/pickplazESP32Port/docs/QEMU_SETUP.md`
- `ESP32Port/pickplazESP32Port/docs/QEMU_SUMMARY.md`
- `ESP32Port/pickplazESP32Port/scripts/merge_flash.sh`
- `ESP32Port/pickplazESP32Port/scripts/run_qemu.sh`
- `ESP32Port/pickplazESP32Port/scripts/run_qemu_with_monitor.sh`
- `ESP32Port/pickplazESP32Port/scripts/run_flash_qemu.sh`

The QEMU build uses the `esp32-c3-qemu` environment with `sdkconfig.qemu`
to force a UART console (QEMU does not emulate USB Serial/JTAG).
Flash size is pinned to 4MB in `platformio.ini` to match the device.

## Limitations
- This does **not** boot the ESP32-C3 firmware.
- The current QEMU install includes `qemu-system-riscv32`; there is no
  guarantee of an ESP32-compatible target in this tree.
- If we later adopt an ESP32‑capable QEMU fork, the smoke script can be
  extended to launch a real firmware run.

## Notes for future automation
If we decide to formalize QEMU testing:
- Pin the QEMU fork/version and target machine in this repo.
- Add a small harness to run the firmware and capture serial output.
- Keep any generated outputs in a dedicated, ignored folder.
