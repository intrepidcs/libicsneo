===============
Python Examples
===============

Transmit CAN frames on HSCAN
============================

.. code-block:: python

   import icsneopy

   devices: list[icsneopy.Device] = icsneopy.find_all_devices()

   # grab the first/only device found
   device: icsneopy.Device = devices[0]

   message = icsneopy.CANMessage()
   message.network = icsneopy.Network(icsneopy.Network.NetID.HSCAN)
   message.arbid = 0x56
   message.data = (0x11, 0x22, 0x33)

   device.open()

   device.go_online()

   device.transmit(message)
     
Receive CAN frames on HSCAN
===========================

.. code-block:: python

   import icsneopy
   import time

   devices: list[icsneopy.Device] = icsneopy.find_all_devices()

   # grab the first/only device found
   device: icsneopy.Device = devices[0]

   def on_message(message: icsneopy.CANMessage):
       print(message.arbid, message.data)
   message_filter = icsneopy.MessageFilter(icsneopy.Network.NetID.HSCAN)
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
