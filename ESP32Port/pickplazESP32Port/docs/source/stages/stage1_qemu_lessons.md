# Stage 1 QEMU Lessons Learned (ESP32-C3 Port)

This document captures what went wrong while bringing up QEMU for Stage 1,
what we tried, what failed, and what ultimately worked.

## Summary
QEMU booted the Stage 1 firmware once three things aligned: correct build
outputs for PlatformIO, a UART console (QEMU does not emulate USB Serial/JTAG),
and a flash image size that matched the firmware header. The biggest blockers
were path assumptions, console routing, and a 2MB vs 4MB flash mismatch.

## What went wrong
### 1) Script paths and working directory assumptions
- Early attempts used scripts that did not exist in this repo, producing
  "No such file or directory".
- Running `pio run` from the repo root failed because it is not a PlatformIO
  project directory.

### 2) Merge script assumptions did not match PlatformIO outputs
- Initial merge logic expected `bootloader/bootloader.bin` and
  `partition_table/partition-table.bin`, but the build produced
  `bootloader.bin` and `partitions.bin` at different paths.
- Result: merge failed or used incorrect inputs.

### 3) Console routed to USB Serial/JTAG
- QEMU does not emulate USB Serial/JTAG, so logs never appeared even though
  the firmware was running.

### 4) Flash size mismatch caused repeated reboots
- `flash_image.bin` was 2MB, but the firmware header indicated 4MB.
- Boot log showed `Detected size(2048k) smaller than the size in the binary
  image header(4096k)` followed by an assert and reboot loop.

### 5) Monitor socket race and port conflicts
- `pio device monitor` exits immediately on connection refused, so starting it
  before QEMU guarantees failure.
- Port 5555 was already in use in one run, preventing QEMU from starting.

## What we tried (and failed)
### Running QEMU without correct build artifacts
- Running merge/run scripts before building the `esp32-c3-qemu` environment
  led to missing bin files.

### Running QEMU with USB console config
- Using the hardware `sdkconfig` (USB Serial/JTAG console) resulted in no
  output in QEMU.

### Launching monitor before QEMU
- `pio device monitor -p socket://localhost:4444` failed with
  "Connection refused" because QEMU had not opened the socket yet.

## What we tried (and succeeded)
### 1) Align build outputs with merge expectations
- `merge_flash.sh` now auto-detects:
  - `bootloader.bin` (or `bootloader/bootloader.bin`)
  - `partitions.bin` (or `partition_table/partition-table.bin`)
  - `pickplazESP32Port.bin` (or `firmware.bin`)
- QEMU scripts now accept `PIO_ENV` to target the correct build directory.

### 2) Add a QEMU-specific console configuration
- Added `sdkconfig.qemu` with UART0 console and USB Serial/JTAG disabled.
- Added `[env:esp32-c3-qemu]` in `platformio.ini`.

### 3) Pin flash size to 4MB
- Set `board_build.flash_size = 4MB` to match the device.
- Rebuilt and re-merged to create a 4MB `flash_image.bin`.
- Result: boot continues past flash init with no assert.

### 4) Remove the socket race
- Added `scripts/run_qemu_with_monitor.sh` to:
  - start QEMU
  - wait for the serial socket
  - attach `pio device monitor`

## Final working flow
```bash
pio run -e esp32-c3-qemu -t clean
pio run -e esp32-c3-qemu
scripts/merge_flash.sh
scripts/run_qemu_with_monitor.sh
```

Alternative two-terminal flow:
```bash
# terminal 1
scripts/run_qemu.sh

# terminal 2
pio device monitor -p socket://localhost:4444 -b 115200
```

## Recommendations for Stage 2+
1) Use `esp32-c3-qemu` only for QEMU and `esp32-c3-devkitm-1` for hardware.
2) After config changes, clean build and verify:
   - `.pio/build/esp32-c3-qemu/flash_args` shows `--flash_size 4MB`
   - `.pio/build/esp32-c3-qemu/flash_image.bin` is 4,194,304 bytes
3) Prefer `run_qemu_with_monitor.sh` to avoid socket ordering issues.
4) If QEMU is silent, double-check console settings in `sdkconfig.qemu`.
