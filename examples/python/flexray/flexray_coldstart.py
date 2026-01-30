"""
FlexRay coldstart example using icsneopy library.

Demonstrates coldstart capability where one FlexRay device can start
the network without needing other devices connected.

CRITICAL COLDSTART REQUIREMENTS:
1. key_slot_used_for_startup = True
2. key_slot_used_for_sync = True
3. set_allow_coldstart(True)
4. Each controller needs a unique key_slot_id
5. Proper bus termination (required for FlexRay)
"""

import icsneopy
import time


def get_coldstart_controller_config(slot_id):
	"""
	Create FlexRay controller configuration for COLDSTART.
	
	The three critical settings for coldstart are marked below.
	"""
	config = icsneopy.FlexRay.Controller.Configuration()
	config.accept_startup_range_microticks = 160
	config.allow_halt_due_to_clock = True
	config.allow_passive_to_active_cycle_pairs = 15
	config.cluster_drift_damping = 2
	config.channel_a = True
	config.channel_b = True
	config.decoding_correction_microticks = 56
	config.delay_compensation_a_microticks = 28
	config.delay_compensation_b_microticks = 28
	config.extern_offset_correction_control = 0
	config.extern_rate_correction_control = 0
	config.extern_offset_correction_microticks = 0
	config.extern_rate_correction_microticks = 0
	
	# CRITICAL FOR COLDSTART: Set the key slot ID
	config.key_slot_id = slot_id
	
	config.key_slot_only_enabled = False
	
	# CRITICAL FOR COLDSTART: Enable startup and sync on key slot
	config.key_slot_used_for_startup = True  # Required for coldstart
	config.key_slot_used_for_sync = True     # Required for coldstart
	
	config.latest_tx_minislot = 226
	config.listen_timeout = 401202
	config.macro_initial_offset_a = 7
	config.macro_initial_offset_b = 7
	config.micro_initial_offset_a = 36
	config.micro_initial_offset_b = 36
	config.micro_per_cycle = 200000
	config.mts_on_a = False
	config.mts_on_b = False
	config.offset_correction_out_microticks = 189
	config.rate_correction_out_microticks = 601
	config.second_key_slot_id = 0
	config.two_key_slot_mode = False
	config.wakeup_pattern = 55
	config.wakeup_on_channel_b = False
	return config


def get_cluster_config():
	"""Create FlexRay cluster configuration."""
	config = icsneopy.FlexRay.Cluster.Configuration()
	config.speed = icsneopy.FlexRay.Cluster.SpeedType.FLEXRAY_BAUDRATE_10M
	config.strobe_point_position = icsneopy.FlexRay.Cluster.SPPType.FLEXRAY_SPP_5
	config.action_point_offset = 4
	config.casr_x_low_max = 64
	config.cold_start_attempts = 8
	config.cycle_duration_micro_sec = 5000
	config.dynamic_slot_idle_phase_minislots = 1
	config.listen_noise_macroticks = 4
	config.macroticks_per_cycle = 5000
	config.macrotick_duration_micro_sec = 1
	config.max_without_clock_correction_fatal = 2
	config.max_without_clock_correction_passive = 2
	config.minislot_action_point_offset_macroticks = 4
	config.minislot_duration_macroticks = 10
	config.network_idle_time_macroticks = 40
	config.network_management_vector_length_bytes = 1
	config.number_of_minislots = 0
	config.number_of_static_slots = 32
	config.offset_correction_start_macroticks = 4991
	config.payload_length_of_static_slot_in_words = 67
	config.static_slot_macroticks = 155
	config.symbol_window_macroticks = 0
	config.symbol_window_action_point_offset_macroticks = 0
	config.sync_frame_id_count_max = 15
	config.transmission_start_sequence_duration_bits = 11
	config.wakeup_rx_idle_bits = 40
	config.wakeup_rx_low_bits = 40
	config.wakeup_rx_window_bits = 301
	config.wakeup_tx_active_bits = 60
	config.wakeup_tx_idle_bits = 180
	return config


