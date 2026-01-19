# PickPlaz STM32 to ESP32 Port (Repo Root)

This root contains the original STM32 firmware alongside the ESP32-C3 port
workspace and local QEMU assets used for emulation-based porting tests.

# This is a PCBWay sponsored project
<img width="262" height="192" alt="PCBWAY" src="https://github.com/user-attachments/assets/07538e30-3bec-4b2b-8a49-50e9ba6a9420" />


## Structure (top level)

- `application/`, `Core/`, `Drivers/`, `include/`, `src/` - STM32 firmware
  sources and Cube-style project layout.
- `ESP32Port/` - ESP32-C3 port workspace (PlatformIO, docs, ported firmware).
- `qemu/` - QEMU source tree used to emulate ESP32-C3 during porting tests.
- `qemu-install/` - Local QEMU install prefix (binaries/libs for the emulator).

## Documentation

- Git Hub Pages: [https://bazmundi.github.io/PickPlaz-ESP32-C3-Port/](https://bazmundi.github.io/PickPlaz-ESP32-C3-Port/)
