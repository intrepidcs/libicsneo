"""
Basic Ethernet frame transmission example using icsneopy library.

Demonstrates how to transmit Ethernet frames on Ethernet 01.
"""

import icsneopy


def transmit_ethernet_frame():
	"""Transmit an Ethernet frame."""
	devices = icsneopy.find_all_devices()
	if not devices:
		raise RuntimeError("No devices found")

	device = devices[0]

	try:
		if not device.open():
			raise RuntimeError("Failed to open device")

		if not device.go_online():
			raise RuntimeError("Failed to go online")

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

	finally:
		device.close()


if __name__ == "__main__":
	transmit_ethernet_frame()
