"""
DoIP activation control example using icsneopy library.

Demonstrates DoIP (Diagnostics over Internet Protocol) Ethernet activation
control with comprehensive error handling and state validation.
"""

import icsneopy
import time


def doip_activation_demo():
	"""Demonstrate DoIP Ethernet activation control with error handling."""
	devices = icsneopy.find_all_devices()
	if not devices:
		raise RuntimeError("No devices found")

	device = devices[0]
	print(f"Using {device} for DoIP activation control")

	try:
		if not device.open():
			raise RuntimeError("Failed to open device")

		if not device.go_online():
			raise RuntimeError("Failed to go online")

		initial_state = device.get_digital_io(icsneopy.IO.EthernetActivation, 1)
		print(f"Initial DoIP activation state: {initial_state}")

		print("Activating DoIP Ethernet...")
		device.set_digital_io(icsneopy.IO.EthernetActivation, 1, True)
		time.sleep(1)

		active_state = device.get_digital_io(icsneopy.IO.EthernetActivation, 1)
		print(f"DoIP activated: {active_state}")
		time.sleep(2)

		print("Deactivating DoIP Ethernet...")
		device.set_digital_io(icsneopy.IO.EthernetActivation, 1, False)
		time.sleep(1)

		final_state = device.get_digital_io(icsneopy.IO.EthernetActivation, 1)
		print(f"DoIP deactivated: {final_state}")

	except Exception as e:
		print(f"DoIP control error: {e}")
	finally:
		device.close()


if __name__ == "__main__":
	doip_activation_demo()
