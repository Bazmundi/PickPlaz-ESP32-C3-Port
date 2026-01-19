# Makefile for rendering Manim GIFs from docs/visuals.

MANIM ?= manim
MANIM_FLAGS ?= -pql --format=gif
DOCS_VENV ?= ../.venv

SINE_FILE := sine_wave_scene.py
PHASE_FILE := led_phase_scene.py
SCROLL_FILE := led_phase_scroll_scene.py
OTHER_FILE := led_other_states_scene.py

ifneq ($(wildcard $(DOCS_VENV)/bin/manim),)
  ifeq ($(MANIM),manim)
    MANIM := $(DOCS_VENV)/bin/manim
  endif
endif

.PHONY: help all sine phase scroll-left scroll-right \
	idle-indexed idle-unindexed default-sine export

help:
	@printf "Targets:\n"
	@printf "  sine            Sine table graph (SineTableScene)\n"
	@printf "  phase           Phase offsets + LED row (LedPhaseScene)\n"
	@printf "  scroll-left     Right-to-left scroll (LedPhaseScrollReverseScene)\n"
	@printf "  scroll-right    Left-to-right scroll (LedPhaseScrollScene)\n"
	@printf "  idle-indexed    Idle/indexed (IdleIndexedScene)\n"
	@printf "  idle-unindexed  Idle/unindexed (IdleUnindexedScene)\n"
	@printf "  default-sine    Default sine (DefaultSineScene)\n"
	@printf "  all             Render all scenes\n"
	@printf "  export          Copy rendered GIF to docs/source/images\n"
	@printf "                   usage: make export SCENE=phase NAME=Forward\n"
	@printf "\nVariables:\n"
	@printf "  MANIM=%s\n" "$(MANIM)"
	@printf "  MANIM_FLAGS=%s\n" "$(MANIM_FLAGS)"
	@printf "  SCENE=<target> NAME=<output name without .gif>\n"

all: sine phase scroll-left scroll-right idle-indexed idle-unindexed default-sine

sine:
	@$(MANIM) $(MANIM_FLAGS) $(SINE_FILE) SineTableScene

phase:
	@$(MANIM) $(MANIM_FLAGS) $(PHASE_FILE) LedPhaseScene

scroll-left:
	@$(MANIM) $(MANIM_FLAGS) $(SCROLL_FILE) LedPhaseScrollReverseScene

scroll-right:
	@$(MANIM) $(MANIM_FLAGS) $(SCROLL_FILE) LedPhaseScrollScene

idle-indexed:
	@$(MANIM) $(MANIM_FLAGS) $(OTHER_FILE) IdleIndexedScene

idle-unindexed:
	@$(MANIM) $(MANIM_FLAGS) $(OTHER_FILE) IdleUnindexedScene

default-sine:
	@$(MANIM) $(MANIM_FLAGS) $(OTHER_FILE) DefaultSineScene


export:
	@if [ -z "$(SCENE)" ] || [ -z "$(NAME)" ]; then \
		echo "Usage: make export SCENE=<target> NAME=<output name>"; \
		exit 2; \
	fi
	@case "$(SCENE)" in \
		sine) SRC="media/videos/$(SINE_FILE:.py=)/480p15/SineTableScene_ManimCE_v0.19.2.gif" ;; \
		phase) SRC="media/videos/$(PHASE_FILE:.py=)/480p15/LedPhaseScene_ManimCE_v0.19.2.gif" ;; \
		scroll-left) SRC="media/videos/$(SCROLL_FILE:.py=)/480p15/LedPhaseScrollReverseScene_ManimCE_v0.19.2.gif" ;; \
		scroll-right) SRC="media/videos/$(SCROLL_FILE:.py=)/480p15/LedPhaseScrollScene_ManimCE_v0.19.2.gif" ;; \
		idle-indexed) SRC="media/videos/$(OTHER_FILE:.py=)/480p15/IdleIndexedScene_ManimCE_v0.19.2.gif" ;; \
		idle-unindexed) SRC="media/videos/$(OTHER_FILE:.py=)/480p15/IdleUnindexedScene_ManimCE_v0.19.2.gif" ;; \
		default-sine) SRC="media/videos/$(OTHER_FILE:.py=)/480p15/DefaultSineScene_ManimCE_v0.19.2.gif" ;; \
		*) echo "Unknown SCENE: $(SCENE)"; exit 2 ;; \
	esac; \
	if [ ! -f "$$SRC" ]; then \
		echo "Missing render: $$SRC"; \
		exit 1; \
	fi; \
	cp "$$SRC" "../source/images/$(NAME).gif"; \
	echo "Copied $$SRC -> ../source/images/$(NAME).gif"
