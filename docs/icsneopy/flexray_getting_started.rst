=======================
FlexRay Getting Started
=======================

Prerequisites
=============

- icsneopy library installed
- FlexRay hardware device connected (e.g., Fire3 Flexray)
- Proper FlexRay bus termination (100Ω on each channel end)

Physical Hardware Setup for Two-Node Testing
---------------------------------------------

For testing the basic transmit and receive examples with a single device:

- Hardware: Device with dual FlexRay controllers (e.g., neoVI FIRE 3 Flexray)
- Connection: FLEXRAY_01 Channel A looped to FLEXRAY_02 Channel A
- Termination: 100Ω termination resistors on both ends of the loopback
- Cable: Use proper FlexRay twisted pair cable (impedance matched)

.. note::
   The basic transmit/receive examples are configured for this loopback setup
   where both controllers act as coldstart nodes. For use on an existing 
   FlexRay network, see the passive monitoring configuration notes in the
   receive example.

FlexRay Coldstart
-----------------

FlexRay networks require at least one "coldstart node" to initialize the network timing. 
The coldstart node is responsible for starting the FlexRay communication cycle.

For a complete standalone coldstart example, see the Additional Examples section below.

Basic Setup
===========

1. Import the library and find FlexRay device:

.. code-block:: python

   import icsneopy

   devices = icsneopy.find_all_devices()
   
   # Find a device with FlexRay support
   device = None
   for dev in devices:
       if dev.get_extension("FlexRay"):
           device = dev
           break
   
   if not device:
       raise RuntimeError("No FlexRay-capable device found")

2. Configure FlexRay controller:

.. literalinclude:: ../../examples/python/flexray/flexray_transmit_basic.py
   :language: python
   :lines: 12-111

3. Open device and go online:

.. code-block:: python

   if not device.open():
       raise RuntimeError("Failed to open device")

   if not device.go_online():
       raise RuntimeError("Failed to go online")

Transmitting FlexRay Frames
============================

This example demonstrates a coldstart node that initiates a FlexRay network
and transmits simulated sensor data continuously in slot 1.

**Hardware Setup**: FLEXRAY_01 looped to FLEXRAY_02

**Usage**:
1. Start the receive example first
2. Start this transmit example second
3. Network will initialize and frames will be transmitted

.. literalinclude:: ../../examples/python/flexray/flexray_transmit_basic.py
   :language: python
   :lines: 113-170

Key Configuration Parameters:

- **slotid**: The FlexRay slot ID for transmission (1-2047 for static segment)
- **cycle**: The FlexRay cycle number (0-63)
- **cycle_repetition**: How often the frame repeats (1 = every cycle, 2 = every other cycle)
- **channel**: Transmission channel (A, B, or AB for both)
- **key_slot_id**: Must be unique per node on the network
- **key_slot_used_for_startup**: True for coldstart nodes
- **key_slot_used_for_sync**: True to provide synchronization frames

Receiving FlexRay Frames
=========================

This example demonstrates receiving FlexRay frames on FLEXRAY_02 Channel A.

**Hardware Setup**: FLEXRAY_01 looped to FLEXRAY_02

**Configuration Note**: This example is configured with coldstart capability
for two-node loopback testing. For passive monitoring on an existing FlexRay
network:

1. Set ``key_slot_used_for_startup = False`` in the controller configuration
2. Remove the ``controller.set_allow_coldstart(True)`` call
3. Ensure all cluster parameters match the existing network
4. The node will sync and receive without transmitting

**Usage**:
1. Start this receive example first
2. Start the transmit example second
3. Frames from slot 1 will be displayed with hex and decimal payload views

.. literalinclude:: ../../examples/python/flexray/flexray_receive_basic.py
   :language: python
   :lines: 103-170

FlexRay Coldstart Configuration
================================

To use the Coldstart example, ensure the following:

Set the Flexray network in neoVI Explorer to Coldstart.

No other nodes should be present on the network during testing.

Nothing connected to Fire3 FlexRay bus.

Critical Coldstart Settings
----------------------------

.. literalinclude:: ../../examples/python/flexray/flexray_coldstart.py
   :language: python
   :lines: 40-48

Configuration Example:

.. literalinclude:: ../../examples/python/flexray/flexray_coldstart.py
   :language: python
   :lines: 20-64

Setting Coldstart on Controller:

.. code-block:: python

   controller.set_allow_coldstart(True)
   controller.set_start_when_going_online(True)

Cleanup and Resource Management
================================

Always close the device when finished:

.. code-block:: python

   try:
       # Your FlexRay operations here
       pass
   finally:
       device.close()

See the basic transmit and receive examples for complete implementations.

Additional Examples
===================

Transmit Basic
--------------

Complete working example with coldstart node transmitting simulated sensor data.

All example files are available for download:

**Transmit Basic** - Coldstart node transmitting simulated sensor data

:download:`flexray_transmit_basic.py <../../examples/python/flexray/flexray_transmit_basic.py>`

**Receive Basic** - Receiving and displaying FlexRay frames with formatted output

:download:`flexray_receive_basic.py <../../examples/python/flexray/flexray_receive_basic.py>`

**Coldstart** - Standalone coldstart example demonstrating network initialization

:download:`flexray_coldstart.py <../../examples/python/flexray/flexray_coldstart.py>`