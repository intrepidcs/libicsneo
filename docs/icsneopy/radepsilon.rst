=======================================
Python Examples for RAD-Epsilon Devices
=======================================



Working with PHY Settings
=========================

.. code-block:: python

    import sys
    import icsneopy


    class PhyStaticInfo:

        def __init__(self, index, type, switch_port):
            self.index = index
            self.type = type
            self.switch_port = switch_port

        def __repr__(self):
            return f"{self.switch_port}: Type {self.type} / Index {self.index}"


    # Switch Port name to PHY index and PHY type
    RAD_EPSILON_PHY_INFO = {
        "1": PhyStaticInfo(0, "88Q2112", "1"),
        "2": PhyStaticInfo(1, "88Q2112", "2"),
        "3": PhyStaticInfo(2, "88Q2112", "3"),
        "4": PhyStaticInfo(3, "88Q2112", "4"),
        "5": PhyStaticInfo(4, "88Q2112", "5"),
        "6": PhyStaticInfo(5, "88Q2112", "6"),
        "9": PhyStaticInfo(6, "88Q2112", "9"),
        "10": PhyStaticInfo(7, "88Q2112", "10")
    }

    RAD_EPSILON_XL_PHY_INFO = {
        "A1": PhyStaticInfo(0, "88Q222x", "A1"),
        "A2": PhyStaticInfo(1, "88Q222x", "A2"),
        "A3": PhyStaticInfo(2, "88Q222x", "A3"),
        "A4": PhyStaticInfo(3, "88Q222x", "A4"),
        "A5": PhyStaticInfo(4, "88Q222x", "A5"),
        "A6": PhyStaticInfo(5, "88Q222x", "A6"),
        "A11": PhyStaticInfo(6, "88Q222x", "A11"),
        "A8": PhyStaticInfo(7, "88Q222x", "A8"),
        "B1": PhyStaticInfo(8, "88Q222x", "B1"),
        "B2": PhyStaticInfo(9, "88Q222x", "B2"),
        "B3": PhyStaticInfo(10, "88Q222x", "B3"),
        "B4": PhyStaticInfo(11, "88Q222x", "B4"),
        "B5": PhyStaticInfo(12, "88Q222x", "B5"),
        "B6": PhyStaticInfo(13, "88Q222x", "B6"),
        "B7": PhyStaticInfo(14, "88Q222x", "B7"),
        "B8": PhyStaticInfo(15, "88Q222x", "B8"),
        "A9": PhyStaticInfo(16, "88X3310", "A9"),
        "B9": PhyStaticInfo(17, "88X3310", "B9"),
    }


    def find_specific_device(type: icsneopy.DeviceType.Enum, serial: str | None = None) -> icsneopy.Device | None:
        devices = icsneopy.find_all_devices()
        for device in devices:
            device_type = device.get_type().get_device_type()
            if serial: # always find by serial if specified
                if device.get_serial() == serial:
                    return device
            elif device_type == type:
                return device
        return None


    def find_rad_epsilonxl(serial: str | None = None) -> icsneopy.Device | None:
        return find_specific_device(icsneopy.DeviceType.Enum.RADEpsilonXL, serial)


    def find_rad_epsilon(serial: str | None = None) -> icsneopy.Device | None:
        return find_specific_device(icsneopy.DeviceType.Enum.RADEpsilon, serial)


    def report_phy_settings(device: icsneopy.Device, phy: PhyStaticInfo) -> bool:
        phy_enable = device.settings.get_phy_enable(phy.index)
        phy_mode = device.settings.get_phy_mode(phy.index)
        phy_speed = device.settings.get_phy_speed(phy.index)
        if phy_enable is None or phy_mode is None or phy_speed is None:
            print(f"failed to get settings for PHY{phy.index} on port {phy.switch_port}")
        print(f"PHY {phy.index} ({phy.type})")
        print(f"\tPort: {phy.switch_port}")
        print(f"\tEnabled: {'TRUE' if phy_enable else 'FALSE'}")
        print(f"\tMode: {phy_mode}")
        print(f"\tSpeed: {phy_speed}")
        return True

    def configure_phy(device: icsneopy.Device, phy: PhyStaticInfo, enable: bool, mode: int, speed: int) -> bool:
        if not device.settings.set_phy_enable(phy.index, enable):
            print(f"failed to set enable for PHY{phy.index} at port {phy.switch_port}")
            return False
        if not device.settings.set_phy_mode(phy.index, mode):
            print(f"failed to set mode for PHY{phy.index} at port {phy.switch_port}")
            return False
        if not device.settings.set_phy_speed(phy.index, speed):
            print(f"failed to set speed for PHY{phy.index} at port {phy.switch_port}")
            return False
        if not device.settings.apply(True):
            print(f"failed to apply settings")
            print(icsneopy.get_last_error())
            return False
        print(f"Successfully set settings for PHY{phy.index} at port {phy.switch_port}")
        return True


    # find the first RAD-EpsilonXL
    device = find_rad_epsilonxl()
    if not device:
        print("failed to find a RAD-EpsilonXL to open")
        sys.exit(-1)
    if not device.open():
        print(f"Failed to open {device.describe()}")
        print(icsneopy.get_last_error())
        sys.exit(-1)
    print(f"Opened {device.describe()}")
    print("Applying default settings")
    if not device.settings.apply_defaults():
        print(icsneopy.get_last_error())
        sys.exit(-1)
    print("Reporting PHY settings")
    for phy in RAD_EPSILON_XL_PHY_INFO.values():
        report_phy_settings(device, phy)
    # Setting Port A1 to Master mode, enabled, 1G speed
    phy = RAD_EPSILON_XL_PHY_INFO["A1"]
    if configure_phy(device, phy, True, icsneopy.Settings.EthernetLinkMode.Master, icsneopy.Settings.EthernetLinkSpeed.Speed1G) and device.settings.refresh(True):
        report_phy_settings(device, phy) # report them back
    else:
        print(icsneopy.get_last_error())
        sys.exit(-1)
    device.close()

