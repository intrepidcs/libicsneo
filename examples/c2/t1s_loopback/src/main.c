#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2messages.h>
#include <icsneo/icsneoc2settings.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define TX_LOCAL_ID 0u
#define RX_LOCAL_ID 1u
#define T1S_MAX_NODES 8u
#define T1S_TX_OPP_TIMER 20u
#define T1S_BURST_TIMER 64u
#define T1S_MAX_BURST 128u
#define LOOPBACK_ETHER_TYPE 0x9000u
#define LOOPBACK_FRAME_SIZE 60u

typedef struct selectable_network {
	icsneoc2_netid_t netid;
	char name[64];
} selectable_network_t;

/* Sleep for a short period while waiting for the device to apply settings. */
static void sleep_ms(uint32_t ms);

/* Print a readable error string and return the same failure code to the caller. */
static int print_error_code(const char* message, icsneoc2_error_t error);

/* Drain and print queued library events when the example encounters an error. */
static void print_events(void);

/* Convert a netid to a readable name such as "AE 02". */
static int get_netid_name(icsneoc2_netid_t netid, char* buffer, size_t buffer_size);

/* Gather the device's TX and RX networks, keeping only automotive Ethernet ports. */
static int get_available_networks(const icsneoc2_device_t* device,
		selectable_network_t* tx_networks,
		size_t* tx_count,
		selectable_network_t* rx_networks,
		size_t* rx_count);

/* Prompt the user to choose one TX or RX network from the filtered list. */
static int prompt_for_network_selection(const char* label, const selectable_network_t* networks, size_t count, icsneoc2_netid_t* selected_netid, char* selected_name, size_t selected_name_size);

/* Apply the small set of T1S settings needed for this two-port loopback example. */
static int configure_t1s_port(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t local_id);

/* Print a MAC address in a compact human-readable form. */
static void print_mac(const char* label, const uint8_t* mac);

/* Print payload bytes as hex for the TX echo and RX frame output. */
static void print_payload_hex(const uint8_t* data, size_t length);

/* Build one recognizable Ethernet frame that both transmit and receive paths share. */
static void build_loopback_frame(uint8_t* frame_data, size_t frame_size, const char* tx_name, const char* rx_name);

/* Check whether a received Ethernet frame matches the loopback frame this example sent. */
static int message_matches_loopback_frame(icsneoc2_message_t* message, const uint8_t* expected, size_t expected_length, bool* matches);

/* Print the key details of an Ethernet message found during the loopback test. */
static int print_ethernet_message(icsneoc2_message_t* message, const char* direction_label);

/* Transmit the loopback Ethernet frame on the selected TX port. */
static int transmit_loopback_frame(icsneoc2_device_t* device, icsneoc2_netid_t tx_netid, const char* tx_name, const char* rx_name);

/* Poll until the example sees both the TX echo and the matching RX frame. */
static int poll_for_loopback_messages(icsneoc2_device_t* device,
		icsneoc2_netid_t tx_netid,
		icsneoc2_netid_t rx_netid,
		const char* tx_name,
		const char* rx_name,
		const uint8_t* expected_frame,
		size_t expected_frame_length);

