"""
Complete SPI example for 10BASE-T1S MACPHYs using icsneopy library.

Demonstrates device setup and SPI frame transmission/reception.
"""

import icsneopy
import time


def setup_device():
    """Initialize SPI device."""
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


def transmit_spi_frame(device, mms, addr, dir, write_data=None):
    """Transmit a SPI frame."""
    frame = icsneopy.SPIMessage()
    frame.network = icsneopy.Network(icsneopy.Network.NetID.SPI_01)
    frame.direction = dir
    frame.mms = mms
    frame.address = addr
    if dir == icsneopy.SPIMessage.Direction.Read:
        frame.payload = [0]  # single register
    else:
        frame.payload = [write_data]

    success = device.transmit(frame)
    if success:
        print("Frame transmitted")
    else:
        print("Failed to transmit frame")

    return success


def setup_spi_reception(device):
    """Configure SPI frame reception with callback."""

    def frame_handler(frame):
        access = (
            "Write"
            if frame.direction == icsneopy.SPIMessage.Direction.Write
            else "Read"
        )
        print(
            f"{access}, "
            f"MMS: 0x{frame.mms:02X}, "
            f"Register: 0x{frame.address:03X}, "
            f"Data: {[hex(b) for b in frame.payload]}, "
            f"Length: {len(frame.payload)}"
        )

    frame_filter = icsneopy.MessageFilter(icsneopy.Network.NetID.SPI_01)
    callback = icsneopy.MessageCallback(frame_handler, frame_filter)
    device.add_message_callback(callback)

    print("SPI frame reception configured")


def cleanup_device(device):
    """Close device connection."""
    if device:
        device.close()
        print("Device connection closed")


def main():
    """Complete SPI example with proper error handling."""
    device = None

    try:
        # Setup device
        device = setup_device()

        # Open device
        if not open_device(device):
            raise RuntimeError("Failed to initialize device")

        # Setup frame reception
        setup_spi_reception(device)

        # Read 10BASE-T1S MACPHY ID register
        transmit_spi_frame(device, 0x0, 0x0001, icsneopy.SPIMessage.Direction.Read)

        # Change 10BASE-T1S Test mode control
        transmit_spi_frame(device, 0x3, 0x08FB, icsneopy.SPIMessage.Direction.Read)
        transmit_spi_frame(
            device, 0x3, 0x08FB, icsneopy.SPIMessage.Direction.Write, 0x6000
        )
        transmit_spi_frame(device, 0x3, 0x08FB, icsneopy.SPIMessage.Direction.Read)

        time.sleep(0.1)

        # Listen for responses
        print("Listening for SPI frames for 5 seconds...")
        time.sleep(5)

    except Exception as e:
        print(f"Error: {e}")
        return 1

    finally:
        cleanup_device(device)

    return 0


if __name__ == "__main__":
    main()
