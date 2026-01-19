# QEMU ESP32-C3 Emulation Setup - Summary

## Status
QEMU emulation for the ESP32-C3 port is available as an optional tool.
It is meant for early smoke tests and inspection, not full hardware fidelity.

## Key artifacts
- QEMU install (ignored by git):
  - `/home/asterion/3D/pick-plaz/pickplazfeederstm_ESP32Port/qemu-install/`
- Scripts (tracked):
  - `ESP32Port/pickplazESP32Port/scripts/merge_flash.sh`
  - `ESP32Port/pickplazESP32Port/scripts/run_qemu.sh`
  - `ESP32Port/pickplazESP32Port/scripts/run_qemu_with_monitor.sh`
  - `ESP32Port/pickplazESP32Port/scripts/run_flash_qemu.sh`
  - `ESP32Port/pickplazESP32Port/scripts/run_qemu_external.sh`
  - `ESP32Port/pickplazESP32Port/scripts/qemu_smoke.sh`
- QEMU build config:
  - `ESP32Port/pickplazESP32Port/sdkconfig.qemu` (UART console for QEMU)
- Documentation:
  - `ESP32Port/pickplazESP32Port/docs/QEMU_SETUP.md`

## Typical flow
1) Build firmware:
   - `pio run -e esp32-c3-qemu`
2) Merge flash image:
   - `scripts/merge_flash.sh`
3) Run QEMU:
   - `scripts/run_qemu.sh`
4) Connect to serial/monitor:
   - `telnet localhost 4444`
   - `telnet localhost 5555`

Or use the helper to start QEMU and attach the monitor in one command:
- `scripts/run_qemu_with_monitor.sh`

The QEMU scripts default to `PIO_ENV=esp32-c3-qemu`. Set `PIO_ENV` if you
want to point them at a different build environment.
Flash size is pinned to 4MB in `platformio.ini` to match the device.

## Known limitations
- Peripheral emulation is incomplete (ADC/PWM timing).
- Serial behavior may differ from real hardware.
- Use QEMU for logic sanity, not timing validation.
