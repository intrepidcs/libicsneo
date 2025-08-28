"""
Complete CAN example using icsneopy library.

Demonstrates device setup and CAN frame transmission/reception.
"""

import icsneopy
import time


def setup_device():
	"""Initialize CAN device."""
	devices = icsneopy.find_all_devices()
	if not devices:
		raise RuntimeError("No devices found")

	device = devices[0]
	print(f"Using device: {device}")
	return device


def open_device(device):
	"""Open device connection."""
	try:
		if not device.open():
			raise RuntimeError("Failed to open device")

		if not device.go_online():
			device.close()
			raise RuntimeError("Failed to go online")

		print("Device initialized successfully")
		return True

	except Exception as e:
		print(f"Device setup failed: {e}")
		return False


def transmit_can_frame(device, arbid, data):
	"""Transmit a CAN frame."""
	frame = icsneopy.CANMessage()
	frame.network = icsneopy.Network(icsneopy.Network.NetID.DWCAN_01)
	frame.arbid = arbid
	frame.data = data

	success = device.transmit(frame)
	if success:
		print(f"Frame transmitted: ID=0x{arbid:03X}, Data={list(data)}")
	else:
		print(f"Failed to transmit frame ID=0x{arbid:03X}")

	return success


def setup_can_reception(device):
	"""Configure CAN frame reception with callback."""
	frame_count = 0

	def frame_handler(frame):
		nonlocal frame_count
		frame_count += 1
		print(f"[RX {frame_count}] ID: 0x{frame.arbid:03X}, "
			  f"Data: {[hex(b) for b in frame.data]}, "
			  f"Length: {len(frame.data)}")

	frame_filter = icsneopy.MessageFilter(icsneopy.Network.NetID.DWCAN_01)
	callback = icsneopy.MessageCallback(frame_handler, frame_filter)
	device.add_message_callback(callback)

	print("CAN frame reception configured")
	return 0


def cleanup_device(device):
	"""Close device connection."""
	if device:
		device.close()
		print("Device connection closed")


def main():
	"""Complete CAN example with proper error handling."""
	device = None

	try:
		# Setup device
		device = setup_device()

		# Open device
		if not open_device(device):
			raise RuntimeError("Failed to initialize device")

		# Setup frame reception
		setup_can_reception(device)

		# Transmit test frames
		test_frames = [
			(0x123, (0x01, 0x02, 0x03, 0x04)),
			(0x456, (0x05, 0x06, 0x07, 0x08)),
			(0x789, (0x09, 0x0A, 0x0B, 0x0C))
		]

		for arbid, data in test_frames:
			transmit_result = transmit_can_frame(device, arbid, data)
			if not transmit_result:
				print(f"Warning: Failed to transmit frame ID=0x{arbid:03X}")
			time.sleep(0.1)

		# Listen for responses
		print("Listening for CAN frames for 5 seconds...")
		time.sleep(5)

	except Exception as e:
		print(f"Error: {e}")
		return 1

	finally:
		cleanup_device(device)

	return 0


if __name__ == "__main__":
	main()
