# Stage 1 Lessons Learned (ESP32-C3 Port)

This document captures what went wrong during Stage 1 (boot + logging),
what we tried, what failed, and what ultimately worked.

## Summary
Stage 1 succeeded once the console was routed to USB Serial/JTAG and the
PlatformIO build actually consumed the project `sdkconfig`. The board was
not "fragged"; the biggest blockers were (1) console output going to UART0
instead of USB, and (2) host USB enumeration flapping leading PIO to guess
an invalid port.

## What went wrong
### 1) Build failed due to log format warnings
- Initial boot log included `esp_chip_info_t` fields and flash size.
- IDF treated format warnings as errors (`cc1: some warnings being treated as errors`).

### 2) No serial output after upload
- The monitor connected at 9600 baud initially, which is wrong for IDF.
- Even after setting 115200, nothing printed because the console was still
  configured for UART0, not USB Serial/JTAG.

### 3) Upload failures after first flash
- PlatformIO auto-detected `/dev/ttyS0` (non-existent), causing upload failures.
- Linux did not show `/dev/ttyACM*` or `/dev/serial/by-id` at times, which
  made it look like the board disappeared.
- The USB device enumerated and then disconnected repeatedly ("flapping"),
  making the port unstable during reset/boot transitions.

### 4) Diagnostics blocked by permissions/tools
- `dmesg` access failed without elevated privileges.
- `rg` (ripgrep) was not installed on the host.
- `/dev/serial/by-id` did not exist on this system.

## What we tried (and failed)
### Formatting fixes
- Switched to `PRIu32` formatting for revision/flash size logs.
  - Still failed with format warning in this toolchain.
- Switched to `%u` with casts.
  - Still produced format-related build errors in the IDF log macros.

### Monitor changes without console reconfiguration
- Set `monitor_speed = 115200`.
  - No output, because the console was still UART0.

### Port discovery and monitor attempts
- `pio device monitor -p /dev/ttyACM0 -b 115200` when the device did not
  enumerate consistently.
  - Resulted in `Input/output error` or disconnect/reconnect loops.

### Upload attempts without a stable port
- PIO auto-detected `/dev/ttyS0` and failed.
  - Root cause was missing USB CDC device due to enumeration instability.

## What we tried (and succeeded)
### 1) Simplified the boot log (build stability)
- Removed chip info + flash size log lines.
- Kept a minimal boot log with IDF version.
- Result: build succeeded without format warnings.

### 2) Ensured the project `sdkconfig` was actually used
- Added `board_build.sdkconfig = sdkconfig.esp32-c3-devkitm-1` to
  `platformio.ini`.
- Result: the console settings in `sdkconfig` were honored.

### 3) Switched console output to USB Serial/JTAG
- In `sdkconfig.esp32-c3-devkitm-1`:
  - `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y`
  - `# CONFIG_ESP_CONSOLE_UART_DEFAULT is not set`
  - `CONFIG_ESP_CONSOLE_SECONDARY_NONE=y`
  - `# CONFIG_ESP_CONSOLE_UART is not set`
  - `CONFIG_ESP_CONSOLE_ROM_SERIAL_PORT_NUM=3`
- Result: logs appeared on `/dev/ttyACM*` over USB‑C.

### 4) Stabilized monitor behavior
- Added to `platformio.ini`:
  - `monitor_dtr = 0`
  - `monitor_rts = 0`
- Result: monitor stopped toggling the device, allowing logs to appear
  after a manual reset.

### 5) Confirmed successful boot logs
- After upload and reset, console showed:
  - ESP-IDF boot banner
  - `pickplaz: Booting PickPlaz ESP32-C3 port (Stage 1)`
  - `pickplaz: IDF version: 5.3.1`

## Host-side observations
### USB enumeration
- `lsusb` was the most reliable check:
  - `303a:1001 Espressif USB JTAG/serial debug unit` indicates the device
    enumerated.
- When `lsusb` showed no Espressif device, uploads failed because there was
  no CDC/ACM device for PIO to use.

### Boot sequence quirks
- The Super Mini required the BOOT/RESET sequence to enter download mode.
- A reconnect cycle was common, and monitor sometimes attached after the
  boot log already printed.

## Final working configuration
### `platformio.ini`
- `monitor_speed = 115200`
- `board_build.sdkconfig = sdkconfig.esp32-c3-devkitm-1`
- `monitor_dtr = 0`
- `monitor_rts = 0`

### `sdkconfig.esp32-c3-devkitm-1`
- USB Serial/JTAG console selected as primary output.
- UART console disabled.

## Recommendations for Stage 2+
1) Keep USB Serial/JTAG as the primary console to avoid GPIO20/21 conflicts.
2) Use `lsusb` and `/dev/ttyACM*` to verify enumeration before blaming firmware.
3) If logs disappear, verify the active build `sdkconfig` in
   `.pio/build/esp32-c3-devkitm-1/sdkconfig` rather than the source file.
4) For port flapping, keep BOOT/RESET manual and avoid automatic DTR/RTS.
