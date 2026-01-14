PIO ?= pio
PORT ?= /dev/ttyACM0
BAUD ?= 115200

PIO_ENV_DEVKIT ?= esp32-c3-devkitm-1
PIO_ENV_QEMU ?= esp32-c3-qemu
PIO_ENV_QEMU_NO_SELFTEST ?= esp32-c3-qemu-noselftest
PICSIMLAB ?= picsimlab

QEMU_SERIAL_PORT ?= 4444
QEMU_MONITOR_PORT ?= 5555

SCRIPTS_DIR := scripts

.PHONY: help \
	build-devkit build-qemu build-qemu-noselftest clean-devkit clean-qemu clean-qemu-noselftest \
	merge-qemu merge-qemu-noselftest qemu-run qemu-run-stdio qemu-run-monitor \
	qemu-run-monitor-noselftest qemu-smoke \
	monitor-qemu monitor-usb flash flash-port qemu-reset picsimlab qemu-picsimlab \
	qemu-picsimlab-noselftest picsimlab-standalone picsimlab-standalone-noselftest \
	picsimlab-external picsimlab-external-noselftest \
	qemu-all qemu-run-fast qemu-all-noselftest qemu-run-fast-noselftest devkit-flash-monitor

help:
	@printf "Targets:\n"
	@printf "  build-devkit      Build for ESP32-C3 devkit\n"
	@printf "  build-qemu        Build for ESP32-C3 QEMU\n"
	@printf "  build-qemu-noselftest  Build QEMU firmware without self-test\n"
	@printf "  clean-devkit      Clean devkit build\n"
	@printf "  clean-qemu        Clean QEMU build\n"
	@printf "  clean-qemu-noselftest  Clean QEMU no-selftest build\n"
	@printf "  merge-qemu        Merge QEMU flash image\n"
	@printf "  merge-qemu-noselftest  Merge QEMU no-selftest flash image\n"
	@printf "  qemu-run          Run QEMU with TCP serial\n"
	@printf "  qemu-run-stdio    Run QEMU with stdio serial\n"
	@printf "  qemu-run-monitor  Run QEMU and attach monitor\n"
	@printf "  qemu-run-monitor-noselftest  Run QEMU (no self-test) with monitor\n"
	@printf "  qemu-smoke        QEMU smoke test\n"
	@printf "  monitor-qemu      Attach PIO monitor to QEMU socket\n"
	@printf "  monitor-usb       Attach PIO monitor to USB serial\n"
	@printf "  flash             Upload to devkit (auto port)\n"
	@printf "  flash-port        Upload to devkit using PORT=<device>\n"
	@printf "  qemu-reset        Print QEMU reset instructions\n"
	@printf "  picsimlab         Launch PICSimLab GUI\n"
	@printf "  qemu-picsimlab    Run QEMU for PICSimLab external integration\n"
	@printf "  picsimlab-standalone  Build+merge then launch PICSimLab (built-in QEMU)\n"
	@printf "  picsimlab-external    Build+merge then run QEMU for PICSimLab (use another terminal for picsimlab)\n"
	@printf "  picsimlab-standalone-noselftest  Build+merge then launch PICSimLab without self-test\n"
	@printf "  picsimlab-external-noselftest    Build+merge then run QEMU without self-test (use another terminal for picsimlab)\n"
	@printf "  qemu-all          Build + merge + run QEMU with monitor\n"
	@printf "  qemu-run-fast     Merge + run QEMU with monitor\n"
	@printf "  qemu-all-noselftest  Build + merge + run QEMU without self-test\n"
	@printf "  qemu-run-fast-noselftest  Merge + run QEMU without self-test\n"
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

build-qemu-noselftest:
	$(PIO) run -e $(PIO_ENV_QEMU_NO_SELFTEST)

clean-devkit:
	$(PIO) run -e $(PIO_ENV_DEVKIT) -t clean

clean-qemu:
	$(PIO) run -e $(PIO_ENV_QEMU) -t clean

clean-qemu-noselftest:
	$(PIO) run -e $(PIO_ENV_QEMU_NO_SELFTEST) -t clean

merge-qemu:
	PIO_ENV=$(PIO_ENV_QEMU) $(SCRIPTS_DIR)/merge_flash.sh

merge-qemu-noselftest:
	PIO_ENV=$(PIO_ENV_QEMU_NO_SELFTEST) $(SCRIPTS_DIR)/merge_flash.sh

qemu-run:
	PIO_ENV=$(PIO_ENV_QEMU) QEMU_SERIAL_PORT=$(QEMU_SERIAL_PORT) \
	QEMU_MONITOR_PORT=$(QEMU_MONITOR_PORT) $(SCRIPTS_DIR)/run_qemu.sh

qemu-run-stdio:
	PIO_ENV=$(PIO_ENV_QEMU) $(SCRIPTS_DIR)/run_flash_qemu.sh

qemu-run-monitor:
	PIO_ENV=$(PIO_ENV_QEMU) QEMU_SERIAL_PORT=$(QEMU_SERIAL_PORT) \
	QEMU_MONITOR_PORT=$(QEMU_MONITOR_PORT) MONITOR_BAUD=$(BAUD) \
	$(SCRIPTS_DIR)/run_qemu_with_monitor.sh

qemu-run-monitor-noselftest:
	PIO_ENV=$(PIO_ENV_QEMU_NO_SELFTEST) QEMU_SERIAL_PORT=$(QEMU_SERIAL_PORT) \
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

picsimlab:
	$(PICSIMLAB)

qemu-picsimlab:
	PIO_ENV=$(PIO_ENV_QEMU) $(SCRIPTS_DIR)/run_qemu_external.sh

qemu-picsimlab-noselftest:
	PIO_ENV=$(PIO_ENV_QEMU_NO_SELFTEST) $(SCRIPTS_DIR)/run_qemu_external.sh

picsimlab-standalone: build-qemu merge-qemu picsimlab

picsimlab-external: build-qemu merge-qemu
	@echo "Starting QEMU for PICSimLab external integration..."
	@echo "In another terminal, run: make picsimlab"
	$(MAKE) qemu-picsimlab

picsimlab-standalone-noselftest: build-qemu-noselftest merge-qemu-noselftest picsimlab

picsimlab-external-noselftest: build-qemu-noselftest merge-qemu-noselftest
	@echo "Starting QEMU (no self-test) for PICSimLab external integration..."
	@echo "In another terminal, run: make picsimlab"
	$(MAKE) qemu-picsimlab-noselftest

qemu-all: build-qemu merge-qemu qemu-run-monitor

qemu-run-fast: merge-qemu qemu-run-monitor

qemu-all-noselftest: build-qemu-noselftest merge-qemu-noselftest qemu-run-monitor-noselftest

qemu-run-fast-noselftest: merge-qemu-noselftest qemu-run-monitor-noselftest

devkit-flash-monitor: flash monitor-usb
