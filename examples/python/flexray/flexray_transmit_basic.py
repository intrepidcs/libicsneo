
# Basic FlexRay frame transmission example using icsneopy library.


import icsneopy
import time
import signal
import sys
import random


def get_controller_config(slot_id, is_coldstart=False):
	"""Create a FlexRay controller configuration matching the network.
	
	Args:
		slot_id: The key slot ID for this node (must be unique per node)
		is_coldstart: True if this node participates in coldstart
		
	Returns:
		FlexRay.Controller.Configuration with all parameters set
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
	config.key_slot_used_for_startup = is_coldstart  # True = coldstart node
	config.key_slot_used_for_sync = is_coldstart     # True = provides sync
	
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
	"""Create a FlexRay cluster configuration matching the network.
	
	All nodes on the FlexRay network must have identical cluster parameters.
	These define the timing and structure of the FlexRay communication cycle.
	
	Key parameters:
	- cycle_duration_micro_sec: 5000 = 5ms cycle time
	- macroticks_per_cycle: 5000 macroticks per cycle
	- number_of_static_slots: 32 static slots for guaranteed transmission
	- payload_length_of_static_slot_in_words: 67 words = 134 bytes max payload
	
	Returns:
		FlexRay.Cluster.Configuration with all timing parameters set
	"""
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


def transmit_flexray_frame():
	"""Transmit FlexRay frames as coldstart node."""
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

	running = True

	def signal_handler(sig, frame):
		nonlocal running
		print("\nShutting down...")
		running = False

	signal.signal(signal.SIGINT, signal_handler)
	signal.signal(signal.SIGTERM, signal_handler)

	try:
		# Configure FlexRay controller 0 (FLEXRAY_01) as coldstart node
		controllers = device.get_flexray_controllers()
		if not controllers:
			raise RuntimeError("Device has no FlexRay controllers")

		controller = controllers[0]  # Use controller 0
		cluster_config = get_cluster_config()
		controller_config = get_controller_config(slot_id=1, is_coldstart=True)

		# Enable coldstart capability
		controller.set_allow_coldstart(True)
		controller.set_configuration(cluster_config, controller_config)
		controller.set_start_when_going_online(True)

		if not device.open():
			raise RuntimeError("Failed to open device")

		if not device.go_online():
			raise RuntimeError("Failed to go online")

		print("="*60)
		print("FlexRay Transmit Node - Starting Network")
		print("="*60)
		print(f"Controller: FLEXRAY_01 | Slot ID: 1 | Channel: A")
		print(f"Transmitting frames continuously...")
		print("="*60)
		print("Press Ctrl+C to stop\n")

		# Transmit frames continuously starting immediately
		counter = 0
		sensor_temp = 20.0  # Simulated temperature sensor
		sensor_pressure = 100.0  # Simulated pressure sensor
		
		while running:
			# Create new frame each time (important for FlexRay)
			frame = icsneopy.FlexRayMessage()
			frame.network = icsneopy.Network(icsneopy.Network.NetID.FLEXRAY_01)
			frame.slotid = 1
			frame.cycle = 0
			frame.cycle_repetition = 1
			frame.channel = icsneopy.FlexRay.Channel.A
			
			# Simulate realistic sensor data
			sensor_temp += random.uniform(-0.5, 0.5)  # Temperature varies
			sensor_pressure += random.uniform(-2.0, 2.0)  # Pressure varies
			
			# Pack data: [status, counter, temp_high, temp_low, pressure_high, pressure_low, checksum_placeholder, sequence]
			status_byte = 0xA0 | (counter % 16)  # Status with rolling bits
			temp_int = int(sensor_temp * 10) & 0xFFFF
			pressure_int = int(sensor_pressure * 10) & 0xFFFF
			
			frame.data = (
				status_byte,
				counter & 0xFF,
				(temp_int >> 8) & 0xFF,
				temp_int & 0xFF,
				(pressure_int >> 8) & 0xFF,
				pressure_int & 0xFF,
				random.randint(0, 255),  # Random data
				(counter >> 8) & 0xFF
			)
			
			success = device.transmit(frame)
			if counter % 100 == 0:  # Print every 100th to reduce spam
				if success:
					print(f"  [TX {counter}] Temp: {sensor_temp:.1f}°C | Pressure: {sensor_pressure:.1f} kPa")
				else:
					print(f"  Frame {counter}: Failed to transmit")
			counter += 1
			time.sleep(0.005)  # 5ms per cycle

		print("\nTransmission complete!")

	finally:
		device.close()


if __name__ == "__main__":
	transmit_flexray_frame()
