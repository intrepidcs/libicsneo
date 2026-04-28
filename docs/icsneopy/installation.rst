============
Installation
============

icsneopy is available on PyPI at https://pypi.org/project/icsneopy/ and can be installed with pip:

.. code-block:: bash

   pip install icsneopy

Pre-release
===========

For the latest features, install with the ``--pre`` flag to include pre-release versions:

.. code-block:: bash

   pip install --pre icsneopy

Upgrading
=========

To upgrade an existing installation:

.. code-block:: bash

   pip install --pre --upgrade icsneopy

Linux udev Rules
================

Linux users may want to install the included udev rules to run icsneopy-based
applications without root. The rules file can be found in the libicsneo source
repository at https://github.com/intrepidcs/libicsneo/.

.. code-block:: bash

   sudo cp 99-intrepidcs.rules /etc/udev/rules.d/
