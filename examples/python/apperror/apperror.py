import icsneopy
import time

#
# App errors are responses from the device indicating internal runtime errors
# NOTE: To trigger the app error in this example, disable the DW CAN 01 network on the device
# (e.g. with ICS Device Manager)
#
def apperror():
	devices = icsneopy.find_all_devices()

	if len(devices) == 0:
		print("no devices found")
		return False

	device = devices[0]
	print(f"selected {device}")

	def on_apperror(message):
		if message.get_app_error_type() != icsneopy.AppErrorType.NetworkNotEnabled:
			print("unexpected app error type")
			return
		print(message.get_app_error_string())

	filter = icsneopy.MessageFilter(icsneopy.Message.Type.AppError)
	callback = icsneopy.MessageCallback(on_apperror, filter)
	device.add_message_callback(callback)

	if not device.open():
		print("unable to open device")
		return False

	if not device.go_online():
		print("unable to go online")
		return False

	frame = icsneopy.CANMessage()
	frame.network = icsneopy.Network(icsneopy.Network.NetID.DWCAN_01)
	frame.arbid = 0x22
	frame.data = (0xAA, 0xBB, 0xCC)

	if not device.transmit(frame):
		print("failed to transmit frame")
		return False

	time.sleep(2) # wait for error to come back

	return True

if __name__ == "__main__":
	apperror()
