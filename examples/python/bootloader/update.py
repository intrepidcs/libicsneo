import sys

import icsneopy

def update(firmware_path):
	devices = icsneopy.find_all_devices()

	if len(devices) == 0:
		print("no devices found")
		return False

	device = devices[0]
	print(f"selected {device}")

	if not device.open():
		print("unable to open device")
		return False

	fw = icsneopy.Bootloader.FWLoader.load(firmware_path)
	if fw is None:
		print(f"unable to load firmware from {firmware_path}")
		return False

	last_status = None

	# repeated status messages overwrite the current line so progress updates in place
	def logger(status_type, status, progress):
		nonlocal last_status
		if last_status not in (None, status):
			print()
		last_status = status
		print(f"\r{status}" + (f" ({progress * 100:.1f}%)" if progress is not None else ""), end="", flush=True)
		return icsneopy.Device.OpenDirective.Continue

	# force_flash updates every chip, even those already at the version in the firmware file
	ok = icsneopy.Bootloader.update(device, fw, logger, force_flash=True)
	print()
	if not ok:
		print(f"update failed: {icsneopy.get_last_error()}")
		return False

	print("update complete")
	return True

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print(f"usage: {sys.argv[0]} <fw.bin|bundle>")
		sys.exit(1)
	sys.exit(0 if update(sys.argv[1]) else 1)
