# PICSimLab Integration (ESP32-C3)

This guide captures how to use PICSimLab with the ESP32-C3 port for a GUI view
of the QEMU run and GPIO state.

## Prerequisites
- PICSimLab installed (`picsimlab` on PATH).
- QEMU binaries installed at repo root (`qemu-install/`).
- A merged flash image produced by `scripts/merge_flash.sh`.

## Quick start targets
- Standalone PICSimLab (built-in QEMU): `make picsimlab-standalone`
- Standalone PICSimLab (no self-test): `make picsimlab-standalone-noselftest`
- External QEMU + PICSimLab: `make picsimlab-external` then in another terminal
  run `make picsimlab`
- External QEMU + PICSimLab (no self-test): `make picsimlab-external-noselftest`
  then in another terminal run `make picsimlab`

## Option A: PICSimLab built-in QEMU (standalone)
Use PICSimLab without the external QEMU process.

1) Build and merge:
```bash
make build-qemu
make merge-qemu
```

2) Launch PICSimLab:
```bash
make picsimlab
```

3) Select board:
   - **ESP32-C3-DevKitC-02**

4) Load firmware:
   - Prefer `flash_image.bin` from:
     `.pio/build/esp32-c3-qemu/flash_image.bin`
   - If PICSimLab requests an app-only image, use:
     `.pio/build/esp32-c3-qemu/firmware.bin`

5) Open Pin Viewer (optional):
   - Use **View → Pin Viewer** to inspect GPIO state changes.

## Option B: PICSimLab + external QEMU (TCP serial)
Use PICSimLab as a GUI on top of our custom QEMU process.

1) Build and merge:
```bash
make build-qemu
make merge-qemu
```

2) Start QEMU with external integration flags:
```bash
make qemu-picsimlab
```

3) Launch PICSimLab:
```bash
make picsimlab
```

4) Configure PICSimLab serial:
   - Serial port: `tcp:localhost:4444`

5) Optional GDB:
   - QEMU exposes `localhost:1234` for GDB.

## Notes
- If QEMU fails to start, make sure ports 4444/5555/1234 are free.
- To disable self-test in QEMU:
  `PIO_ENV=esp32-c3-qemu-noselftest make qemu-picsimlab`
