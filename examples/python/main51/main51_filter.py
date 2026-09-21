import icsneopy
import time

#
# Main51 messages are device-originated notifications from the firmware's Main51 processor.
# This example filters for RX buffer overflow and TX FIFO overflow events.
#
def main51_filter():
	devices = icsneopy.find_all_devices()

	if len(devices) == 0:
		print("no devices found")
		return False

	device = devices[0]
	print(f"selected {device}")

	def on_main51(message):
		if not isinstance(message, icsneopy.Main51Message):
			return
		if message.command == icsneopy.Command.Main51RxBufferOverflow:
			print("RX buffer overflow")
		elif message.command == icsneopy.Command.Main51TxFifoOverflow:
			print("TX FIFO overflow")
		else:
			print(f"Main51 command: {message.command}")

	filter = icsneopy.Main51MessageFilter()
	callback = icsneopy.MessageCallback(on_main51, filter)
	device.add_message_callback(callback)

	if not device.open():
		print("unable to open device")
		return False

	if not device.go_online():
		print("unable to go online")
		return False

	time.sleep(5)

	return True

if __name__ == "__main__":
	main51_filter()
