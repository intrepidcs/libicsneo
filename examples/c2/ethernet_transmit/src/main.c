#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2messages.h>

#include <stdio.h>
#include <inttypes.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

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

	icsneoc2_netid_t tx_networks[255] = {0};
    size_t tx_net_count = sizeof(tx_networks) / sizeof(tx_networks[0]);
	res = icsneoc2_device_supported_tx_networks_get(device, tx_networks, &tx_net_count);
	if(res != icsneoc2_error_success) {
		print_events();
		return print_error_code("Failed to get TX networks", res);
	}

	/* Filter for Ethernet/AutomotiveEthernet networks */
	icsneoc2_netid_t eth_networks[64] = {0};
	char eth_names[64][128] = {{0}};
	size_t eth_count = 0;
	for(size_t i = 0; i < tx_net_count && eth_count < 64; i++) {
		/* Create a temporary message to check network type */
		icsneoc2_message_t* tmp = NULL;
		res = icsneoc2_message_eth_create(&tmp);
		if(res != icsneoc2_error_success) continue;
		res = icsneoc2_message_netid_set(tmp, tx_networks[i]);
		if(res != icsneoc2_error_success) { icsneoc2_message_free(tmp); continue; }
		icsneoc2_network_type_t ntype = 0;
		res = icsneoc2_message_network_type_get(tmp, &ntype);
		icsneoc2_message_free(tmp);
		if(res != icsneoc2_error_success) continue;
		if(ntype == icsneoc2_network_type_ethernet || ntype == icsneoc2_network_type_automotive_ethernet) {
			eth_networks[eth_count] = tx_networks[i];
			size_t name_len = 128;
			icsneoc2_netid_name_get(tx_networks[i], eth_names[eth_count], &name_len);
			eth_count++;
		}
	}

	if(eth_count == 0) {
		printf("No Ethernet TX networks available on this device.\n");
		icsneoc2_device_close(device);
		return 0;
	}

	/* Let the user pick */
	printf("Available Ethernet TX networks:\n");
	for(size_t i = 0; i < eth_count; i++) {
		printf("  %zu) %s\n", i + 1, eth_names[i]);
	}
	printf("Select network [1-%zu]: ", eth_count);
	int selection = 0;
	if(scanf("%d", &selection) != 1 || selection < 1 || (size_t)selection > eth_count) {
		printf("Invalid selection, using first available.\n");
		selection = 1;
	}
	icsneoc2_netid_t netid = eth_networks[selection - 1];
	printf("Selected: %s\n", eth_names[selection - 1]);
	/* Transmit Ethernet frames */
	const size_t msg_count = 10;
	printf("Transmitting %zu Ethernet frames on %s...\n", msg_count, eth_names[selection - 1]);

	for(size_t i = 0; i < msg_count; i++) {
		/* Create an Ethernet message */
		icsneoc2_message_t* message = NULL;
		res = icsneoc2_message_eth_create(&message);
		if(res != icsneoc2_error_success) {
			print_events();
			return print_error_code("Failed to create Ethernet message", res);
		}

		/* Set the network ID */
		res = icsneoc2_message_netid_set(message, netid );
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(message);
			print_events();
			return print_error_code("Failed to set netid", res);
		}

		/* Build Ethernet frame data:
		 * Bytes 0-5:   Destination MAC (00:FC:70:00:01:02)
		 * Bytes 6-11:  Source MAC      (00:FC:70:00:01:01)
		 * Bytes 12-13: EtherType       (0x0800 = IPv4)
		 * Bytes 14+:   Payload
		 */
		uint8_t frame_data[] = {
			0x00, 0xFC, 0x70, 0x00, 0x01, 0x02, /* Destination MAC */
			0x00, 0xFC, 0x70, 0x00, 0x01, 0x01, /* Source MAC */
			0x08, 0x00,                           /* EtherType (IPv4) */
			0x45, 0x00, 0x00, 0x20,               /* IPv4: ver/IHL, DSCP, total length (32) */
			0x00, 0x00, 0x00, 0x00,               /* Identification, flags/fragment offset */
			0x40, 0x11, 0x00, 0x00,               /* TTL (64), protocol (UDP), checksum (0) */
			0xC0, 0xA8, 0x01, 0x01,               /* Source IP (192.168.1.1) */
			0xC0, 0xA8, 0x01, 0x02,               /* Destination IP (192.168.1.2) */
			0xC3, 0x50, 0xC3, 0x51,               /* UDP: src port (50000), dst port (50001) */
			0x00, 0x0C, 0x00, 0x00,               /* UDP: length (12), checksum (0) */
			0x00, 0x00, 0x00, 0x00                /* UDP payload (4 bytes, frame counter) */
		};

		/* Put the frame counter in the UDP payload */
		frame_data[42] = (uint8_t)((i >> 24) & 0xFF);
		frame_data[43] = (uint8_t)((i >> 16) & 0xFF);
		frame_data[44] = (uint8_t)((i >> 8) & 0xFF);
		frame_data[45] = (uint8_t)(i & 0xFF);

		res = icsneoc2_message_data_set(message, frame_data, sizeof(frame_data));
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(message);
			print_events();
			return print_error_code("Failed to set frame data", res);
		}

		/* Transmit the message */
		res = icsneoc2_device_message_transmit(device, message);
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(message);
			print_events();
			return print_error_code("Failed to transmit Ethernet frame", res);
		}

		icsneoc2_message_free(message);
		printf("\tTransmitted frame %zu\n", i + 1);
	}

	printf("Successfully transmitted %zu Ethernet frames\n", msg_count);

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
