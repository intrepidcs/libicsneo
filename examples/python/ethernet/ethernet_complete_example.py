"""
Complete Ethernet example using icsneopy library.

Demonstrates device setup and Ethernet frame transmission/reception.
"""

import icsneopy
import time


def setup_device():
	"""Initialize Ethernet device."""
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

def on_status_message(message):
	print(f"info: network: {message.network}, state: {message.state}, speed: {message.speed}, duplex: {message.duplex}, mode: {message.mode}")
	

def setup_ethernet_reception(device):
	"""Configure Ethernet frame reception with callback."""
	frame_count = 0

	def frame_handler(frame):
		nonlocal frame_count
		frame_count += 1
		print(f"[RX {frame_count}], "
			  f"Data: {[hex(b) for b in frame.data]}, "
			  f"Length: {len(frame.data)}")
	frame_filter = icsneopy.MessageFilter(icsneopy.Network.NetID.ETHERNET_01)
	callback = icsneopy.MessageCallback(frame_handler, frame_filter)
	device.add_message_callback(callback)

	print("CAN frame reception configured")
	return 0


def transmit_ethernet_frame(device):
	"""Transmit an Ethernet frame."""
	frame = icsneopy.EthernetMessage()
	frame.network = icsneopy.Network(icsneopy.Network.NetID.ETHERNET_01)
	frame.data = [
		0x00, 0xFC, 0x70, 0x00, 0x01, 0x02,
		0x00, 0xFC, 0x70, 0x00, 0x01, 0x01,
		0x08, 0x00,
		0x01, 0xC5, 0x01, 0xC5
	]

	success = device.transmit(frame)
	if success:
		print("Frame transmitted successfully")
	else:
		print("Failed to transmit frame")

	return success

def cleanup_device(device):
	"""Close device connection."""
	if device:
		device.close()
		print("Device connection closed")

def main():
	"""Complete Ethernet example"""
	device = None

	try:
		# Initialize device
		device = setup_device()

		# Open device
		if not open_device(device):
			raise RuntimeError("Failed to initialize device")
		
		filter = icsneopy.MessageFilter(icsneopy.Message.Type.EthernetStatus)
		status_callback = icsneopy.MessageCallback(on_status_message, filter)
		device.add_message_callback(status_callback)

		#Setup Ethernet Callback
		setup_ethernet_reception(device)

		# Transmit an Ethernet frame
		transmit_result = transmit_ethernet_frame(device)
		if not transmit_result:
			print("Warning: Failed to transmit frame")

		# Monitor for a period
		print("Monitoring for 10 seconds...")
		time.sleep(10)

		print(f"Monitoring completed.")

	except Exception as e:
		print(f"Error: {e}")
		return 1

	finally:
		cleanup_device(device)

	return 0


if __name__ == "__main__":
	main()