def flexray_coldstart():
	"""Perform FlexRay coldstart operation."""
	devices = icsneopy.find_all_devices()
	if not devices:
		raise RuntimeError("No devices found")

	# Find a device with FlexRay support
	device = None
	for dev in devices:
		if dev.get_extension("FlexRay"):
			device = dev
			break
	
	if not device:
		raise RuntimeError("No FlexRay-capable device found")

	print(f"Using device: {device.get_product_name()} {device.get_serial()}")

	try:
		# Get FlexRay controllers
		controllers = device.get_flexray_controllers()
		if not controllers:
			raise RuntimeError("Device has no FlexRay controllers")

		print(f"Device has {len(controllers)} FlexRay controller(s)")

		# Configure controllers for coldstart
		cluster_config = get_cluster_config()
		base_slot_id = 1

		for i, controller in enumerate(controllers):
			slot_id = base_slot_id + i
			controller_config = get_coldstart_controller_config(slot_id)

			print(f"\nConfiguring controller {i} for COLDSTART:")
			print(f"  Key Slot ID: {slot_id}")
			print(f"  Key Slot Used for Startup: {controller_config.key_slot_used_for_startup}")
			print(f"  Key Slot Used for Sync: {controller_config.key_slot_used_for_sync}")

			# CRITICAL FOR COLDSTART: Enable coldstart capability
			controller.set_allow_coldstart(True)
			print(f"  Allow Coldstart: True")

			# Configure to start when going online
			controller.set_start_when_going_online(True)

			# Set the configuration
			controller.set_configuration(cluster_config, controller_config)

		# Open device
		print("\nOpening device...")
		if not device.open():
			raise RuntimeError("Failed to open device")
		print("Device opened successfully")

		# Go online - this triggers coldstart
		print("\nGoing online (coldstart will initiate)...")
		if not device.go_online():
			raise RuntimeError("Failed to go online - check bus termination and configuration")

		print("Device online successfully!")
		print("\n" + "=" * 60)
		print("✓ FlexRay network started via COLDSTART")
		print("=" * 60)

		# Transmit test messages on the coldstart key slot
		print("\nTransmitting initial test messages...")
		for i in range(5):
			frame = icsneopy.FlexRayMessage()
			frame.network = icsneopy.Network(icsneopy.Network.NetID.FLEXRAY_01)
			frame.slotid = base_slot_id  # Use the first key slot
			frame.cycle = 0
			frame.cycle_repetition = 1
			frame.channel = icsneopy.FlexRay.Channel.AB
			frame.data = (0xAA, 0xBB, 0xCC, 0xDD, i, i+1, i+2, i+3)

			if device.transmit(frame):
				print(f"  ✓ Transmitted message {i+1}")
			else:
				print(f"  ✗ Failed to transmit message {i+1}")

			time.sleep(0.1)

		print("\n" + "=" * 60)
		print("Network is now active and will stay alive.")
		print("You can now run transmit/receive examples in another terminal.")
		print("Press Ctrl+C to stop and shut down the network.")
		print("=" * 60)

		# Keep transmitting periodically to maintain network presence
		counter = 0
		try:
			while True:
				frame = icsneopy.FlexRayMessage()
				frame.network = icsneopy.Network(icsneopy.Network.NetID.FLEXRAY_01)
				frame.slotid = base_slot_id
				frame.cycle = 0
				frame.cycle_repetition = 1
				frame.channel = icsneopy.FlexRay.Channel.AB
				frame.data = (0xCA, 0xFE, 0xBA, 0xBE, counter & 0xFF, 
				             (counter >> 8) & 0xFF, (counter >> 16) & 0xFF, (counter >> 24) & 0xFF)
				
				device.transmit(frame)
				counter += 1
				time.sleep(1)  # Transmit every second
		except KeyboardInterrupt:
			print("\n\nStopping coldstart node...")

		print("\n✓ Coldstart example completed successfully!")

	finally:
		device.close()


if __name__ == "__main__":
	flexray_coldstart()
