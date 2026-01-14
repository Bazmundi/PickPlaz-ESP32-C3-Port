#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../../.." && pwd)"
QEMU_BIN_DIR="${ROOT_DIR}/qemu-install/usr/local/bin"

if [[ ! -d "${QEMU_BIN_DIR}" ]]; then
    echo "QEMU install bin dir not found: ${QEMU_BIN_DIR}"
    echo "Expected qemu-install at repo root."
    exit 1
fi

mapfile -t qemu_bins < <(find "${QEMU_BIN_DIR}" -maxdepth 1 -type f -name "qemu-system-*" -printf "%f\n" | sort)

if [[ ${#qemu_bins[@]} -eq 0 ]]; then
    echo "No qemu-system-* binaries found in ${QEMU_BIN_DIR}"
    exit 1
fi

echo "QEMU system binaries in ${QEMU_BIN_DIR}:"
for bin in "${qemu_bins[@]}"; do
    echo " - ${bin}"
done

echo "QEMU versions:"
for bin in "${qemu_bins[@]}"; do
    "${QEMU_BIN_DIR}/${bin}" --version
done

if [[ -x "${QEMU_BIN_DIR}/qemu-system-riscv32" ]]; then
    "${QEMU_BIN_DIR}/qemu-system-riscv32" -machine help >/dev/null
    echo "qemu-system-riscv32 machine list OK"
fi
