
# Basic FlexRay frame reception example using icsneopy library.


import icsneopy
import time
import signal
import sys


def get_controller_config(slot_id):
	"""Create a FlexRay controller configuration matching the network.
	
	Args:
		slot_id: The key slot ID for this node (must be unique per node)
		
	Returns:
		FlexRay.Controller.Configuration with all parameters set
		
	Note:
		For passive listening on an existing network, set:
		- key_slot_used_for_startup = False
		- Remove set_allow_coldstart(True) call below
	"""
	config = icsneopy.FlexRay.Controller.Configuration()
	config.accept_startup_range_microticks = 160
	config.allow_halt_due_to_clock = True
	config.allow_passive_to_active_cycle_pairs = 15
	config.cluster_drift_damping = 2
	
	# Physical channel configuration (Channel A only for this example)
	config.channel_a = True
	config.channel_b = False  # Single channel A only
	
	config.decoding_correction_microticks = 56
	config.delay_compensation_a_microticks = 28
	config.delay_compensation_b_microticks = 28
	config.extern_offset_correction_control = 0
	config.extern_rate_correction_control = 0
	config.extern_offset_correction_microticks = 0
	config.extern_rate_correction_microticks = 0
	
	# KEY SLOT CONFIGURATION - Critical for FlexRay operation
	config.key_slot_id = slot_id  # Must be unique per node
	config.key_slot_only_enabled = False
	config.key_slot_used_for_startup = True   # True = participate in coldstart
	config.key_slot_used_for_sync = True      # True = synchronize with network
	
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
	"""Create a FlexRay cluster configuration matching the network."""
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


def receive_flexray_frames():
	"""Receive FlexRay frames as passive node with callback handling."""
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

	frame_count = 0
	running = True

	def on_frame(frame):
		nonlocal frame_count
		if isinstance(frame, icsneopy.FlexRayMessage):
			# Only show frames from slot 1 (filter out null frames)
			if frame.slotid == 1:
				frame_count += 1
				# Nice formatted view of the frame
				payload_hex = ' '.join([f'{b:02X}' for b in frame.data[:8]])
				payload_dec = ' '.join([f'{b:3d}' for b in frame.data[:8]])
				print(f"[Frame {frame_count:4d}] Slot: {frame.slotid:2d} | Cycle: {frame.cycle:2d} | "
					  f"Channel: {str(frame.channel):10s}")
				print(f"              Hex: [{payload_hex}]")
				print(f"              Dec: [{payload_dec}]\n")

	def signal_handler(sig, frame):
		nonlocal running
		print("\nShutting down...")
		running = False

	signal.signal(signal.SIGINT, signal_handler)
	signal.signal(signal.SIGTERM, signal_handler)

	frame_filter = icsneopy.MessageFilter(icsneopy.Network.NetID.FLEXRAY_02)
	callback = icsneopy.MessageCallback(on_frame, frame_filter)

	try:
		# Configure FlexRay controller 1 (FLEXRAY_02) as passive node
		controllers = device.get_flexray_controllers()
		if len(controllers) < 2:
			raise RuntimeError("Device needs at least 2 FlexRay controllers")

		controller = controllers[1]  # Use controller 1
		cluster_config = get_cluster_config()
		controller_config = get_controller_config(slot_id=2)

		# Enable coldstart so this node transmits and coldstart node sees activity
		controller.set_allow_coldstart(True)
		controller.set_configuration(cluster_config, controller_config)
		controller.set_start_when_going_online(True)

		if not device.open():
			raise RuntimeError("Failed to open device")

		if not device.go_online():
			raise RuntimeError("Failed to go online")

		device.add_message_callback(callback)
		print("="*60)
		print("FlexRay Receive Node - Coldstart Config Loaded")
		print("="*60)
		print(f"Controller: FLEXRAY_02 | Slot ID: 2 | Channel: A")
		print(f"Listening for frames...")
		print(f"Start the transmit script now to begin communication")
		print("="*60)
		print("Press Ctrl+C to stop\n")

		while running:
			time.sleep(0.1)

		print(f"\nTotal frames received: {frame_count}")

	finally:
		device.close()


if __name__ == "__main__":
	receive_flexray_frames()
