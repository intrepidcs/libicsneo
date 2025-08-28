=========================
Ethernet Getting Started
=========================

This guide provides basic instructions for working with Ethernet devices using the icsneopy library.

Prerequisites
=============

- Python 3.8 or higher
- icsneopy library installed
- Intrepid Control Systems Device

:download:`Download complete example <../../examples/python/ethernet/ethernet_complete_example.py>`

Opening the Device
==================

To begin working with an Ethernet device, you must first discover, open, and bring it online:

.. literalinclude:: ../../examples/python/ethernet/ethernet_complete_example.py
   :language: python
   :lines: 11-37

Transmitting Ethernet Frames
=============================

Transmit Ethernet frames using the EthernetMessage class:

.. literalinclude:: ../../examples/python/ethernet/ethernet_complete_example.py
   :language: python
   :lines: 61-78

Monitoring Ethernet Status
==========================

Monitor Ethernet link status changes using message callbacks:

.. literalinclude:: ../../examples/python/ethernet/ethernet_complete_example.py
   :language: python
   :lines: 39-40

DoIP Ethernet Activation Control
================================

Diagnostics over Internet Protocol (DoIP) can be controlled through digital I/O pins:

:download:`Download DoIP example <../../examples/python/doip/doip_activation_control.py>`

.. literalinclude:: ../../examples/python/doip/doip_activation_control.py
   :language: python
   :lines: 28-44

Network Configuration Reference
===============================

Standard Ethernet network identifiers:

- ``icsneopy.Network.NetID.ETHERNET_01`` - Standard Ethernet Port 1

Automotive Ethernet network identifiers:

- ``icsneopy.Network.NetID.AE_01`` - Automotive Ethernet Port 1

Complete Example with Resource Management
=========================================

.. literalinclude:: ../../examples/python/ethernet/ethernet_complete_example.py
   :language: python
   :lines: 86-127