int main(void) {
	icsneoc2_device_t* device = NULL;
	icsneoc2_error_t res;
	char description[255] = {0};
	char serial[64] = {0};
	size_t description_length = sizeof(description);
	size_t serial_length = sizeof(serial);
	selectable_network_t available_tx_networks[128] = {0};
	selectable_network_t available_rx_networks[128] = {0};
	size_t available_tx_count = 0;
	size_t available_rx_count = 0;
	icsneoc2_netid_t tx_netid = 0;
	icsneoc2_netid_t rx_netid = 0;
	uint8_t expected_frame[LOOPBACK_FRAME_SIZE] = {0};
	char tx_name[64] = {0};
	char rx_name[64] = {0};
	int status = 1;

	printf("RAD-Comet3 C2 T1S loopback example\n");
	printf("Opening first available RAD-Comet3...\n");

	res = icsneoc2_device_open_first(icsneoc2_devicetype_rad_comet3, icsneoc2_open_options_default, &device);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to open a RAD-Comet3", res);
	}

	res = icsneoc2_device_description_get(device, description, &description_length);
	if(res != icsneoc2_error_success) {
		print_error_code("Failed to get device description", res);
		goto cleanup;
	}
	res = icsneoc2_device_serial_get(device, serial, &serial_length);
	if(res != icsneoc2_error_success) {
		print_error_code("Failed to get device serial", res);
		goto cleanup;
	}
	printf("Opened device: %s [%s]\n", description, serial);

	if(get_available_networks(device, available_tx_networks, &available_tx_count, available_rx_networks, &available_rx_count) != 0) {
		goto cleanup;
	}

	if(prompt_for_network_selection("TX", available_tx_networks, available_tx_count, &tx_netid, tx_name, sizeof(tx_name)) != 0) {
		goto cleanup;
	}
	if(prompt_for_network_selection("RX", available_rx_networks, available_rx_count, &rx_netid, rx_name, sizeof(rx_name)) != 0) {
		goto cleanup;
	}

	printf("Selected loopback wiring: %s connected to %s\n", tx_name, rx_name);

	if(tx_netid == rx_netid) {
		printf("TX and RX networks are the same. This example is intended for a physical loopback between two ports.\n");
		goto cleanup;
	}

	// Use the same expected frame bytes for transmit and for receive-side matching.
	build_loopback_frame(expected_frame, sizeof(expected_frame), tx_name, rx_name);

	res = icsneoc2_settings_refresh(device);
	if(res != icsneoc2_error_success) {
		print_error_code("Failed to refresh device settings", res);
		goto cleanup;
	}

	if(configure_t1s_port(device, tx_netid, TX_LOCAL_ID) != 0) {
		goto cleanup;
	}
	if(configure_t1s_port(device, rx_netid, RX_LOCAL_ID) != 0) {
		goto cleanup;
	}

	printf("Applying T1S settings to the device.\n");
	printf("Note: icsneoc2_settings_apply() persists these settings on the device.\n");
	res = icsneoc2_settings_apply(device);
	if(res != icsneoc2_error_success) {
		print_error_code("Failed to apply T1S settings", res);
		goto cleanup;
	}

	sleep_ms(500);

	if(transmit_loopback_frame(device, tx_netid, tx_name, rx_name) != 0) {
		goto cleanup;
	}

	if(poll_for_loopback_messages(device, tx_netid, rx_netid, tx_name, rx_name, expected_frame, sizeof(expected_frame)) != 0) {
		print_events();
		goto cleanup;
	}

	status = 0;

cleanup:
	if(device != NULL) {
		res = icsneoc2_device_close(device);
		if(res != icsneoc2_error_success) {
			(void)print_error_code("Failed to close device", res);
		}
		res = icsneoc2_device_free(device);
		if(res != icsneoc2_error_success) {
			(void)print_error_code("Failed to free device", res);
		}
	}

	return status;
}

static void sleep_ms(uint32_t ms) {
#ifdef _WIN32
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

static int print_error_code(const char* message, icsneoc2_error_t error) {
	char error_str[64] = {0};
	size_t error_str_len = sizeof(error_str);
	icsneoc2_error_t res = icsneoc2_error_code_get(error, error_str, &error_str_len);
	if(res != icsneoc2_error_success) {
		printf("%s: failed to get string for error code %u with error code %u\n", message, error, res);
		return (int)res;
	}
	printf("%s: \"%s\" (%u)\n", message, error_str, error);
	return (int)error;
}

static void print_events(void) {
	icsneoc2_event_t* events[64] = {0};
	size_t count = sizeof(events) / sizeof(events[0]);

	for(size_t i = 0; i < count; ++i) {
		icsneoc2_error_t res = icsneoc2_event_get(&events[i], NULL);
		if(res != icsneoc2_error_success) {
			(void)print_error_code("Failed to get events", res);
			return;
		}
		if(events[i] == NULL) {
			count = i;
			break;
		}
	}

	for(size_t i = 0; i < count; ++i) {
		char description[255] = {0};
		size_t description_length = sizeof(description);
		icsneoc2_error_t res = icsneoc2_event_description_get(events[i], description, &description_length);
		if(res == icsneoc2_error_success) {
			printf("Event %zu: %s\n", i, description);
		}
		icsneoc2_event_free(events[i]);
	}
}

static int get_netid_name(icsneoc2_netid_t netid, char* buffer, size_t buffer_size) {
	size_t length = buffer_size;
	icsneoc2_error_t res = icsneoc2_netid_name_get(netid, buffer, &length);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to get netid name", res);
	}
	return 0;
}

