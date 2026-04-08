#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2messages.h>
#include <icsneo/icsneoc2settings.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <inttypes.h>
#include <time.h>

/**
 * Sleeps for a specified number of milliseconds using Sleep() on Windows and sleep() on *nix.
 *
 * @param ms The number of milliseconds to sleep.
 */
void sleep_ms(uint32_t ms) {
#ifdef _WIN32
	Sleep(ms);
#else
	sleep(ms / 1000);
#endif
}

/**
 * Prints an error message with the given string and error code.
 *
 * If the error code is not icsneoc2_error_success, prints the error string for the given error code
 * and returns the error code.
 *
 * @param message The message to print.
 * @param error The error code to print.
 * @return error as int
 */
int print_error_code(const char* message, icsneoc2_error_t error) {
	char error_str[64];
	size_t error_str_len = sizeof(error_str);
	icsneoc2_error_t res = icsneoc2_error_code_get(error, error_str, &error_str_len);
	if(res != icsneoc2_error_success) {
		printf("%s: Failed to get string for error code %d with error code %d\n", message, error, res);
		return res;
	}
	printf("%s: \"%s\" (%u)\n", message, error_str, error);
	return (int)error;
}

/**
 * Processes a list of messages from a device.
 *
 * This function iterates over a given array of messages received from a specified device.
 * For each message in the array, it retrieves and prints the message type and bus type.
 * If an error occurs while retrieving these details, an error message is printed.
 *
 * @param messages An array of pointers to icsneoc2_message_t structures containing the messages to process.
 * @param messages_count The number of messages in the messages array.
 *
 * @return An icsneoc2_error_t value indicating success or failure of the message processing.
 */
int process_messages(icsneoc2_message_t** messages, size_t messages_count);

/**
 * Prints all events
 *
 * @param device_description A description of the device used in the output.
 */
void print_events(const char* device_description);

/**
 * Transmits a series of CAN messages from a device.
 *
 * This function creates and transmits 100 CAN messages with incrementing payload data.
 * Each message is configured with specific attributes such as network ID, arbitration
 * ID, CANFD status, extended status, and baudrate switch. After successfully transmitting
 * each message, it is freed from memory.
 *
 * @param device A pointer to the icsneoc2_device_t structure representing the device to transmit messages from.
 *
 * @return An icsneoc2_error_t value indicating success or failure of the message transmission process.
 */
int transmit_can_messages(icsneoc2_device_t* device);

