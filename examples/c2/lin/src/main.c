/* Note: This example requires LIN 1 and LIN 2 channels to be connected on the device */

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

void print_events(const char* device_description) {
	icsneoc2_event_t* events[1024] = {0};
	size_t events_count = 1024;
	for(size_t i = 0; i < events_count; ++i) {
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

void process_lin_messages(icsneoc2_message_t** messages, size_t count) {
	for(size_t i = 0; i < count; i++) {
		bool is_lin = false;
		icsneoc2_error_t res = icsneoc2_message_is_lin(messages[i], &is_lin);
		if(res != icsneoc2_error_success || !is_lin)
			continue;

		uint8_t id = 0;
		uint8_t protected_id = 0;
		uint8_t checksum = 0;
		icsneoc2_lin_msg_type_t msg_type = 0;
		bool is_enhanced_checksum = false;
		res = icsneoc2_message_lin_props_get(messages[i], &id, &protected_id, &checksum, &msg_type, &is_enhanced_checksum);
		if(res != icsneoc2_error_success) {
			print_error_code("\tFailed to get LIN properties", res);
			continue;
		}

		icsneoc2_netid_t netid = 0;
		icsneoc2_message_netid_get(messages[i], &netid);
		char netid_name[128] = {0};
		size_t netid_name_length = 128;
		icsneoc2_netid_name_get(netid, netid_name, &netid_name_length);

		uint8_t data[64] = {0};
		size_t data_length = 64;
		icsneoc2_message_data_get(messages[i], data, &data_length);

		icsneoc2_lin_err_flags_t err_flags = 0;
		icsneoc2_message_lin_err_flags_get(messages[i], &err_flags);

		printf("\t%s RX | ID: 0x%02x | Protected ID: 0x%02x\n", netid_name, id, protected_id);
		printf("\tData: [");
		for(size_t x = 0; x < data_length; x++) {
			printf(" 0x%02x", data[x]);
		}
		printf(" ]\n");
		printf("\tChecksum type: %s\n", is_enhanced_checksum ? "Enhanced" : "Classic");
		printf("\tChecksum: 0x%02x\n", checksum);
		printf("\tChecksum valid: %s\n\n", (!(err_flags & ICSNEOC2_LIN_ERR_CHECKSUM_MATCH)) ? "yes" : "no");
	}
}

int main() {
	/* Open the first available device */
	printf("Opening first available device...\n");
	icsneoc2_device_t* device = NULL;
	icsneoc2_error_t res = icsneoc2_device_open_first(0, icsneoc2_open_options_default, &device);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to open first device", res);
	}

	char description[255] = {0};
	size_t description_length = 255;
	res = icsneoc2_device_description_get(device, description, &description_length);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_close(device);
		return print_error_code("\tFailed to get device description", res);
	}
	printf("\tOpened device: %s\n\n", description);

	/* Configure LIN settings */
	int64_t baud = 19200;

	printf("Enable LIN 01 commander resistor... ");
	res = icsneoc2_settings_commander_resistor_set(device, icsneoc2_netid_lin_01, true);
	printf("%s\n", res == icsneoc2_error_success ? "OK" : "FAIL");

	printf("Disable LIN 02 commander resistor... ");
	res = icsneoc2_settings_commander_resistor_set(device, icsneoc2_netid_lin_02, false);
	printf("%s\n", res == icsneoc2_error_success ? "OK" : "FAIL");

	printf("Setting LIN 01 baudrate to %" PRId64 " bit/s... ", baud);
	res = icsneoc2_settings_baudrate_set(device, icsneoc2_netid_lin_01, baud);
	printf("%s\n", res == icsneoc2_error_success ? "OK" : "FAIL");

	printf("Setting LIN 02 baudrate to %" PRId64 " bit/s... ", baud);
	res = icsneoc2_settings_baudrate_set(device, icsneoc2_netid_lin_02, baud);
	printf("%s\n", res == icsneoc2_error_success ? "OK" : "FAIL");

	printf("Setting LIN 01 mode to NORMAL... ");
	res = icsneoc2_settings_lin_mode_set(device, icsneoc2_netid_lin_01, icsneoc2_lin_mode_normal);
	printf("%s\n", res == icsneoc2_error_success ? "OK" : "FAIL");

	printf("Setting LIN 02 mode to NORMAL... ");
	res = icsneoc2_settings_lin_mode_set(device, icsneoc2_netid_lin_02, icsneoc2_lin_mode_normal);
	printf("%s\n", res == icsneoc2_error_success ? "OK" : "FAIL");

	printf("Applying settings... ");
	res = icsneoc2_settings_apply(device);
	if(res != icsneoc2_error_success) {
		print_events(description);
		icsneoc2_device_close(device);
		return print_error_code("\tFailed to apply settings", res);
	}
	printf("OK\n");

	printf("Getting LIN 01 baudrate... ");
	int64_t read_baud = 0;
	res = icsneoc2_settings_baudrate_get(device, icsneoc2_netid_lin_01, &read_baud);
	if(res == icsneoc2_error_success)
		printf("OK, %" PRId64 " bit/s\n", read_baud);
	else
		printf("FAIL\n");

	printf("Getting LIN 02 baudrate... ");
	res = icsneoc2_settings_baudrate_get(device, icsneoc2_netid_lin_02, &read_baud);
	if(res == icsneoc2_error_success)
		printf("OK, %" PRId64 " bit/s\n\n", read_baud);
	else
		printf("FAIL\n\n");

	/* Transmit a LIN responder data update on LIN 02 */
	printf("Transmitting a LIN 02 responder data frame... ");
	{
		icsneoc2_message_t* msg = NULL;
		res = icsneoc2_message_lin_create(&msg, 0x11);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to create LIN message", res);
		}
		icsneoc2_lin_msg_type_t msg_type = icsneoc2_lin_msg_type_update_responder;
		res = icsneoc2_message_lin_props_set(msg, NULL, NULL, &msg_type, NULL);
		uint8_t data[] = {0xaa, 0xbb, 0xcc, 0xdd, 0x11, 0x22, 0x33, 0x44};
		res += icsneoc2_message_data_set(msg, data, sizeof(data));
		res += icsneoc2_message_netid_set(msg, icsneoc2_netid_lin_02);
        res += icsneoc2_message_lin_calc_checksum(msg);
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(msg);
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to set LIN message properties", res);
		}
		res = icsneoc2_device_message_transmit(device, msg);
		icsneoc2_message_free(msg);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to transmit LIN responder message", res);
		}
		printf("OK\n");
	}

	/* Transmit a LIN commander header on LIN 01 */
	printf("Transmitting a LIN 01 commander header... ");
	{
		icsneoc2_message_t* msg = NULL;
		res = icsneoc2_message_lin_create(&msg, 0x11);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to create LIN message", res);
		}
		icsneoc2_lin_msg_type_t msg_type = icsneoc2_lin_msg_type_header_only;
		res = icsneoc2_message_lin_props_set(msg, NULL, NULL, &msg_type, NULL);
		res += icsneoc2_message_netid_set(msg, icsneoc2_netid_lin_01);
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(msg);
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to set LIN message properties", res);
		}
		res = icsneoc2_device_message_transmit(device, msg);
		icsneoc2_message_free(msg);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to transmit LIN header", res);
		}
		printf("OK\n\n");
	}

	sleep_ms(100);

	/* Transmit a LIN commander message with data on LIN 01 */
	printf("Transmitting a LIN 01 commander frame with data... ");
	{
		icsneoc2_message_t* msg = NULL;
		res = icsneoc2_message_lin_create(&msg, 0x22);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to create LIN message", res);
		}
		icsneoc2_lin_msg_type_t msg_type = icsneoc2_lin_msg_type_commander_msg;
		bool enhanced = true;
		res = icsneoc2_message_lin_props_set(msg, NULL, NULL, &msg_type, &enhanced);
		uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0xaa, 0xbb, 0xcc, 0xdd};
		res += icsneoc2_message_data_set(msg, data, sizeof(data));
		res += icsneoc2_message_netid_set(msg, icsneoc2_netid_lin_01);
        res += icsneoc2_message_lin_calc_checksum(msg);
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(msg);
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to set LIN message properties", res);
		}
		res = icsneoc2_device_message_transmit(device, msg);
		icsneoc2_message_free(msg);
		if(res != icsneoc2_error_success) {
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to transmit LIN commander message", res);
		}
		printf("OK\n\n");
	}

	sleep_ms(100);

	/* Read back any received messages and display LIN frames */
	icsneoc2_message_t* messages[2048] = {0};
	size_t message_count = 2048;
	printf("Getting messages...\n");
	for(size_t i = 0; i < message_count; ++i) {
		res = icsneoc2_device_message_get(device, &messages[i], 0);
		if(res != icsneoc2_error_success) {
			for(size_t j = 0; j < i; ++j) {
				icsneoc2_message_free(messages[j]);
			}
			print_events(description);
			icsneoc2_device_close(device);
			return print_error_code("\tFailed to get messages", res);
		}
		if(messages[i] == NULL) {
			message_count = i;
			break;
		}
	}
	printf("\tReceived %zu messages\n", message_count);
	process_lin_messages(messages, message_count);
	for(size_t i = 0; i < message_count; ++i) {
		icsneoc2_message_free(messages[i]);
	}

	/* Cleanup */
	print_events(description);
	printf("Closing device... ");
	res = icsneoc2_device_close(device);
	printf("%s\n", res == icsneoc2_error_success ? "OK" : "FAIL");

	return 0;
}
