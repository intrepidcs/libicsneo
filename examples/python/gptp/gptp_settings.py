import icsneopy

GPTP_PORT_MAP = {
	icsneopy.Network.NetID.AE_01:       (1,  "AE 01"),
	icsneopy.Network.NetID.AE_02:       (2,  "AE 02"),
	icsneopy.Network.NetID.AE_03:       (3,  "AE 03"),
	icsneopy.Network.NetID.AE_04:       (4,  "AE 04"),
	icsneopy.Network.NetID.AE_05:       (5,  "AE 05"),
	icsneopy.Network.NetID.AE_06:       (6,  "AE 06"),
	icsneopy.Network.NetID.AE_07:       (7,  "AE 07"),
	icsneopy.Network.NetID.AE_08:       (8,  "AE 08"),
	icsneopy.Network.NetID.AE_09:       (9,  "AE 09"),
	icsneopy.Network.NetID.AE_10:       (10, "AE 10"),
	icsneopy.Network.NetID.AE_11:       (11, "AE 11"),
	icsneopy.Network.NetID.AE_12:       (12, "AE 12"),
	icsneopy.Network.NetID.ETHERNET_01: (13, "Ethernet 01"),
	icsneopy.Network.NetID.ETHERNET_02: (14, "Ethernet 02"),
	icsneopy.Network.NetID.ETHERNET_03: (15, "Ethernet 03"),
	icsneopy.Network.NetID.AE_13:       (16, "AE 13"),
	icsneopy.Network.NetID.AE_14:       (17, "AE 14"),
	icsneopy.Network.NetID.AE_15:       (18, "AE 15"),
	icsneopy.Network.NetID.AE_16:       (19, "AE 16"),
}

PROFILE_NAMES = {
	icsneopy.Settings.GTPPProfile.Standard:   "Standard",
	icsneopy.Settings.GTPPProfile.Automotive: "Automotive",
}

ROLE_NAMES = {
	icsneopy.Settings.GTPPRole.Disabled: "Disabled",
	icsneopy.Settings.GTPPRole.Passive:  "Passive",
	icsneopy.Settings.GTPPRole.Master:   "Master",
	icsneopy.Settings.GTPPRole.Slave:    "Slave",
}


def prompt_int(prompt, default, lo, hi):
	try:
		raw = input(f"{prompt} [{default}]: ").strip()
		if not raw:
			return default
		val = int(raw)
		if lo <= val <= hi:
			return val
	except (ValueError, EOFError):
		pass
	print(f"Invalid input, using {default}.")
	return default


def gptp_ports(device):
	ports = []
	for net in device.get_supported_tx_networks():
		entry = GPTP_PORT_MAP.get(net.get_net_id())
		if entry:
			ports.append(entry)
	return sorted(ports)


def print_settings(device):
	s = device.settings
	profile  = s.get_gptp_profile()
	role     = s.get_gptp_role()
	port     = s.get_gptp_enabled_port()
	synton   = s.is_gptp_clock_syntonization_enabled()

	print("\nCurrent gPTP settings:")
	if profile is not None:
		print(f"  Profile:             {PROFILE_NAMES.get(profile, profile)}")
	if role is not None:
		print(f"  Role:                {ROLE_NAMES.get(role, role)}")
	if port is not None:
		suffix = " (disabled)" if port == 0 else ""
		print(f"  Enabled port:        {port}{suffix}")
	if synton is not None:
		print(f"  Clock syntonization: {'enabled' if synton else 'disabled'}")


def configure(device):
	if not device.settings.refresh():
		print("error: failed to refresh settings")
		return False

	print_settings(device)

	ports = gptp_ports(device)
	print("\nAvailable gPTP ports (0 = disabled):")
	print("    [0] Disabled")
	for idx, name in ports:
		print(f"    [{idx}] {name}")

	print("\nConfigure gPTP:")
	profile   = prompt_int("  Profile (0=Standard, 1=Automotive)", 1, 0, 1)
	role      = prompt_int("  Role (0=Disabled, 1=Passive, 2=Master, 3=Slave)", 3, 0, 3)
	port      = prompt_int("  Enabled port index", 0, 0, 19)
	synton    = prompt_int("  Clock syntonization (0=disabled, 1=enabled)", 0, 0, 1)
	permanent = prompt_int("\nSave to EEPROM? (0=temporary, 1=permanent)", 0, 0, 1)

	s = device.settings
	profile_enum = list(PROFILE_NAMES.keys())[profile]
	role_enum    = list(ROLE_NAMES.keys())[role]

	s.set_gptp_profile(profile_enum)
	s.set_gptp_role(role_enum)
	s.set_gptp_enabled_port(port)
	s.set_gptp_clock_syntonization_enabled(bool(synton))

	if not s.apply(not permanent):
		print("error: failed to apply settings")
		return False

	print_settings(device)
	return True


def main():
	devices = icsneopy.find_all_devices()
	if not devices:
		print("error: no devices found")
		return 1

	gptp_devices = [d for d in devices if d.settings.get_gptp_profile() is not None]
	if not gptp_devices:
		print("error: no gPTP-capable devices found")
		return 1

	print("Available gPTP-capable devices:")
	for i, d in enumerate(gptp_devices, 1):
		print(f"  [{i}] {d}")

	choice = prompt_int("Select device", 1, 1, len(gptp_devices))
	device = gptp_devices[choice - 1]

	print(f"\nOpening {device}... ", end="", flush=True)
	if not device.open():
		print("failed")
		return 1
	print("OK")

	try:
		ok = configure(device)
	finally:
		print(f"\nClosing {device}")
		device.close()

	return 0 if ok else 1


if __name__ == "__main__":
	raise SystemExit(main())