static int get_available_networks(const icsneoc2_device_t* device,
		selectable_network_t* tx_networks,
		size_t* tx_count,
		selectable_network_t* rx_networks,
		size_t* rx_count) {
	icsneoc2_netid_t supported_tx_networks[128] = {0};
	icsneoc2_netid_t supported_rx_networks[128] = {0};
	size_t tx_supported_count = sizeof(supported_tx_networks) / sizeof(supported_tx_networks[0]);
	size_t rx_supported_count = sizeof(supported_rx_networks) / sizeof(supported_rx_networks[0]);
	icsneoc2_message_t* probe = NULL;
	icsneoc2_network_type_t network_type = 0;
	icsneoc2_error_t res = icsneoc2_device_supported_tx_networks_get(device, supported_tx_networks, &tx_supported_count);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to get supported TX networks", res);
	}
	res = icsneoc2_device_supported_rx_networks_get(device, supported_rx_networks, &rx_supported_count);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to get supported RX networks", res);
	}

	*tx_count = 0;
	*rx_count = 0;

	// Reuse one temporary Ethernet message to classify each netid by network type.
	res = icsneoc2_message_eth_create(&probe);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to create temporary Ethernet message", res);
	}

	for(size_t i = 0; i < tx_supported_count; ++i) {
		res = icsneoc2_message_netid_set(probe, supported_tx_networks[i]);
		if(res != icsneoc2_error_success) {
			continue;
		}
		res = icsneoc2_message_network_type_get(probe, &network_type);
		if(res != icsneoc2_error_success || network_type != icsneoc2_network_type_automotive_ethernet) {
			continue;
		}
		tx_networks[*tx_count].netid = supported_tx_networks[i];
		if(get_netid_name(supported_tx_networks[i], tx_networks[*tx_count].name, sizeof(tx_networks[*tx_count].name)) != 0) {
			icsneoc2_message_free(probe);
			return 1;
		}
		(*tx_count)++;
	}

	for(size_t i = 0; i < rx_supported_count; ++i) {
		res = icsneoc2_message_netid_set(probe, supported_rx_networks[i]);
		if(res != icsneoc2_error_success) {
			continue;
		}
		res = icsneoc2_message_network_type_get(probe, &network_type);
		if(res != icsneoc2_error_success || network_type != icsneoc2_network_type_automotive_ethernet) {
			continue;
		}
		rx_networks[*rx_count].netid = supported_rx_networks[i];
		if(get_netid_name(supported_rx_networks[i], rx_networks[*rx_count].name, sizeof(rx_networks[*rx_count].name)) != 0) {
			icsneoc2_message_free(probe);
			return 1;
		}
		(*rx_count)++;
	}
	icsneoc2_message_free(probe);

	if(*tx_count == 0) {
		printf("No automotive Ethernet TX networks are available on this device.\n");
		return 1;
	}
	if(*rx_count == 0) {
		printf("No automotive Ethernet RX networks are available on this device.\n");
		return 1;
	}
	return 0;
}

static int prompt_for_network_selection(const char* label, const selectable_network_t* networks, size_t count, icsneoc2_netid_t* selected_netid, char* selected_name, size_t selected_name_size) {
	char input[32] = {0};
	char* end_ptr = NULL;
	long selected_index = 0;

	if(!label || !networks || count == 0 || !selected_netid || !selected_name || selected_name_size == 0) {
		return print_error_code("Invalid network selection parameters", icsneoc2_error_invalid_parameters);
	}

	printf("Available automotive Ethernet %s networks:\n", label);
	for(size_t i = 0; i < count; ++i) {
		printf("  %zu) %s\n", i + 1, networks[i].name);
	}
	printf("Select %s network [1-%zu, default 1]: ", label, count);

	if(fgets(input, sizeof(input), stdin) == NULL || input[0] == '\n') {
		selected_index = 1;
	} else {
		selected_index = strtol(input, &end_ptr, 10);
		if(end_ptr == input || selected_index < 1 || (size_t)selected_index > count) {
			printf("Invalid selection, using %s.\n", networks[0].name);
			selected_index = 1;
		}
	}

	*selected_netid = networks[selected_index - 1].netid;
	strncpy(selected_name, networks[selected_index - 1].name, selected_name_size - 1);
	selected_name[selected_name_size - 1] = '\0';
	return 0;
}

