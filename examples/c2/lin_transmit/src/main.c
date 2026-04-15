#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2messages.h>
#include <icsneo/icsneoc2settings.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

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

int read_int(const char* prompt, int min_val, int max_val) {
	int value;
	while(1) {
		printf("%s", prompt);
		if(scanf("%d", &value) != 1) {
			/* Clear invalid input */
			int c;
			while((c = getchar()) != '\n' && c != EOF) {}
			printf("Invalid input, try again.\n");
			continue;
		}
		if(value < min_val || value > max_val) {
			printf("Please enter a value between %d and %d.\n", min_val, max_val);
			continue;
		}
		return value;
	}
}

int read_hex(const char* prompt, int min_val, int max_val) {
	int value;
	while(1) {
		printf("%s", prompt);
		if(scanf("%x", &value) != 1) {
			/* Clear invalid input */
			int c;
			while((c = getchar()) != '\n' && c != EOF) {}
			printf("Invalid input, try again.\n");
			continue;
		}
		if(value < min_val || value > max_val) {
			printf("Please enter a value between 0x%X and 0x%X.\n", min_val, max_val);
			continue;
		}
		return value;
	}
}

int main() {
	icsneoc2_error_t res;

	/* ===== Device Selection ===== */
	printf("Searching for devices...\n");
	icsneoc2_device_info_t* found_devices = NULL;
	res = icsneoc2_device_enumerate(0, &found_devices);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to enumerate devices", res);
	}
	if(!found_devices) {
		printf("No devices found.\n");
		return 1;
	}

	/* Count and display devices */
	int device_count = 0;
	for(icsneoc2_device_info_t* cur = found_devices; cur; cur = icsneoc2_device_info_next(cur)) {
		char desc[128] = {0};
		size_t desc_len = sizeof(desc);
		icsneoc2_device_info_description_get(cur, desc, &desc_len);
		printf("  [%d] %s\n", device_count + 1, desc);
		device_count++;
	}

	int device_choice = read_int("Select device: ", 1, device_count);

	/* Find the selected device_info node */
	icsneoc2_device_info_t* selected_info = found_devices;
	for(int i = 1; i < device_choice; i++) {
		selected_info = icsneoc2_device_info_next(selected_info);
	}

	/* Open the selected device */
	icsneoc2_device_t* device = NULL;
	res = icsneoc2_device_create(selected_info, &device);
	if(res != icsneoc2_error_success) {
		icsneoc2_enumeration_free(found_devices);
		return print_error_code("Failed to create device from device info", res);
	}
	res = icsneoc2_device_open(device, icsneoc2_open_options_default);
	icsneoc2_enumeration_free(found_devices);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_free(device);
		return print_error_code("Failed to open device", res);
	}

	char description[128] = {0};
	size_t description_length = sizeof(description);
	icsneoc2_device_description_get(device, description, &description_length);
	printf("Opened device: %s\n\n", description);

	/* ===== LIN Network Selection ===== */
	/* Get all supported TX networks */
	size_t tx_net_count = 0;
	res = icsneoc2_device_supported_tx_networks_get(device, NULL, &tx_net_count);
	if(res != icsneoc2_error_success || tx_net_count == 0) {
		printf("No supported TX networks.\n");
		icsneoc2_device_close(device);
		return 1;
	}

	icsneoc2_netid_t* tx_networks = (icsneoc2_netid_t*)malloc(tx_net_count * sizeof(icsneoc2_netid_t));
	if(!tx_networks) {
		printf("Out of memory.\n");
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return 1;
	}
	res = icsneoc2_device_supported_tx_networks_get(device, tx_networks, &tx_net_count);
	if(res != icsneoc2_error_success) {
		free(tx_networks);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return print_error_code("Failed to get TX networks", res);
	}

	/* Filter for LIN networks */
	icsneoc2_netid_t lin_networks[64] = {0};
	char lin_names[64][128] = {{0}};
	size_t lin_count = 0;
	for(size_t i = 0; i < tx_net_count && lin_count < 64; i++) {
		icsneoc2_message_t* tmp = NULL;
		res = icsneoc2_message_lin_create(&tmp, 0);
		if(res != icsneoc2_error_success) continue;
		res = icsneoc2_message_netid_set(tmp, tx_networks[i]);
		if(res != icsneoc2_error_success) { icsneoc2_message_free(tmp); continue; }
		icsneoc2_network_type_t ntype = 0;
		res = icsneoc2_message_network_type_get(tmp, &ntype);
		icsneoc2_message_free(tmp);
		if(res != icsneoc2_error_success) continue;
		if(ntype == icsneoc2_network_type_lin) {
			lin_networks[lin_count] = tx_networks[i];
			size_t name_len = 128;
			icsneoc2_netid_name_get(tx_networks[i], lin_names[lin_count], &name_len);
			lin_count++;
		}
	}
	free(tx_networks);

	if(lin_count == 0) {
		printf("No LIN networks available on this device.\n");
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return 1;
	}

	printf("Available LIN networks:\n");
	for(size_t i = 0; i < lin_count; i++) {
		printf("  [%zu] %s\n", i + 1, lin_names[i]);
	}

	int lin_choice = read_int("Select LIN network: ", 1, (int)lin_count);
	icsneoc2_netid_t selected_netid = lin_networks[lin_choice - 1];
	printf("Selected: %s\n\n", lin_names[lin_choice - 1]);

	/* ===== Commander / Responder Selection ===== */
	printf("Message type:\n");
	printf("  [1] Commander frame\n");
	printf("  [2] Responder frame (update responder + header only)\n");
	int type_choice = read_int("Select message type: ", 1, 2);
	bool is_commander = (type_choice == 1);
	printf("Selected: %s\n\n", is_commander ? "Commander" : "Responder");

	uint8_t id_choice = (uint8_t)read_hex("Select LIN ID (0x00-0x3F): ", 0, 0x3F);
	printf("Selected: 0x%02X\n\n", id_choice);

	/* ===== Configure LIN ===== */
	printf("Configuring %s... ", lin_names[lin_choice - 1]);
	res = icsneoc2_settings_commander_resistor_set(device, selected_netid, is_commander);
	res += icsneoc2_settings_baudrate_set(device, selected_netid, 19200);
	res += icsneoc2_settings_lin_mode_set(device, selected_netid, icsneoc2_lin_mode_normal);
	res += icsneoc2_settings_apply(device);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return print_error_code("Failed to configure LIN", res);
	}
	printf("OK\n\n");

	/* ===== Transmit Loop ===== */
	printf("Transmitting on %s every second for 10 seconds...\n", lin_names[lin_choice - 1]);
	uint8_t counter = 0;
	for(int i = 0; i < 10; i++) {
		icsneoc2_message_t* msg = NULL;
		res = icsneoc2_message_lin_create(&msg, id_choice);
		if(res != icsneoc2_error_success) {
			print_error_code("\tFailed to create LIN message", res);
			break;
		}

		uint8_t data[] = {counter, counter + 1, counter + 2, counter + 3, 0xAA, 0xBB, 0xCC, 0xDD};
		bool enhanced = true;

		if(is_commander) {
			/* Commander: send header-only frame to poll the bus */
			icsneoc2_lin_msg_type_t msg_type = icsneoc2_lin_msg_type_header_only;
			res = icsneoc2_message_lin_props_set(msg, NULL, NULL, &msg_type, &enhanced);
			res += icsneoc2_message_netid_set(msg, selected_netid);
			if(res != icsneoc2_error_success) {
				icsneoc2_message_free(msg);
				print_error_code("\tFailed to set commander properties", res);
				break;
			}
			res = icsneoc2_device_message_transmit(device, msg);
			icsneoc2_message_free(msg);
			if(res != icsneoc2_error_success) {
				print_error_code("\tFailed to transmit header", res);
				break;
			}
		} else {
			/* Responder: update the responder table with new data */
			icsneoc2_lin_msg_type_t msg_type = icsneoc2_lin_msg_type_update_responder;
			res = icsneoc2_message_lin_props_set(msg, NULL, NULL, &msg_type, &enhanced);
			res += icsneoc2_message_data_set(msg, data, sizeof(data));
			res += icsneoc2_message_lin_calc_checksum(msg);
			res += icsneoc2_message_netid_set(msg, selected_netid);
			if(res != icsneoc2_error_success) {
				icsneoc2_message_free(msg);
				print_error_code("\tFailed to update responder", res);
				break;
			}
			res = icsneoc2_device_message_transmit(device, msg);
			icsneoc2_message_free(msg);
			if(res != icsneoc2_error_success) {
				print_error_code("\tFailed to transmit responder update", res);
				break;
			}
		}
		printf("[%2d/10] Transmitted %s msg ID=0x%02X, counter=%u\n",
			i + 1, is_commander ? "commander" : "responder", id_choice, counter);
		counter += 4;
		sleep_ms(1000);
	}

	/* Cleanup */
	printf("\nClosing device... ");
	res = icsneoc2_device_close(device);
	printf("%s\n", res == icsneoc2_error_success ? "OK" : "FAIL");
	icsneoc2_device_free(device);

	return 0;
}
