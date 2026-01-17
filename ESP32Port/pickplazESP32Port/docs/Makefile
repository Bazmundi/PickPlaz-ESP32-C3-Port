# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = build
DOXYGEN       ?= doxygen
DOXYFILE      ?= Doxyfile
DOXYFILE_INTERNAL ?= Doxyfile.internal
INTERNAL_BUILDDIR ?= build-internal
INTERNAL_EXHALE_DIR ?= ./api/exhale-internal
INTERNAL_BREATHE_XML ?= $(CURDIR)/doxygen-internal/xml

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile doxygen doxygen-internal sphinx sphinx-internal all all-internal

doxygen:
	@echo "Running Doxygen (public)..."
	$(DOXYGEN) $(DOXYFILE)

doxygen-internal:
	@echo "Running Doxygen (internal)..."
	$(DOXYGEN) $(DOXYFILE_INTERNAL)

sphinx:
	@echo "Building Sphinx (public)..."
	@$(SPHINXBUILD) -M html "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

sphinx-internal:
	@echo "Building Sphinx (internal)..."
	BREATHE_XML_DIR=$(INTERNAL_BREATHE_XML) \
	EXHALE_CONTAINMENT_FOLDER=$(INTERNAL_EXHALE_DIR) \
	$(SPHINXBUILD) -M html "$(SOURCEDIR)" "$(INTERNAL_BUILDDIR)" $(SPHINXOPTS) $(O)

all: doxygen sphinx

all-internal: doxygen-internal sphinx-internal

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
