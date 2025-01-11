#!/bin/sh

python3 -m venv env || exit 1
. env/bin/activate || exit 1
python3 -m pip install cibuildwheel || exit 1
python3 -m cibuildwheel --output-dir wheelhouse || exit 1

