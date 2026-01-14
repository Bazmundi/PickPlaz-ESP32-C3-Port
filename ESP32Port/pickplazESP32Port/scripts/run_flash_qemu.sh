#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ROOT_DIR="$(cd "${PROJECT_DIR}/../.." && pwd)"
PIO_ENV="${PIO_ENV:-esp32-c3-qemu}"
BUILD_DIR="${PROJECT_DIR}/.pio/build/${PIO_ENV}"
FLASH_IMAGE="${BUILD_DIR}/flash_image.bin"
QEMU_PATH="${ROOT_DIR}/qemu-install/usr/local/bin/qemu-system-riscv32"

if [[ ! -f "${FLASH_IMAGE}" ]]; then
    echo "Error: Flash image not found at ${FLASH_IMAGE}"
    echo "Run scripts/merge_flash.sh first."
    exit 1
fi

if [[ ! -f "${QEMU_PATH}" ]]; then
    echo "Error: QEMU not found at ${QEMU_PATH}"
    exit 1
fi

echo "QEMU ESP32-C3 flash image runner"
echo "Build environment: ${PIO_ENV}"
echo "Flash image: ${FLASH_IMAGE}"
echo "Monitor: telnet localhost 5555"
echo "Press Ctrl+C to stop"
echo ""

"${QEMU_PATH}" \
  -machine esp32c3 \
  -nographic \
  -drive file="${FLASH_IMAGE}",if=mtd,format=raw \
  -chardev stdio,id=conout,signal=off \
  -serial chardev:conout \
  -monitor telnet::5555,server,nowait

echo "QEMU emulation stopped"
