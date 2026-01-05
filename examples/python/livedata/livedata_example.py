"""
LiveData subscription and monitoring example using icsneopy library.

"""

import icsneopy
import time
from datetime import timedelta


def livedata_example():
	"""Subscribe to and monitor LiveData signals."""
	devices = icsneopy.find_all_devices()
	if not devices:
		raise RuntimeError("No devices found")

	device = devices[0]
	print(f"Using device: {device}")

	try:
		if not device.open():
			raise RuntimeError("Failed to open device")

		if not device.go_online():
			raise RuntimeError("Failed to go online")

		device.enable_message_polling()

		# Create subscription message
		msg = icsneopy.LiveDataCommandMessage()
		msg.handle = icsneopy.get_new_handle()
		msg.cmd = icsneopy.LiveDataCommand.SUBSCRIBE
		msg.update_period = timedelta(milliseconds=500)
		msg.expiration_time = timedelta(milliseconds=0)

		# Subscribe to various LiveData signals
		msg.append_signal_arg(icsneopy.LiveDataValueType.GPS_LATITUDE)
		msg.append_signal_arg(icsneopy.LiveDataValueType.GPS_LONGITUDE)
		msg.append_signal_arg(icsneopy.LiveDataValueType.GPS_ACCURACY)
		msg.append_signal_arg(icsneopy.LiveDataValueType.DAQ_ENABLE)
		msg.append_signal_arg(icsneopy.LiveDataValueType.MANUAL_TRIGGER)
		msg.append_signal_arg(icsneopy.LiveDataValueType.TIME_SINCE_MSG)

		print("\nSubscribing to LiveData signals...")
		if not device.subscribe_live_data(msg):
			raise RuntimeError(f"Subscription failed: {icsneopy.get_last_error()}")

		print("Subscription successful")
		print("\nMonitoring LiveData for 5 seconds...")

		response_count = 0
		start_time = time.time()

		while time.time() - start_time < 5:
			result = device.get_messages()
			messages = result[0] if isinstance(result, tuple) else result

			for m in messages:
				if isinstance(m, icsneopy.LiveDataStatusMessage):
					if m.handle == msg.handle:
						print(f"\n[Status] Command: {m.requested_command}, Status: {m.status}")

				elif isinstance(m, icsneopy.LiveDataValueMessage):
					if m.handle == msg.handle:
						response_count += 1
						print(f"\n[Response #{response_count}]")
						signal_names = ["GPS_LAT", "GPS_LON", "GPS_ACC", 
									   "DAQ_EN", "MAN_TRIG", "TIME_SINCE"]
						for idx, val in enumerate(m.values):
							value = icsneopy.livedata_value_to_double(val)
							name = signal_names[idx] if idx < len(signal_names) else f"Signal_{idx}"
							print(f"  {name:12s}: {value:10.2f}")

			time.sleep(0.1)

		print(f"\nReceived {response_count} response messages")

		# Demonstrate setting values
		print("\nSetting custom values...")
		set_msg = icsneopy.LiveDataSetValueMessage()
		set_msg.handle = icsneopy.get_new_handle()
		set_msg.cmd = icsneopy.LiveDataCommand.SET_VALUE

		# Set DAQ_ENABLE
		value = icsneopy.livedata_double_to_value(1.0)
		if value:
			set_msg.append_set_value(icsneopy.LiveDataValueType.DAQ_ENABLE, value)

		# Set MANUAL_TRIGGER
		value = icsneopy.livedata_double_to_value(1.0)
		if value:
			set_msg.append_set_value(icsneopy.LiveDataValueType.MANUAL_TRIGGER, value)

		if device.set_value_live_data(set_msg):
			print("Values set successfully")
			time.sleep(0.5)

			# Check the results
			result = device.get_messages()
			messages = result[0] if isinstance(result, tuple) else result
			for m in messages:
				if isinstance(m, icsneopy.LiveDataStatusMessage):
					if m.handle == set_msg.handle:
						print(f" Set status: {m.status}")

		# Keep device awake by resetting TIME_SINCE_MSG
		print("\nResetting TIME_SINCE_MSG to keep device awake...")
		reset_msg = icsneopy.LiveDataSetValueMessage()
		reset_msg.handle = icsneopy.get_new_handle()
		reset_msg.cmd = icsneopy.LiveDataCommand.SET_VALUE
		
		value = icsneopy.livedata_double_to_value(0.0)
		if value:
			reset_msg.append_set_value(icsneopy.LiveDataValueType.TIME_SINCE_MSG, value)
			if device.set_value_live_data(reset_msg):
				print("TIME_SINCE_MSG reset to 0")

		# Unsubscribe
		print("\nUnsubscribing...")
		if device.unsubscribe_live_data(msg.handle):
			print("Unsubscribed successfully")

	finally:
		device.close()
		print("\nDevice closed")


if __name__ == "__main__":
	livedata_example()

