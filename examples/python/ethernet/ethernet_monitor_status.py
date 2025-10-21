"""
Ethernet status monitoring example using icsneopy library.

Demonstrates how to monitor Ethernet link status changes.
"""

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
      print(f"info: timestamp: {message.timestamp}, network: {message.network}, state: {message.state}, speed: {message.speed}, duplex: {message.duplex}, mode: {message.mode}")

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