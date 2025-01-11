# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import subprocess

subprocess.call('cd ..; doxygen docs/icsneocpp/Doxyfile', shell=True)
subprocess.call('cd ..; doxygen docs/icsneoc/Doxyfile', shell=True)

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'libicsneo'
copyright = '2024, Intrepid Control Systems, Inc.'
author = 'Intrepid Control Systems, Inc.'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['breathe', 'sphinx.ext.autodoc']

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

breathe_projects = {
    'icsneocpp': 'icsneocpp/doxygen/xml',
    'icsneoc': 'icsneoc/doxygen/xml',
}

breathe_default_project = 'icsneocpp'

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
