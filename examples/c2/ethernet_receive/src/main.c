#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2messages.h>

#include <stdio.h>
#include <inttypes.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void sleep_ms(uint32_t ms) {
#ifdef _WIN32
	Sleep(ms);
#else
	sleep(ms / 1000);
#endif
}

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

void print_events(void) {
	icsneoc2_event_t* events[256] = {0};
	size_t events_count = 256;
	for(size_t i = 0; i < events_count; ++i) {
		icsneoc2_error_t res = icsneoc2_event_get(&events[i], NULL);
		if(res != icsneoc2_error_success) {
			(void)print_error_code("\tFailed to get events", res);
			return;
		}
		if(events[i] == NULL) {
			events_count = i;
			break;
		}
	}
	for(size_t i = 0; i < events_count; i++) {
		char description[255] = {0};
		size_t description_length = 255;
		icsneoc2_error_t res = icsneoc2_event_description_get(events[i], description, &description_length);
		if(res != icsneoc2_error_success) {
			print_error_code("\tFailed to get event description", res);
			continue;
		}
		printf("\tEvent %zu: %s\n", i, description);
	}
	for(size_t i = 0; i < events_count; i++) {
		icsneoc2_event_free(events[i]);
	}
	if(events_count > 0) {
		printf("\tReceived %zu events\n", events_count);
	}
}

void print_mac(const char* label, const uint8_t* mac) {
	printf("%s: %02x:%02x:%02x:%02x:%02x:%02x", label, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int process_ethernet_message(icsneoc2_message_t* message, size_t index) {
	icsneoc2_netid_t netid = 0;
	char netid_name[128] = {0};
	size_t netid_name_length = 128;

	icsneoc2_error_t res = icsneoc2_message_netid_get(message, &netid);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to get netid", res);
	}
	res = icsneoc2_netid_name_get(netid, netid_name, &netid_name_length);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to get netid name", res);
	}

	/* Get data length first */
	size_t data_length = 0;
	res = icsneoc2_message_data_get(message, NULL, &data_length);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to get data length", res);
	}

	printf("\t%zu) Ethernet Frame on %s (0x%x) - %zu bytes\n", index, netid_name, netid, data_length);

	/* Get MAC addresses and EtherType if we have enough data */
	uint8_t dst_mac[6] = {0};
	uint8_t src_mac[6] = {0};
	uint16_t ether_type = 0;

	res = icsneoc2_message_eth_mac_get(message, dst_mac, src_mac);
	if(res == icsneoc2_error_success) {
		printf("\t  ");
		print_mac("Dst", dst_mac);
		printf("  ");
		print_mac("Src", src_mac);
		printf("\n");
	}

	res = icsneoc2_message_eth_ether_type_get(message, &ether_type);
	if(res == icsneoc2_error_success) {
		printf("\t  EtherType: 0x%04x\n", ether_type);
	}

	/* Get flags */
	icsneoc2_message_eth_flags_t flags = 0;
	res = icsneoc2_message_eth_props_get(message, &flags, NULL, NULL);
	if(res == icsneoc2_error_success && flags != 0) {
		printf("\t  Flags: 0x%" PRIx64, flags);
		if(flags & ICSNEOC2_MESSAGE_ETH_FLAGS_CRC_ERROR) printf(" [CRC_ERROR]");
		if(flags & ICSNEOC2_MESSAGE_ETH_FLAGS_FRAME_TOO_SHORT) printf(" [FRAME_TOO_SHORT]");
		if(flags & ICSNEOC2_MESSAGE_ETH_FLAGS_TX_ABORTED) printf(" [TX_ABORTED]");
		if(flags & ICSNEOC2_MESSAGE_ETH_FLAGS_FCS_VERIFIED) printf(" [FCS_VERIFIED]");
		if(flags & ICSNEOC2_MESSAGE_ETH_FLAGS_PREEMPTION_ENABLED) printf(" [PREEMPTION]");
		if(flags & ICSNEOC2_MESSAGE_ETH_FLAGS_IS_T1S) printf(" [T1S]");
		printf("\n");
	}

	/* Print data bytes */
	uint8_t data[1600] = {0};
	res = icsneoc2_message_data_get(message, data, &data_length);
	if(res == icsneoc2_error_success) {
		printf("\t  Data:\n\t    ");
		for(size_t x = 0; x < data_length; x++) {
			printf("0x%02x ", data[x]);
			if((x + 1) % 20 == 0 && x + 1 < data_length) {
				printf("\n\t    ");
			}
		}
		printf("\n");
	}

	return icsneoc2_error_success;
}

int main(void) {
	/* Open the first available device */
	printf("Opening first available device...\n");
	icsneoc2_device_t* device = NULL;
	icsneoc2_error_t res = icsneoc2_device_open_first(0, icsneoc2_open_options_default, &device);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to open first device", res);
	}

	/* Get a description of the opened device */
	char description[255] = {0};
	size_t description_length = 255;
	res = icsneoc2_device_description_get(device, description, &description_length);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to get device description", res);
	}
	printf("Opened device: %s\n", description);

	/* Wait for Ethernet frames to arrive */
	const int duration_seconds = 10;
	printf("Listening for Ethernet frames for %d seconds...\n", duration_seconds);
	sleep_ms(duration_seconds * 1000);

	/* Retrieve and process messages */
	icsneoc2_message_t* messages[20000] = {0};
	size_t message_count = 20000;
	size_t eth_count = 0;

	for(size_t i = 0; i < message_count; ++i) {
		res = icsneoc2_device_message_get(device, &messages[i], 0);
		if(res != icsneoc2_error_success) {
			print_events();
			return print_error_code("Failed to get messages", res);
		}
		if(messages[i] == NULL) {
			message_count = i;
			break;
		}
	}

	printf("Got %zu messages total, filtering for Ethernet...\n", message_count);

	for(size_t i = 0; i < message_count; i++) {
		icsneoc2_message_t* message = messages[i];

		/* Check if this is a TX echo (skip it) */
		bool is_tx = false;
		res = icsneoc2_message_is_transmit(message, &is_tx);
		if(res != icsneoc2_error_success || is_tx) {
			continue;
		}

		/* Check if this is an Ethernet message */
		bool is_ethernet = false;
		res = icsneoc2_message_is_ethernet(message, &is_ethernet);
		if(res != icsneoc2_error_success || !is_ethernet) {
			continue;
		}

		process_ethernet_message(message, eth_count);
		eth_count++;
	}

	printf("Received %zu Ethernet frames out of %zu total messages\n", eth_count, message_count);

	/* Free all messages */
	for(size_t i = 0; i < message_count; ++i) {
		icsneoc2_message_free(messages[i]);
	}

	/* Print any events */
	print_events();

	/* Close the device */
	printf("Closing device...\n");
	res = icsneoc2_device_close(device);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to close device", res);
	}

	return 0;
}
