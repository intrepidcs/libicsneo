===============
Python Examples
===============

Transmit CAN frames on DW CAN 01
============================

.. code-block:: python

   import icsneopy

   devices: list[icsneopy.Device] = icsneopy.find_all_devices()

   # grab the first/only device found
   device: icsneopy.Device = devices[0]

   message = icsneopy.CANMessage()
   message.network = icsneopy.Network(icsneopy.Network.NetID.DWCAN_01)
   message.arbid = 0x56
   message.data = (0x11, 0x22, 0x33)

   device.open()

   device.go_online()

   device.transmit(message)
     
Receive CAN frames on DW CAN 01
===========================

.. code-block:: python

   import icsneopy
   import time

   devices: list[icsneopy.Device] = icsneopy.find_all_devices()

   # grab the first/only device found
   device: icsneopy.Device = devices[0]

   def on_message(message: icsneopy.CANMessage):
       print(message.arbid, message.data)
   message_filter = icsneopy.MessageFilter(icsneopy.Network.NetID.DWCAN_01)
   callback = icsneopy.MessageCallback(on_message, message_filter)

   device.add_message_callback(callback)

   device.open()
   device.go_online()

   # rx for 10s
   time.sleep(10)


Monitor Ethernet Status
=======================

.. code-block:: python

   import icsneopy
   import time

   def main():
      devices = icsneopy.find_all_devices()
      if len(devices) == 0:
         print("error: no devices found")
         return False

      device = devices[0]
      print(f"info: monitoring Ethernet status on {device}")

      def on_message(message):
         print(f"info: network: {message.network}, state: {message.state}, speed: {message.speed}, duplex: {message.duplex}, mode: {message.mode}")

      filter = icsneopy.MessageFilter(icsneopy.Message.Type.EthernetStatus)
      callback = icsneopy.MessageCallback(on_message, filter)
      device.add_message_callback(callback)

      if not device.open():
         print("error: unable to open device")
         return False

      if not device.go_online():
         print("error: unable to go online")
         return False

      while True:
         time.sleep(1)

   main()

TC10
====

:download:`Download example <../../examples/python/tc10/tc10.py>`

.. literalinclude:: ../../examples/python/tc10/tc10.py

DoIP Ethernet Activation
========================

.. code-block:: python

   import icsneopy
   import time

   devs = icsneopy.find_all_devices()

   dev = devs[0]

   dev.open()

   # the device must be online for digital I/O
   dev.go_online()

   print(f"initial state: {dev.get_digital_io(icsneopy.IO.EthernetActivation, 1)}")

   dev.set_digital_io(icsneopy.IO.EthernetActivation, 1, True)

   print(f"after setting to true: {dev.get_digital_io(icsneopy.IO.EthernetActivation, 1)}")

   # allow for observing the change
   time.sleep(2)

   dev.set_digital_io(icsneopy.IO.EthernetActivation, 1, False)

   print(f"after setting to false: {dev.get_digital_io(icsneopy.IO.EthernetActivation, 1)}")

   # allow for observing the change
   time.sleep(2)
