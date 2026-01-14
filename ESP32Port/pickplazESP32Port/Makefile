PIO ?= pio
PORT ?= /dev/ttyACM0
BAUD ?= 115200

PIO_ENV_DEVKIT ?= esp32-c3-devkitm-1
PIO_ENV_QEMU ?= esp32-c3-qemu

QEMU_SERIAL_PORT ?= 4444
QEMU_MONITOR_PORT ?= 5555

SCRIPTS_DIR := scripts

.PHONY: help \
	build-devkit build-qemu clean-devkit clean-qemu \
	merge-qemu qemu-run qemu-run-stdio qemu-run-monitor qemu-smoke \
	monitor-qemu monitor-usb flash flash-port qemu-reset \
	qemu-all qemu-run-fast devkit-flash-monitor

help:
	@printf "Targets:\n"
	@printf "  build-devkit      Build for ESP32-C3 devkit\n"
	@printf "  build-qemu        Build for ESP32-C3 QEMU\n"
	@printf "  clean-devkit      Clean devkit build\n"
	@printf "  clean-qemu        Clean QEMU build\n"
	@printf "  merge-qemu        Merge QEMU flash image\n"
	@printf "  qemu-run          Run QEMU with TCP serial\n"
	@printf "  qemu-run-stdio    Run QEMU with stdio serial\n"
	@printf "  qemu-run-monitor  Run QEMU and attach monitor\n"
	@printf "  qemu-smoke        QEMU smoke test\n"
	@printf "  monitor-qemu      Attach PIO monitor to QEMU socket\n"
	@printf "  monitor-usb       Attach PIO monitor to USB serial\n"
	@printf "  flash             Upload to devkit (auto port)\n"
	@printf "  flash-port        Upload to devkit using PORT=<device>\n"
	@printf "  qemu-reset        Print QEMU reset instructions\n"
	@printf "  qemu-all          Build + merge + run QEMU with monitor\n"
	@printf "  qemu-run-fast     Merge + run QEMU with monitor\n"
	@printf "  devkit-flash-monitor  Flash devkit then attach monitor\n"
	@printf "\n"
	@printf "Notes:\n"
	@printf "  QEMU socket monitor must attach after QEMU starts.\n"
	@printf "  Use qemu-run-monitor or qemu-all to avoid the race.\n"
	@printf "  If monitor-qemu fails, run qemu-run first, then monitor-qemu.\n"

build-devkit:
	$(PIO) run -e $(PIO_ENV_DEVKIT)

build-qemu:
	$(PIO) run -e $(PIO_ENV_QEMU)

clean-devkit:
	$(PIO) run -e $(PIO_ENV_DEVKIT) -t clean

clean-qemu:
	$(PIO) run -e $(PIO_ENV_QEMU) -t clean

merge-qemu:
	PIO_ENV=$(PIO_ENV_QEMU) $(SCRIPTS_DIR)/merge_flash.sh

qemu-run:
	PIO_ENV=$(PIO_ENV_QEMU) QEMU_SERIAL_PORT=$(QEMU_SERIAL_PORT) \
	QEMU_MONITOR_PORT=$(QEMU_MONITOR_PORT) $(SCRIPTS_DIR)/run_qemu.sh

qemu-run-stdio:
	PIO_ENV=$(PIO_ENV_QEMU) $(SCRIPTS_DIR)/run_flash_qemu.sh

qemu-run-monitor:
	PIO_ENV=$(PIO_ENV_QEMU) QEMU_SERIAL_PORT=$(QEMU_SERIAL_PORT) \
	QEMU_MONITOR_PORT=$(QEMU_MONITOR_PORT) MONITOR_BAUD=$(BAUD) \
	$(SCRIPTS_DIR)/run_qemu_with_monitor.sh

qemu-smoke:
	$(SCRIPTS_DIR)/qemu_smoke.sh

monitor-qemu:
	@printf "Attaching QEMU monitor to localhost:%s...\n" "$(QEMU_SERIAL_PORT)"
	@if ! $(PIO) device monitor -p socket://localhost:$(QEMU_SERIAL_PORT) -b $(BAUD); then \
		echo ""; \
		echo "Monitor failed. QEMU may not be running yet."; \
		echo "Start it first with: make qemu-run"; \
		echo "Or use: make qemu-run-monitor"; \
		exit 1; \
	fi

monitor-usb:
	$(PIO) device monitor -p $(PORT) -b $(BAUD)

flash:
	$(PIO) run -e $(PIO_ENV_DEVKIT) -t upload

flash-port:
	$(PIO) run -e $(PIO_ENV_DEVKIT) -t upload --upload-port $(PORT)

qemu-reset:
	@printf "telnet localhost %s\n" "$(QEMU_MONITOR_PORT)"
	@printf "system_reset\n"

qemu-all: build-qemu merge-qemu qemu-run-monitor

qemu-run-fast: merge-qemu qemu-run-monitor

devkit-flash-monitor: flash monitor-usb
