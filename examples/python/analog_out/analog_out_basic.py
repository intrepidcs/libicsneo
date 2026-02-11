"""
Basic analog output control example using icsneopy library.

Demonstrates how to configure and control analog outputs on supported devices.

Usage: python analog_out_basic.py <pin> <voltage> [--yes]

Arguments:
  pin: Pin number (1-3 for RAD Galaxy)
  voltage: Voltage level (0-5)
  --yes: Skip confirmation prompt
"""

import sys
import icsneopy


def analog_output_example(pin: int, voltage: int, skip_confirm: bool = False):
	"""Configure and control analog outputs."""
	# Confirmation prompt
	if not skip_confirm:
		print(f"WARNING: This will set analog output pin {pin} to {voltage}V")
		print("Make sure nothing sensitive is connected to this pin.")
		response = input("Continue? (yes/no): ")
		if response.lower() != "yes":
			print("Aborted.")
			return
	
	devices = icsneopy.find_all_devices()
	if not devices:
		raise RuntimeError("No devices found")

	device = devices[0]

	try:
		if not device.open():
			raise RuntimeError("Failed to open device")

		settings = device.settings
		if not settings:
			raise RuntimeError("Device settings not available")

		print("Refreshing device settings...")
		if not settings.refresh():
			raise RuntimeError("Failed to refresh settings")

		# Enable analog output on specified pin
		print(f"Enabling analog output on pin {pin}...")
		if not settings.set_misc_io_analog_output_enabled(pin, True):
			raise RuntimeError(f"Failed to enable analog output on pin {pin}")

		# Map voltage level to enum
		voltage_map = {
			0: icsneopy.Settings.MiscIOAnalogVoltage.V0,
			1: icsneopy.Settings.MiscIOAnalogVoltage.V1,
			2: icsneopy.Settings.MiscIOAnalogVoltage.V2,
			3: icsneopy.Settings.MiscIOAnalogVoltage.V3,
			4: icsneopy.Settings.MiscIOAnalogVoltage.V4,
			5: icsneopy.Settings.MiscIOAnalogVoltage.V5
		}
		voltage_enum = voltage_map[voltage]

		# Set pin to specified voltage
		print(f"Setting pin {pin} to {voltage}V...")
		if not settings.set_misc_io_analog_output(pin, voltage_enum):
			raise RuntimeError(f"Failed to set voltage on pin {pin}")

		# Apply settings
		print("Applying settings...")
		if not settings.apply():
			raise RuntimeError("Failed to apply settings")

		print("Analog output configured successfully!")
		print(f"Pin {pin}: Enabled at {voltage}V")

	finally:
		device.close()


if __name__ == "__main__":
	if len(sys.argv) < 3:
		print("Error: Missing required arguments\n")
		print("Usage: python analog_out_basic.py <pin> <voltage> [--yes]")
		print("\nArguments:")
		print("  pin: Pin number (1-3 for RAD Galaxy)")
		print("  voltage: Voltage level (0-5)")
		print("  --yes: Skip confirmation prompt")
		sys.exit(1)
	
	try:
		pin = int(sys.argv[1])
		if pin < 1 or pin > 3:
			print("Error: Invalid pin number (must be 1-3)")
			sys.exit(1)
		
		voltage = int(sys.argv[2])
		if voltage < 0 or voltage > 5:
			print("Error: Invalid voltage level (must be 0-5)")
			sys.exit(1)
		
		skip_confirm = "--yes" in sys.argv
		
		analog_output_example(pin, voltage, skip_confirm)
		
	except ValueError:
		print("Error: Pin and voltage must be integers")
		sys.exit(1)
