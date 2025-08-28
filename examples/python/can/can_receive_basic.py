"""
Basic CAN frame reception example using icsneopy library.

Demonstrates how to receive CAN frames on DW CAN 01 using callback handlers.
"""

import icsneopy
import time


def receive_can_frames():
	"""Receive CAN frames with callback handling."""
	devices = icsneopy.find_all_devices()
	if not devices:
		raise RuntimeError("No devices found")

	device = devices[0]
	frame_count = 0

	def on_frame(frame):
		nonlocal frame_count
		frame_count += 1
		print(f"[RX {frame_count}] ID: 0x{frame.arbid:03X}, "
			  f"Data: {[hex(b) for b in frame.data]}")

	frame_filter = icsneopy.MessageFilter(icsneopy.Network.NetID.DWCAN_01)
	callback = icsneopy.MessageCallback(on_frame, frame_filter)

	try:
		if not device.open():
			raise RuntimeError("Failed to open device")

		if not device.go_online():
			raise RuntimeError("Failed to go online")

		device.add_message_callback(callback)
		print("Listening for CAN frames for 10 seconds...")
		time.sleep(10)

		print(f"Total frames received: {frame_count}")

	finally:
		device.close()


if __name__ == "__main__":
	receive_can_frames()
