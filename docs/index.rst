=========
libicsneo
=========

libicsneo is the `Intrepid Control Systems <https://intrepidcs.com/>`_ device
communication library. The source code for libicsneo can be found on GitHub: 
`https://github.com/intrepidcs/libicsneo <https://github.com/intrepidcs/libicsneo>`_

.. toctree::
   :maxdepth: 1
   :caption: Documentation

   icsneocpp/index
   icsneopy/index
   icsneoc/index
   icsneoc2/index

Linux Installation
==================

Applications that use raw Ethernet device discovery, such as PCAP-backed
discovery, need permission to open raw network sockets. Instead of running your
application with ``sudo``, grant the installed executable the required Linux
capabilities:

.. code-block:: bash

   sudo setcap cap_net_raw,cap_net_admin=eip /usr/bin/your-app

Replace ``/usr/bin/your-app`` with the full path to the application executable.