static int configure_t1s_port(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint8_t local_id) {
	char netid_name[64] = {0};
	icsneoc2_error_t res;
	bool termination = false;

	if(get_netid_name(netid, netid_name, sizeof(netid_name)) != 0) {
		return 1;
	}

	printf("Configuring %s: PLCA on, LocalID=%u, MaxNodes=%u, TxOpp=%u, BurstTimer=%u, MaxBurst=%u\n",
		netid_name,
		(unsigned)local_id,
		(unsigned)T1S_MAX_NODES,
		(unsigned)T1S_TX_OPP_TIMER,
		(unsigned)T1S_BURST_TIMER,
		(unsigned)T1S_MAX_BURST);

	// Keep the example explicit about the small set of T1S settings needed for loopback.
	res = icsneoc2_settings_t1s_plca_enabled_for_set(device, netid, true);
	if(res != icsneoc2_error_success) return print_error_code("Failed to enable T1S PLCA", res);
	res = icsneoc2_settings_t1s_local_id_set(device, netid, local_id);
	if(res != icsneoc2_error_success) return print_error_code("Failed to set T1S local ID", res);
	res = icsneoc2_settings_t1s_max_nodes_set(device, netid, T1S_MAX_NODES);
	if(res != icsneoc2_error_success) return print_error_code("Failed to set T1S max nodes", res);
	res = icsneoc2_settings_t1s_tx_opp_timer_set(device, netid, T1S_TX_OPP_TIMER);
	if(res != icsneoc2_error_success) return print_error_code("Failed to set T1S TX opportunity timer", res);
	res = icsneoc2_settings_t1s_burst_timer_set(device, netid, T1S_BURST_TIMER);
	if(res != icsneoc2_error_success) return print_error_code("Failed to set T1S burst timer", res);
	res = icsneoc2_settings_t1s_max_burst_timer_for_set(device, netid, T1S_MAX_BURST);
	if(res != icsneoc2_error_success) return print_error_code("Failed to set T1S max burst", res);

	res = icsneoc2_settings_t1s_is_termination_enabled_for(device, netid, &termination);
	if(res == icsneoc2_error_success) {
		res = icsneoc2_settings_t1s_termination_for_set(device, netid, true);
		if(res != icsneoc2_error_success) return print_error_code("Failed to enable T1S termination", res);
	} else if(res != icsneoc2_error_get_settings_failure) {
		return print_error_code("Failed to query T1S termination support", res);
	}

	return 0;
}

static void print_mac(const char* label, const uint8_t* mac) {
	printf("%s %02x:%02x:%02x:%02x:%02x:%02x", label, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void print_payload_hex(const uint8_t* data, size_t length) {
	for(size_t i = 0; i < length; ++i) {
		printf("%02x", data[i]);
		if(i + 1 < length) {
			printf(" ");
		}
	}
	printf("\n");
}

static void build_loopback_frame(uint8_t* frame_data, size_t frame_size, const char* tx_name, const char* rx_name) {
	const char* tx_label = tx_name ? tx_name : "TX";
	const char* rx_label = rx_name ? rx_name : "RX";

	// Build one recognizable Ethernet frame so the receive side can match exactly what we sent.
	memset(frame_data, 0, frame_size);
	frame_data[0] = 0x00;
	frame_data[1] = 0xFC;
	frame_data[2] = 0x70;
	frame_data[3] = 0x00;
	frame_data[4] = 0x00;
	frame_data[5] = 0x02;
	frame_data[6] = 0x00;
	frame_data[7] = 0xFC;
	frame_data[8] = 0x70;
	frame_data[9] = 0x00;
	frame_data[10] = 0x00;
	frame_data[11] = 0x01;
	frame_data[12] = (uint8_t)((LOOPBACK_ETHER_TYPE >> 8) & 0xFF);
	frame_data[13] = (uint8_t)(LOOPBACK_ETHER_TYPE & 0xFF);
	snprintf((char*)&frame_data[14], frame_size - 14, "C2 T1S loopback %s->%s", tx_label, rx_label);
}

static int message_matches_loopback_frame(icsneoc2_message_t* message, const uint8_t* expected, size_t expected_length, bool* matches) {
	uint8_t data[1600] = {0};
	size_t data_length = sizeof(data);
	uint16_t ether_type = 0;
	icsneoc2_error_t res;

	if(!expected || !matches || expected_length < 14) {
		return print_error_code("Invalid loopback match output", icsneoc2_error_invalid_parameters);
	}

	*matches = false;

	res = icsneoc2_message_eth_ether_type_get(message, &ether_type);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to read loopback EtherType", res);
	}
	if(ether_type != LOOPBACK_ETHER_TYPE) {
		return 0;
	}

	res = icsneoc2_message_data_get(message, data, &data_length);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to read loopback frame data", res);
	}

	if(data_length < 14) {
		return 0;
	}

	if(data_length >= expected_length) {
		*matches = memcmp(data, expected, expected_length) == 0;
	} else {
		*matches = memcmp(data, expected, data_length) == 0;
	}
	return 0;
}