/**
 * Get the RTC (Real time clock) of a device and print it.
 *
 * @param[in] device The device to get the RTC of.
 * @param[in] description A description of the device for printing purpose.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t get_and_print_rtc(icsneoc2_device_t* device);

int main() {
	icsneoc2_device_info_t* found_devices = NULL;

	printf("Finding devices...\n");
	icsneoc2_error_t res = icsneoc2_device_enumerate(0, &found_devices);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to find devices", res);
	}
	if(found_devices == NULL) {
		printf("No devices found, exiting\n");
		return 0;
	}
	// Count and list off the devices
	size_t devices_count = 0;
	for(icsneoc2_device_info_t* cur = found_devices; cur != NULL; cur = icsneoc2_device_info_next(cur)) {
		devices_count++;
	}
	printf("OK, %zu device%s found\n", devices_count, devices_count == 1 ? "" : "s");
	for(icsneoc2_device_info_t* cur = found_devices; cur != NULL; cur = icsneoc2_device_info_next(cur)) {
		// Get description of the device
		char description[255] = {0};
		size_t description_length = 255;
		res = icsneoc2_device_info_description_get(cur, description, &description_length);
		if(res != icsneoc2_error_success) {
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to get device description", res);
		};
		printf("%.*s\n", (int)description_length, description);
		// Open the device without RTC sync and going online
		icsneoc2_open_options_t options = icsneoc2_open_options_default;
		options &= ~ICSNEOC2_OPEN_OPTIONS_SYNC_RTC;
		options &= ~ICSNEOC2_OPEN_OPTIONS_GO_ONLINE;
		printf("\tDevice open options: 0x%x\n", options);
		printf("\tOpening device: %s...\n", description);
		icsneoc2_device_t* open_device = NULL;
		res = icsneoc2_device_create(cur, &open_device);
		if(res != icsneoc2_error_success) {
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to create device", res);
		}
		res = icsneoc2_device_open(open_device, options);
		if(res != icsneoc2_error_success) {
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to open device", res);
		};

		// Get timestamp resolution of the device
		printf("\tGetting timestamp resolution... ");
		uint32_t timestamp_resolution = 0;
		res = icsneoc2_device_timestamp_resolution_get(open_device, &timestamp_resolution);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to get timestamp resolution", res);
		}
		printf("%uns\n", timestamp_resolution);
		// Get baudrates for HSCAN
		printf("\tGetting DW CAN 01 Baudrate... ");
		int64_t baudrate = 0;
		res = icsneoc2_settings_baudrate_get(open_device, icsneoc2_netid_dwcan_01, &baudrate);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to get baudrate", res);
		};
		printf("%" PRIu64 "mbit/s\n", baudrate);
		// Get FDbaudrates for HSCAN
		printf("\tGetting FD DW CAN 01 Baudrate... ");
		int64_t fd_baudrate = 0;
		res = icsneoc2_settings_canfd_baudrate_get(open_device, icsneoc2_netid_dwcan_01, &fd_baudrate);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to get FD baudrate", res);
		};
		printf("%" PRIu64 "mbit/s\n", fd_baudrate);
		// Set baudrates for HSCAN
		// save_to_device: If this is set to true, the baudrate will be saved on the device
		// and will persist through a power cycle
		printf("\tSetting DW CAN 01 Baudrate... ");
		res = icsneoc2_settings_baudrate_set(open_device, icsneoc2_netid_dwcan_01, baudrate);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to set baudrate", res);
		};
		printf("Ok\n");
		// Set FDbaudrates for HSCAN
		printf("\tSetting FD DW CAN 01 Baudrate... ");
		res = icsneoc2_settings_canfd_baudrate_set(open_device, icsneoc2_netid_dwcan_01, fd_baudrate);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to set FD baudrate", res);
		};
		printf("Ok\n");
		// Get RTC
		printf("\tGetting RTC... ");
		res = get_and_print_rtc(open_device);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to get RTC", res);
		}
		// Set RTC
		printf("\tSetting RTC to current time... ");
		time_t current_time = time(NULL);
		res = icsneoc2_device_rtc_set(open_device, current_time);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to set RTC", res);
		}
		printf("Ok\n");
		// Get RTC
		printf("\tGetting RTC... ");
		res = get_and_print_rtc(open_device);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to get RTC", res);
		}
		// Go online, start acking traffic
		printf("\tGoing online... ");
		res = icsneoc2_device_go_online(open_device, true);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to go online", res);
		}
		// Redundant check to show how to check if the device is online, if the previous
		// icsneoc2_device_go_online call was successful we can assume we are online already
		bool is_online = false;
		res = icsneoc2_device_is_online(open_device, &is_online);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to check if online", res);
		}
		printf("%s\n", is_online ? "Online" : "Offline");
		// Transmit CAN messages
		res = transmit_can_messages(open_device);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to transmit CAN messages", res);
		}
		// Wait for the bus to collect some messages, requires an active bus to get messages
		printf("\tWaiting 1 second for messages...\n");
		sleep_ms(1000);
		// Get the messages
		icsneoc2_message_t* messages[20000] = {0};
		size_t message_count = 20000;
		printf("\tGetting messages from device with timeout of 3000ms on %s...\n", description);
		for(size_t i = 0; i < message_count; ++i) {
			res = icsneoc2_device_message_get(open_device, &messages[i], 0);
			if(res != icsneoc2_error_success) {
				for(size_t j = 0; j < i; ++j) {
					icsneoc2_message_free(messages[j]);
				}
				print_events(description);
				icsneoc2_device_free(open_device);
				icsneoc2_enumeration_free(found_devices);
				return print_error_code("\tFailed to get messages from device", res);
			};
			if(messages[i] == NULL) {
				// no more messages
				message_count = i;
				break;
			}
		}
		// Process the messages
		res = process_messages(messages, message_count);
		if(res != icsneoc2_error_success) {
			for(size_t i = 0; i < message_count; ++i) {
				icsneoc2_message_free(messages[i]);
			}
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to process messages", res);
		}
		for(size_t i = 0; i < message_count; ++i) {
			icsneoc2_message_free(messages[i]);
		}
		// Finally, close the device.
		printf("\tClosing device: %s...\n", description);
		res = icsneoc2_device_close(open_device);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			icsneoc2_enumeration_free(found_devices);
			return print_error_code("\tFailed to close device", res);
		};
		// Print device events
		print_events(description);
		icsneoc2_device_free(open_device);
	}
	icsneoc2_enumeration_free(found_devices);
	printf("\n");
	return 0;
}

icsneoc2_error_t get_and_print_rtc(icsneoc2_device_t* device) {
	time_t unix_epoch = 0;
	icsneoc2_error_t res = icsneoc2_device_rtc_get(device, &unix_epoch);
	if(res != icsneoc2_error_success) {
		return res;
	}
	char rtc_time[32] = {0};
	strftime(rtc_time, sizeof(rtc_time), "%Y-%m-%d %H:%M:%S", localtime(&unix_epoch));
	printf("RTC: %lld %s\n", (long long)unix_epoch, rtc_time);

	return icsneoc2_error_success;
}

void print_events(const char* device_description) {
	icsneoc2_event_t* events[1024] = {0};
	size_t events_count = 1024;
	for(size_t i = 0; i < events_count; ++i) {
		// no device filter, get all events
		icsneoc2_error_t res = icsneoc2_event_get(&events[i], NULL);
		if(res != icsneoc2_error_success) {
			for(size_t j = 0; j < i; ++j) {
				icsneoc2_event_free(events[j]);
			}
			(void)print_error_code("\tFailed to get device events", res);
			return;
		}
		if(events[i] == NULL) {
			events_count = i;
			break;
		}
	}
	// Loop over each event and describe it.
	for(size_t i = 0; i < events_count; i++) {
		char event_description[255] = {0};
		size_t event_description_length = 255;
		icsneoc2_error_t res = icsneoc2_event_description_get(events[i], event_description, &event_description_length);
		if(res != icsneoc2_error_success) {
			print_error_code("\tFailed to get event description", res);
			continue;
		}
		printf("\t%s: Event %zu: %s\n", device_description, i, event_description);
	}
	for(size_t i = 0; i < events_count; i++) {
		icsneoc2_event_free(events[i]);
	}
	printf("\t%s: Received %zu events\n", device_description, events_count);
}

int process_messages(icsneoc2_message_t** messages, size_t messages_count) {
	// Print the type and bus type of each message
	size_t tx_count = 0;
	for(size_t i = 0; i < messages_count; i++) {
		icsneoc2_message_t* message = messages[i];
		bool is_frame = false;
		icsneoc2_error_t res = icsneoc2_message_is_frame(message, &is_frame);
		if(res != icsneoc2_error_success) {
			return print_error_code("\tFailed to check if message is a frame", res);
		}
		if(!is_frame) {
			printf("Ignoring non-frame message at index %zu\n", i);
			continue;
		}
		icsneoc2_network_type_t network_type;
		res = icsneoc2_message_network_type_get(message, &network_type);
		if(res != icsneoc2_error_success) {
			return print_error_code("\tFailed to get message network type", res);
		}

		char network_type_name[128] = {0};
		size_t network_type_name_length = 128;
		res = icsneoc2_network_type_name_get(network_type, network_type_name, &network_type_name_length);
		if(res != icsneoc2_error_success) {
			return print_error_code("\tFailed to get network type name", res);
		}
		bool is_tx = false;
		res = icsneoc2_message_is_transmit(message, &is_tx);
		if(res != icsneoc2_error_success) {
			return print_error_code("\tFailed to get message is transmit", res);
		}
		if(is_tx) {
			tx_count++;
			continue;
		}

		printf("\t%zd) network type: %s (%u)\n", i, network_type_name, network_type);

		if(network_type == icsneoc2_network_type_can) {
			uint32_t arbid = 0;
			int32_t dlc = 0;
			icsneoc2_netid_t netid = 0;
			bool is_remote = false;
			bool is_canfd = false;
			bool is_extended = false;
			uint8_t data[64] = {0};
			size_t data_length = 64;
			char netid_name[128] = {0};
			size_t netid_name_length = 128;
			icsneoc2_error_t result = icsneoc2_message_netid_get(message, &netid);
			result += icsneoc2_netid_name_get(netid, netid_name, &netid_name_length);
			result += icsneoc2_message_data_get(message, data, &data_length);
			if(result != icsneoc2_error_success) {
				printf("\tFailed get get CAN parameters (error: %u) for index %zu\n", result, i);
				continue;
			}
			printf("\t  NetID: %s (0x%x)\tArbID: 0x%x\t DLC: %u\t Remote: %d\t CANFD: %d\t Extended: %d\t Data length: %zu\n", netid_name, netid, arbid, dlc, is_remote, is_canfd, is_extended, data_length);
			printf("\t  Data: [");
			for(size_t x = 0; x < data_length; x++) {
				printf(" 0x%x", data[x]);
			}
			printf(" ]\n");
		}
	}
	printf("\tReceived %zu messages total, %zu were TX messages\n", messages_count, tx_count);

	return icsneoc2_error_success;
}

int transmit_can_messages(icsneoc2_device_t* device) {
	uint64_t counter = 0;
	const size_t msg_count = 100;
	printf("\tTransmitting %zd messages...\n", msg_count);
	for(size_t i = 0; i < msg_count; i++) {
		// Create the message
		icsneoc2_message_t* message = NULL;
		icsneoc2_error_t res = icsneoc2_message_can_create(&message);
		if(res != icsneoc2_error_success) {
			return print_error_code("\tFailed to create messages", res);
		}
		// Set the message attributes
		res = icsneoc2_message_netid_set(message, icsneoc2_netid_dwcan_01);
		uint64_t arb_id = 0x10;
		uint64_t flags = ICSNEOC2_MESSAGE_CAN_FLAGS_BRS | ICSNEOC2_MESSAGE_CAN_FLAGS_IDE | ICSNEOC2_MESSAGE_CAN_FLAGS_FDF;
		res += icsneoc2_message_can_props_set(message, &arb_id, &flags);
		res += icsneoc2_message_data_set(message, (uint8_t*)&counter, sizeof(counter));
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(message);
			return print_error_code("\tFailed to modify message", res);
		}
		res = icsneoc2_device_message_transmit(device, message);
		res += icsneoc2_message_free(message);
		if(res != icsneoc2_error_success) {
			return print_error_code("\tFailed to transmit message", res);
		}
		counter++;
	}

	return icsneoc2_error_success;
}
