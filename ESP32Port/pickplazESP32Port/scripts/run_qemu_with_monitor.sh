#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ROOT_DIR="$(cd "${PROJECT_DIR}/../.." && pwd)"

PIO_ENV="${PIO_ENV:-esp32-c3-qemu}"
SERIAL_PORT="${QEMU_SERIAL_PORT:-4444}"
MONITOR_PORT="${QEMU_MONITOR_PORT:-5555}"
MONITOR_BAUD="${MONITOR_BAUD:-115200}"
WAIT_SECONDS="${QEMU_WAIT_SECONDS:-5}"

BUILD_DIR="${PROJECT_DIR}/.pio/build/${PIO_ENV}"
FLASH_IMAGE="${BUILD_DIR}/flash_image.bin"
QEMU_PATH="${ROOT_DIR}/qemu-install/usr/local/bin/qemu-system-riscv32"

if [[ ! -f "${FLASH_IMAGE}" ]]; then
    echo "Error: Flash image not found at ${FLASH_IMAGE}"
    echo "Run scripts/merge_flash.sh first."
    exit 1
fi

if [[ ! -x "${QEMU_PATH}" ]]; then
    echo "Error: QEMU not found at ${QEMU_PATH}"
    exit 1
fi

echo "Starting QEMU with serial socket..."
echo "Build environment: ${PIO_ENV}"
echo "Flash image: ${FLASH_IMAGE}"
echo "Serial port: localhost:${SERIAL_PORT}"
echo "Monitor port: localhost:${MONITOR_PORT}"
echo ""

"${QEMU_PATH}" \
  -machine esp32c3 \
  -nographic \
  -drive file="${FLASH_IMAGE}",if=mtd,format=raw \
  -serial "tcp::${SERIAL_PORT},server,nowait" \
  -monitor "telnet::${MONITOR_PORT},server,nowait" &

QEMU_PID=$!

cleanup() {
    if kill -0 "${QEMU_PID}" 2>/dev/null; then
        kill "${QEMU_PID}" >/dev/null 2>&1 || true
    fi
}
trap cleanup EXIT

echo "Waiting for serial socket to open..."
max_tries=$((WAIT_SECONDS * 10))
for ((i=0; i<max_tries; i++)); do
    if (echo > "/dev/tcp/127.0.0.1/${SERIAL_PORT}") >/dev/null 2>&1; then
        break
    fi
    sleep 0.1
done

if ! (echo > "/dev/tcp/127.0.0.1/${SERIAL_PORT}") >/dev/null 2>&1; then
    echo "Error: serial socket did not open within ${WAIT_SECONDS}s."
    echo "Check QEMU output for errors."
    exit 1
fi

echo "Launching monitor..."
pio device monitor -p "socket://localhost:${SERIAL_PORT}" -b "${MONITOR_BAUD}"
