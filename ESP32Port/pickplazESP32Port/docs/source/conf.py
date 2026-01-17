# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'PickPlaz ESP32-C3 Port'
copyright = '2026, Asterion Daedalus'
author = 'Asterion Daedalus'
release = '0.0.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['myst_parser', 'breathe', 'exhale']
source_suffix = {'.md': 'markdown', '.rst': 'restructuredtext'}

templates_path = ['_templates']
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'alabaster'
html_static_path = ['_static']
html_css_files = ['custom.css']

# -- Breathe + Exhale configuration -----------------------------------------

_here = os.path.abspath(os.path.dirname(__file__))

_breathe_xml_dir = os.environ.get(
    'BREATHE_XML_DIR',
    os.path.abspath(os.path.join(_here, '..', 'doxygen', 'xml')),
)
breathe_projects = {
    'pickplaz': _breathe_xml_dir,
}
breathe_default_project = 'pickplaz'

_exhale_containment = os.environ.get('EXHALE_CONTAINMENT_FOLDER', './api/exhale')
exhale_args = {
    'containmentFolder': _exhale_containment,
    'rootFileName': 'library_root.rst',
    'rootFileTitle': 'API Reference',
    'doxygenStripFromPath': os.path.abspath(os.path.join(_here, '..', '..')),
    'createTreeView': True,
    'listingExclude': [
        r'.*_8c',
        r'.*\\.c$',
        r'.*/src/.*',
        r'^dir_docs.*',
        r'^file_docs.*',
        r'.*porting__design_8md.*',
        r'.*porting_design\\.md.*',
    ],
}
