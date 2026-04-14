#include <icsneo/icsneoc2.h>

#include <stdio.h>
#include <inttypes.h>

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

		char serial[32] = {0};
		size_t serial_len = sizeof(serial);
		icsneoc2_device_info_serial_get(cur, serial, &serial_len);

		printf("  [%d] %s (Serial: %s)\n", device_count + 1, desc, serial);
		device_count++;
	}

	int device_choice;
	printf("Select device (1-%d): ", device_count);
	if(scanf("%d", &device_choice) != 1 || device_choice < 1 || device_choice > device_count) {
		printf("Invalid selection.\n");
		icsneoc2_enumeration_free(found_devices);
		return 1;
	}

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
	printf("\nOpened device: %s\n\n", description);

	/* ===== Serial Number ===== */
	char serial[32] = {0};
	size_t serial_len = sizeof(serial);
	res = icsneoc2_device_serial_get(device, serial, &serial_len);
	if(res == icsneoc2_error_success) {
		printf("Serial:     %s\n", serial);
	} else {
		print_error_code("Failed to get serial", res);
	}

	/* ===== PCB Serial Number ===== */
	uint8_t pcbsn[16] = {0};
	size_t pcbsn_len = sizeof(pcbsn);
	res = icsneoc2_device_pcb_serial_get(device, pcbsn, &pcbsn_len);
	if(res == icsneoc2_error_success) {
		printf("PCB Serial: ");
		for(size_t i = 0; i < pcbsn_len; i++) {
			printf("%c", pcbsn[i]);
		}
		printf("\n");
	} else {
		print_error_code("Failed to get PCB serial (device may not support it)", res);
	}

	/* ===== MAC Address ===== */
	uint8_t mac[6] = {0};
	size_t mac_len = sizeof(mac);
	res = icsneoc2_device_mac_address_get(device, mac, &mac_len);
	if(res == icsneoc2_error_success) {
		printf("MAC:        ");
		for(size_t i = 0; i < mac_len; i++) {
			if(i > 0) printf(":");
			printf("%02X", mac[i]);
		}
		printf("\n");
	} else {
		print_error_code("Failed to get MAC address (device may not support it)", res);
	}

	/* Cleanup */
	printf("\nClosing device... ");
	res = icsneoc2_device_close(device);
	printf("%s\n", res == icsneoc2_error_success ? "OK" : "FAIL");

	icsneoc2_device_free(device);
	return 0;
}
