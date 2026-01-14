#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PIO_ENV="${PIO_ENV:-esp32-c3-qemu}"
BUILD_DIR="${PROJECT_DIR}/.pio/build/${PIO_ENV}"
OUTPUT_FILE="${BUILD_DIR}/flash_image.bin"
ESPTOOL="${HOME}/.platformio/packages/tool-esptoolpy/esptool.py"
PIO_PYTHON="${HOME}/.platformio/penv/bin/python"
FLASH_ARGS="${BUILD_DIR}/flash_args"

flash_mode="dio"
flash_freq="80m"
flash_size="2MB"

if [[ -f "${FLASH_ARGS}" ]]; then
    parsed_mode=$(awk '{for (i=1;i<=NF;i++) if ($i=="--flash_mode") print $(i+1)}' "${FLASH_ARGS}")
    parsed_freq=$(awk '{for (i=1;i<=NF;i++) if ($i=="--flash_freq") print $(i+1)}' "${FLASH_ARGS}")
    parsed_size=$(awk '{for (i=1;i<=NF;i++) if ($i=="--flash_size") print $(i+1)}' "${FLASH_ARGS}")
    if [[ -n "${parsed_mode}" ]]; then
        flash_mode="${parsed_mode}"
    fi
    if [[ -n "${parsed_freq}" ]]; then
        flash_freq="${parsed_freq}"
    fi
    if [[ -n "${parsed_size}" ]]; then
        flash_size="${parsed_size}"
    fi
fi

echo "Merging ESP32-C3 binaries into flash image..."
echo "Build environment: ${PIO_ENV}"

if [[ ! -d "${BUILD_DIR}" ]]; then
    echo "Error: build directory not found at ${BUILD_DIR}"
    echo "Build with: pio run -e ${PIO_ENV}"
    exit 1
fi

BOOTLOADER_BIN=""
if [[ -f "${BUILD_DIR}/bootloader/bootloader.bin" ]]; then
    BOOTLOADER_BIN="${BUILD_DIR}/bootloader/bootloader.bin"
elif [[ -f "${BUILD_DIR}/bootloader.bin" ]]; then
    BOOTLOADER_BIN="${BUILD_DIR}/bootloader.bin"
fi

APP_BIN=""
if [[ -f "${BUILD_DIR}/pickplazESP32Port.bin" ]]; then
    APP_BIN="${BUILD_DIR}/pickplazESP32Port.bin"
elif [[ -f "${BUILD_DIR}/firmware.bin" ]]; then
    APP_BIN="${BUILD_DIR}/firmware.bin"
fi

PARTITION_BIN=""
if [[ -f "${BUILD_DIR}/partition_table/partition-table.bin" ]]; then
    PARTITION_BIN="${BUILD_DIR}/partition_table/partition-table.bin"
elif [[ -f "${BUILD_DIR}/partitions.bin" ]]; then
    PARTITION_BIN="${BUILD_DIR}/partitions.bin"
fi

if [[ -z "${BOOTLOADER_BIN}" ]]; then
    echo "Error: bootloader bin not found."
    echo "Checked:"
    echo "  ${BUILD_DIR}/bootloader/bootloader.bin"
    echo "  ${BUILD_DIR}/bootloader.bin"
    exit 1
fi

if [[ -z "${APP_BIN}" ]]; then
    echo "Error: app bin not found."
    echo "Checked:"
    echo "  ${BUILD_DIR}/pickplazESP32Port.bin"
    echo "  ${BUILD_DIR}/firmware.bin"
    exit 1
fi

if [[ -z "${PARTITION_BIN}" ]]; then
    echo "Error: partition table bin not found."
    echo "Checked:"
    echo "  ${BUILD_DIR}/partition_table/partition-table.bin"
    echo "  ${BUILD_DIR}/partitions.bin"
    exit 1
fi

if [[ ! -f "${ESPTOOL}" ]]; then
    echo "Error: esptool.py not found at ${ESPTOOL}"
    exit 1
fi

PYTHON_BIN="python3"
if [[ -x "${PIO_PYTHON}" ]]; then
    PYTHON_BIN="${PIO_PYTHON}"
fi

"${PYTHON_BIN}" "${ESPTOOL}" \
    --chip esp32c3 merge_bin \
    --flash_mode "${flash_mode}" \
    --flash_freq "${flash_freq}" \
    --flash_size "${flash_size}" \
    0x0 "${BOOTLOADER_BIN}" \
    0x8000 "${PARTITION_BIN}" \
    0x10000 "${APP_BIN}" \
    -o "${OUTPUT_FILE}" --fill-flash-size "${flash_size}"

echo "Created flash image: ${OUTPUT_FILE}"
echo "Flash image size: $(ls -la "${OUTPUT_FILE}" | awk '{print $5}') bytes"
