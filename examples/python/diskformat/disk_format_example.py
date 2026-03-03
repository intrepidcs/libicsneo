import sys
import icsneopy

def disk_format_example():
    devices = icsneopy.find_all_devices()
    if not devices:
        print("error: no devices found")
        return False

    print(f"Found {len(devices)} device(s):")
    for i, d in enumerate(devices):
        print(f"  [{i}] {d}")

    if len(devices) == 1:
        choice = 0
    else:
        try:
            choice = int(input(f"Select a device [0-{len(devices)-1}]: "))
        except (ValueError, EOFError):
            print("error: invalid selection")
            return False
        if choice < 0 or choice >= len(devices):
            print("error: invalid selection")
            return False

    device = devices[choice]
    print(f"\nOpening {device}... ", end="", flush=True)
    if not device.open():
        print("FAIL")
        print(f"error: {icsneopy.get_last_error().describe()}")
        return False
    print("OK")

    if not device.supports_disk_formatting():
        print(f"error: {device} does not support disk formatting")
        device.close()
        return False

    print(f"Disk count: {device.get_disk_count()}")

    # Query current disk state
    print("\nQuerying disk details... ", end="", flush=True)
    details = device.get_disk_details()
    if details is None:
        print("FAIL")
        print(f"error: {icsneopy.get_last_error().describe()}")
        device.close()
        return False
    print("OK")

    layout_name = "RAID0" if details.layout == icsneopy.DiskLayout.RAID0 else "Spanned"
    print(f"  Layout        : {layout_name}")
    for i, disk in enumerate(details.disks):
        print(f"  Disk [{i}]:")
        print(f"    Present     : {'yes' if disk.present else 'no'}")
        print(f"    Initialized : {'yes' if disk.initialized else 'no'}")
        print(f"    Formatted   : {'yes' if disk.formatted else 'no'}")
        if disk.present:
            mb = disk.size() // (1024 * 1024)
            print(f"    Size        : {mb} MB ({disk.sectors} sectors x {disk.bytes_per_sector} bytes)")

    any_present = any(d.present for d in details.disks)
    if not any_present:
        print("\nerror: no disks are present in the device")
        device.close()
        return False

    # Build format config from the queried state
    fmt = icsneopy.DiskDetails()
    fmt.layout = details.layout
    fmt.full_format = False  # Quick format; set True for a full (slow) format
    fmt.disks = details.disks
    for disk in fmt.disks:
        if disk.present:
            disk.formatted = True  # mark for formatting

    confirm = input(
        f"\nThis will format the disk(s) in {device}.\n"
        "All existing data will be lost. Continue? [y/N]: "
    ).strip()
    if confirm.lower() != "y":
        print("Aborted.")
        device.close()
        return True

    print("\nStarting format...")
    state = {"total": 0}

    ok = device.format_disk(fmt)
    print()  # newline after progress line

    if not ok:
        print(f"error: format failed: {icsneopy.get_last_error().describe()}")
        device.close()
        return False

    print("Format complete!")

    # Verify
    print("\nVerifying disk state after format... ", end="", flush=True)
    post = device.get_disk_details()
    if post is None:
        print("FAIL (could not re-query disk details)")
    else:
        print("OK")
        for i, disk in enumerate(post.disks):
            print(f"  Disk [{i}] formatted: {'yes' if disk.formatted else 'no'}")

    device.close()
    return True


if __name__ == "__main__":
    sys.exit(0 if disk_format_example() else 1)
