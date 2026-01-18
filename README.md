# PickPlaz STM32 to ESP32 Port (Repo Root)

This root contains the original STM32 firmware alongside the ESP32-C3 port
workspace and local QEMU assets used for emulation-based porting tests.

## Structure (top level)

- `application/`, `Core/`, `Drivers/`, `include/`, `src/` - STM32 firmware
  sources and Cube-style project layout.
- `ESP32Port/` - ESP32-C3 port workspace (PlatformIO, docs, ported firmware).
- `qemu/` - QEMU source tree used to emulate ESP32-C3 during porting tests.
- `qemu-install/` - Local QEMU install prefix (binaries/libs for the emulator).

## Pointers

- ESP32 port overview: `ESP32Port/pickplazESP32Port/README.md`
- QEMU usage details: `ESP32Port/pickplazESP32Port/docs/source/tooling/qemu_setup.md`