static int print_ethernet_message(icsneoc2_message_t* message, const char* direction_label) {
	icsneoc2_netid_t netid = 0;
	char netid_name[64] = {0};
	uint8_t dst_mac[6] = {0};
	uint8_t src_mac[6] = {0};
	uint16_t ether_type = 0;
	icsneoc2_message_eth_t1s_flags_t t1s_flags = 0;
	uint8_t node_id = 0;
	uint8_t burst_count = 0;
	uint8_t symbol_type = 0;
	uint8_t data[1600] = {0};
	size_t data_length = sizeof(data);
	icsneoc2_error_t res;

	res = icsneoc2_message_netid_get(message, &netid);
	if(res != icsneoc2_error_success) return print_error_code("Failed to get message netid", res);
	if(get_netid_name(netid, netid_name, sizeof(netid_name)) != 0) return 1;

	res = icsneoc2_message_data_get(message, data, &data_length);
	if(res != icsneoc2_error_success) return print_error_code("Failed to get Ethernet data", res);

	printf("%s on %s: %zu bytes\n", direction_label, netid_name, data_length);

	res = icsneoc2_message_eth_mac_get(message, dst_mac, src_mac);
	if(res == icsneoc2_error_success) {
		print_mac("  Dst", dst_mac);
		printf("  ");
		print_mac("Src", src_mac);
		printf("\n");
	}

	res = icsneoc2_message_eth_ether_type_get(message, &ether_type);
	if(res == icsneoc2_error_success) {
		printf("  EtherType: 0x%04x\n", ether_type);
	}

	res = icsneoc2_message_eth_t1s_props_get(message, &t1s_flags, &node_id, &burst_count, &symbol_type);
	if(res == icsneoc2_error_success && (t1s_flags != 0 || node_id != 0 || burst_count != 0 || symbol_type != 0)) {
		printf("  T1S: node=%u burst=%u symbol=%u", (unsigned)node_id, (unsigned)burst_count, (unsigned)symbol_type);
		if(t1s_flags & ICSNEOC2_MESSAGE_ETH_T1S_FLAGS_IS_T1S_SYMBOL) printf(" [SYMBOL]");
		if(t1s_flags & ICSNEOC2_MESSAGE_ETH_T1S_FLAGS_IS_T1S_BURST) printf(" [BURST]");
		if(t1s_flags & ICSNEOC2_MESSAGE_ETH_T1S_FLAGS_TX_COLLISION) printf(" [TX_COLLISION]");
		if(t1s_flags & ICSNEOC2_MESSAGE_ETH_T1S_FLAGS_IS_T1S_WAKE) printf(" [WAKE]");
		printf("\n");
	}

	printf("  Payload bytes: ");
	if(data_length > 14) {
		print_payload_hex(&data[14], data_length - 14);
	} else {
		printf("<none>\n");
	}

	return 0;
}

