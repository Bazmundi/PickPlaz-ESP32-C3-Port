# Makefile for Sphinx documentation with Doxygen helpers
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
DOCS_VENV     ?= .venv
SPHINXBUILD   ?= sphinx-build
DOXYGEN       ?= doxygen
SOURCEDIR     = source
BUILDDIR      = build
BUILDDIR_INTERNAL = build-internal

ifneq ($(wildcard $(DOCS_VENV)/bin/sphinx-build),)
  ifeq ($(SPHINXBUILD),sphinx-build)
    SPHINXBUILD := $(DOCS_VENV)/bin/sphinx-build
  endif
endif

# Put it first so that "make" without argument is like "make help".
help:
	@printf "Custom targets:\n"
	@printf "  docs              Build Doxygen + Sphinx (public)\n"
	@printf "  docs-internal     Build internal Doxygen + Sphinx\n"
	@printf "  doxygen           Build public Doxygen output\n"
	@printf "  doxygen-internal  Build internal Doxygen output\n"
	@printf "  html-internal     Build internal Sphinx HTML\n"
	@printf "  clean             Remove Sphinx build output\n"
	@printf "  clean-doxygen     Remove Doxygen output\n"
	@printf "  clean-all         Remove Sphinx + Doxygen output\n"
	@printf "  venv              Create docs venv and install requirements (uv)\n"
	@printf "\nSphinx targets:\n"
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile docs docs-internal doxygen doxygen-internal html-internal \
	clean clean-doxygen clean-all venv

docs: doxygen html

docs-internal: doxygen-internal html-internal

doxygen:
	@$(DOXYGEN) Doxyfile

doxygen-internal:
	@$(DOXYGEN) Doxyfile.internal

html:
	@$(SPHINXBUILD) -M html "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

html-internal:
	@BREATHE_XML_DIR="$(CURDIR)/doxygen-internal/xml" \
	$(SPHINXBUILD) -M html "$(SOURCEDIR)" "$(BUILDDIR_INTERNAL)" $(SPHINXOPTS) $(O)

venv:
	@uv venv
	@uv pip install -r requirements.txt

clean:
	@$(SPHINXBUILD) -M clean "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
	@$(SPHINXBUILD) -M clean "$(SOURCEDIR)" "$(BUILDDIR_INTERNAL)" $(SPHINXOPTS) $(O)

clean-doxygen:
	@rm -rf doxygen doxygen-internal

clean-all: clean clean-doxygen

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
