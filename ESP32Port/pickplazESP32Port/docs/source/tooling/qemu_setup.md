# QEMU Setup for ESP32-C3 Emulation

This guide captures the QEMU setup used in the earlier esp32version folder,
adapted to the current ESP32 port project structure.

## Prerequisites
- Ubuntu/Debian-based Linux
- PlatformIO installed
- ESP-IDF toolchain available via PlatformIO
- Build tools (git, make)

## Installation Steps

### 1) Install dependencies
```bash
sudo apt-get update
sudo apt-get install libslirp-dev libglib2.0-dev libpixman-1-dev
sudo apt-get install gdb-multiarch telnet
```

### 2) Build and install Espressif QEMU fork
```bash
REPO_ROOT=/home/asterion/3D/pick-plaz/pickplazfeederstm_ESP32Port
cd "$REPO_ROOT"

git clone https://github.com/espressif/qemu.git
cd qemu

./configure --target-list=riscv32-softmmu
make -j$(nproc)
make install PREFIX="$REPO_ROOT/qemu-install"
```

QEMU binaries will be at:
```
${REPO_ROOT}/qemu-install/usr/local/bin/
```

### 3) Verify QEMU
```bash
$REPO_ROOT/qemu-install/usr/local/bin/qemu-system-riscv32 --version
$REPO_ROOT/qemu-install/usr/local/bin/qemu-system-riscv32 -machine help | grep esp32
```

## Build Firmware
```bash
cd /home/asterion/3D/pick-plaz/pickplazfeederstm_ESP32Port/ESP32Port/pickplazESP32Port
pio run -e esp32-c3-qemu
```

The `esp32-c3-qemu` environment uses `sdkconfig.qemu` to force the console
to UART0 (QEMU does not emulate USB Serial/JTAG). For hardware builds, use
`pio run -e esp32-c3-devkitm-1`.
Flash size is pinned to 4MB in `platformio.ini` to match the device and
avoid header/flash-size mismatches.

Create a merged flash image (required by the QEMU scripts). The script
auto-detects PlatformIO output filenames (e.g., `firmware.bin`,
`bootloader.bin`, `partitions.bin`) and uses `flash_args` only for flash
settings (mode/freq/size). It prefers PlatformIO's Python runtime if present.
```bash
ESP32Port/pickplazESP32Port/scripts/merge_flash.sh
```
To use a different build environment, set `PIO_ENV`, for example:
```bash
PIO_ENV=esp32-c3-qemu ESP32Port/pickplazESP32Port/scripts/merge_flash.sh
```

## Run QEMU

### One-command run (QEMU + monitor)
```bash
ESP32Port/pickplazESP32Port/scripts/run_qemu_with_monitor.sh
```

This starts QEMU, waits for the serial socket, and then attaches
`pio device monitor`. It avoids the connection‑refused race.

### Basic run (TCP serial + monitor)
```bash
ESP32Port/pickplazESP32Port/scripts/run_qemu.sh
```

Serial console (TCP):
```bash
telnet localhost 4444
```

Order matters: start QEMU first, then attach the monitor.
```bash
# terminal 1
ESP32Port/pickplazESP32Port/scripts/run_qemu.sh

# terminal 2
pio device monitor -p socket://localhost:4444 -b 115200
```

Monitor:
```bash
telnet localhost 5555
```

### Console on stdio
```bash
ESP32Port/pickplazESP32Port/scripts/run_flash_qemu.sh
```

### External/PICSimLab integration
```bash
ESP32Port/pickplazESP32Port/scripts/run_qemu_external.sh
```

## Debugging with GDB
Run QEMU with `-s -S` (see `run_qemu_external.sh`) then:
```bash
${HOME}/.platformio/packages/toolchain-riscv32-esp/bin/riscv32-esp-elf-gdb \
  /home/asterion/3D/pick-plaz/pickplazfeederstm_ESP32Port/ESP32Port/pickplazESP32Port/.pio/build/esp32-c3-qemu/firmware.elf

(gdb) target remote localhost:1234
(gdb) monitor r gdb_sync
(gdb) maintenance flush register-cache
(gdb) continue
```

## Limitations
- ADC input simulation is limited.
- PWM timing may not match real hardware.
- WiFi/BLE not emulated.
- Serial output behavior may differ from the real board.

## Troubleshooting
1) QEMU not found:
   - Ensure `qemu-install` exists at repo root.
2) No ESP32 machine:
   - Ensure you used Espressif's QEMU fork.
3) No serial output:
   - Use `run_flash_qemu.sh` for stdio or `telnet localhost 4444` for TCP.