static int transmit_loopback_frame(icsneoc2_device_t* device, icsneoc2_netid_t tx_netid, const char* tx_name, const char* rx_name) {
	icsneoc2_message_t* message = NULL;
	icsneoc2_error_t res;
	uint8_t frame_data[LOOPBACK_FRAME_SIZE] = {0};

	build_loopback_frame(frame_data, sizeof(frame_data), tx_name, rx_name);

	res = icsneoc2_message_eth_create(&message);
	if(res != icsneoc2_error_success) return print_error_code("Failed to create Ethernet message", res);

	res = icsneoc2_message_netid_set(message, tx_netid);
	if(res != icsneoc2_error_success) {
		icsneoc2_message_free(message);
		return print_error_code("Failed to set Ethernet netid", res);
	}

	res = icsneoc2_message_data_set(message, frame_data, sizeof(frame_data));
	if(res != icsneoc2_error_success) {
		icsneoc2_message_free(message);
		return print_error_code("Failed to set Ethernet payload", res);
	}

	printf("Transmitting one T1S loopback frame on %s...\n", tx_name ? tx_name : "selected TX network");
	res = icsneoc2_device_message_transmit(device, message);
	icsneoc2_message_free(message);
	if(res != icsneoc2_error_success) return print_error_code("Failed to transmit loopback frame", res);

	return 0;
}

static int poll_for_loopback_messages(icsneoc2_device_t* device,
		icsneoc2_netid_t tx_netid,
		icsneoc2_netid_t rx_netid,
		const char* tx_name,
		const char* rx_name,
		const uint8_t* expected_frame,
		size_t expected_frame_length) {
	bool saw_tx_echo = false;
	bool saw_rx_frame = false;

	printf("Polling for TX echo on %s and RX frame on %s...\n", tx_name, rx_name);
	for(size_t attempt = 0; attempt < 60 && !(saw_tx_echo && saw_rx_frame); ++attempt) {
		icsneoc2_message_t* message = NULL;
		icsneoc2_error_t res = icsneoc2_device_message_get(device, &message, 100);
		if(res != icsneoc2_error_success) {
			return print_error_code("Failed while polling for loopback messages", res);
		}
		if(message == NULL) {
			continue;
		}

		bool is_ethernet = false;
		bool matches_loopback = false;
		res = icsneoc2_message_is_ethernet(message, &is_ethernet);
		if(res != icsneoc2_error_success || !is_ethernet) {
			icsneoc2_message_free(message);
			continue;
		}

		icsneoc2_netid_t netid = 0;
		res = icsneoc2_message_netid_get(message, &netid);
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(message);
			return print_error_code("Failed to get polled netid", res);
		}

		if(netid != tx_netid && netid != rx_netid) {
			icsneoc2_message_free(message);
			continue;
		}

		// Ignore unrelated traffic on the selected ports and only count the frame this example transmitted.
		if(message_matches_loopback_frame(message, expected_frame, expected_frame_length, &matches_loopback) != 0) {
			icsneoc2_message_free(message);
			return 1;
		}
		if(!matches_loopback) {
			icsneoc2_message_free(message);
			continue;
		}

		bool is_tx = false;
		res = icsneoc2_message_is_transmit(message, &is_tx);
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(message);
			return print_error_code("Failed to determine TX status", res);
		}

		if(netid == tx_netid && is_tx && !saw_tx_echo) {
			if(print_ethernet_message(message, "TX echo") != 0) {
				icsneoc2_message_free(message);
				return 1;
			}
			saw_tx_echo = true;
		} else if(netid == rx_netid && !saw_rx_frame) {
			if(print_ethernet_message(message, "RX frame") != 0) {
				icsneoc2_message_free(message);
				return 1;
			}
			if(is_tx) {
				printf("  Note: RX port message was also marked as transmit.\n");
			}
			saw_rx_frame = true;
		}

		icsneoc2_message_free(message);
	}

	if(!saw_tx_echo || !saw_rx_frame) {
		printf("Loopback incomplete: saw_tx_echo=%s, saw_rx_frame=%s\n",
			saw_tx_echo ? "true" : "false",
			saw_rx_frame ? "true" : "false");
		printf("Confirm %s is physically connected to %s and both ports are configured for 10BASE-T1S.\n", tx_name, rx_name);
		return 1;
	}

	printf("Loopback complete: TX echo on %s and RX frame on %s were both observed.\n", tx_name, rx_name);
	return 0;
}