#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2settings.h>
#include <icsneo/icsneoc2messages.h>

#include <stdio.h>
#include <inttypes.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


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
 * Prints all events
 *
 * @param device_description A description of the device used in the output.
 */
void print_events(const char* device_description);

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
int process_message(icsneoc2_message_t** messages, size_t messages_count);

int main() {
	// Open the first available device with default options
	printf("Opening first available device...\n");
	icsneoc2_device_t* open_device = NULL;
	icsneoc2_error_t res = icsneoc2_device_open_first(0, icsneoc2_open_options_default, &open_device);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to open first device", res);
	};

	// Get a description of the opened device
	char description[255] = {0};
	size_t description_length = 255;
	res = icsneoc2_device_description_get(open_device, description, &description_length);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_free(open_device);
		return print_error_code("\tFailed to get device description", res);
	};
	printf("\tOpened device: %s\n", description);

	// Get the messages
	icsneoc2_message_t* messages[20000] = {0};
	size_t message_count = 20000;
	time_t start_time = time(NULL);
	printf("\tGetting messages from device with timeout of 3000ms on %s...\n", description);
	for(size_t i = 0; i < message_count; ++i) {
		res = icsneoc2_device_message_get(open_device, &messages[i], 0);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_free(open_device);
			return print_error_code("\tFailed to get messages from device", res);
		};
		if(messages[i] == NULL) {
			// no more messages
			message_count = i;
			break;
		}
	}
	if(res != icsneoc2_error_success) {
		print_events(description);
		icsneoc2_device_free(open_device);
		return print_error_code("\tFailed to get messages from device", res);
	}
	time_t end_time = time(NULL);
	printf("\tGot %zu messages in %lld seconds\n", message_count, (long long)(end_time - start_time));
	// Process the messages
	res = process_message(messages, message_count);
	if(res != icsneoc2_error_success) {
		print_events(description);
		icsneoc2_device_free(open_device);
		return print_error_code("\tFailed to process messages", res);
	}
	// Finally, close the device.
	printf("\tClosing device: %s...\n", description);
	res = icsneoc2_device_close(open_device);
	if(res != icsneoc2_error_success) {
		print_events(description);
		icsneoc2_device_free(open_device);
		return print_error_code("\tFailed to close device", res);
	};
	icsneoc2_device_free(open_device);

	printf("\n");
	return 0;
}

void print_events(const char* device_description) {
	icsneoc2_event_t* events[1024] = {0};
	size_t events_count = 1024;
	for(size_t i = 0; i < events_count; ++i) {
		// no device filter, get all events
		icsneoc2_error_t res = icsneoc2_event_get(&events[i], NULL);
		if(res != icsneoc2_error_success) {
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

int process_message(icsneoc2_message_t** messages, size_t messages_count) {
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
			return print_error_code("\tFailed to get message bus type name", res);
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
		res += icsneoc2_message_data_set(message, (uint8_t*)&counter, sizeof(counter));
		if(res != icsneoc2_error_success) {
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
