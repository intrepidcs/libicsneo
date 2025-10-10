import icsneopy

def chip_versions():
	devices = icsneopy.find_all_devices()

	if len(devices) == 0:
		print("no devices found")
		return False

	device = devices[0]
	print(f"selected {device}")

	if not device.open():
		print("unable to open device")
		return False

	chip_versions = device.get_chip_versions()
	if not chip_versions:
		print("no chip versions")
		return False

	print("chip versions:")
	for i in chip_versions:
		version = f"{i.major}.{i.minor}.{i.maintenance}.{i.build}"
		print(f"    id: {i.id}, name: {i.name}, version: {version}")

	return True

if __name__ == "__main__":
	chip_versions()
