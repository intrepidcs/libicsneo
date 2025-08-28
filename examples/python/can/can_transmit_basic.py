"""
Basic CAN frame transmission example using icsneopy library.

Demonstrates how to transmit CAN frames on DW CAN 01.
"""

import icsneopy


def transmit_can_frame():
	"""Transmit a CAN frame."""
	devices = icsneopy.find_all_devices()
	if not devices:
		raise RuntimeError("No devices found")

	device = devices[0]

	try:
		if not device.open():
			raise RuntimeError("Failed to open device")

		if not device.go_online():
			raise RuntimeError("Failed to go online")

		frame = icsneopy.CANMessage()
		frame.network = icsneopy.Network(icsneopy.Network.NetID.DWCAN_01)
		frame.arbid = 0x123
		frame.data = (0x01, 0x02, 0x03, 0x04)

		success = device.transmit(frame)
		if success:
			print(f"Frame transmitted: ID=0x{frame.arbid:03X}")
		else:
			print("Failed to transmit frame")

	finally:
		device.close()


if __name__ == "__main__":
	transmit_can_frame()
