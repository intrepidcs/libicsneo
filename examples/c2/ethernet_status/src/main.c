#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2messages.h>

#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void sleep_ms(uint32_t ms) {
#ifdef _WIN32
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

int print_error_code(const char* message, icsneoc2_error_t error) {
	char error_str[64] = {0};
	size_t error_str_len = sizeof(error_str);
	icsneoc2_error_t res = icsneoc2_error_code_get(error, error_str, &error_str_len);
	if(res != icsneoc2_error_success) {
		printf("%s: Failed to get string for error code %u with error code %u\n", message, error, res);
		return (int)res;
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
		size_t description_length = sizeof(description);
		icsneoc2_error_t res = icsneoc2_event_description_get(events[i], description, &description_length);
		if(res != icsneoc2_error_success) {
			(void)print_error_code("\tFailed to get event description", res);
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

const char* link_speed_name(icsneoc2_link_speed_t speed) {
	switch(speed) {
	case icsneoc2_link_speed_auto:
		return "Auto";
	case icsneoc2_link_speed_10mbps:
		return "10 Mbps";
	case icsneoc2_link_speed_100mbps:
		return "100 Mbps";
	case icsneoc2_link_speed_1000mbps:
		return "1000 Mbps";
	case icsneoc2_link_speed_2500mbps:
		return "2500 Mbps";
	case icsneoc2_link_speed_5000mbps:
		return "5000 Mbps";
	case icsneoc2_link_speed_10000mbps:
		return "10000 Mbps";
	default:
		return "Unknown";
	}
}

const char* link_mode_name(icsneoc2_link_mode_t mode) {
	switch(mode) {
	case icsneoc2_link_mode_auto:
		return "Auto";
	case icsneoc2_link_mode_master:
		return "Master";
	case icsneoc2_link_mode_slave:
		return "Slave";
	case icsneoc2_link_mode_invalid:
		return "Invalid";
	case icsneoc2_link_mode_none:
		return "None";
	default:
		return "Unknown";
	}
}

int print_ethernet_status_message(icsneoc2_message_t* message, size_t index) {
	icsneoc2_netid_t netid = 0;
	char netid_name[128] = {0};
	size_t netid_name_length = sizeof(netid_name);
	bool link_state = false;
	bool duplex = false;
	icsneoc2_link_speed_t link_speed = icsneoc2_link_speed_auto;
	icsneoc2_link_mode_t link_mode = icsneoc2_link_mode_auto;

	icsneoc2_error_t res = icsneoc2_message_netid_get(message, &netid);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to get Ethernet status netid", res);
	}

	res = icsneoc2_netid_name_get(netid, netid_name, &netid_name_length);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to get Ethernet status netid name", res);
	}

	res = icsneoc2_message_eth_status_props_get(message, &link_state, &duplex, &link_speed, &link_mode);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to get Ethernet status properties", res);
	}

	printf("\t%zu) Ethernet status on %s (0x%x)\n", index, netid_name, netid);
	printf("\t   Link: %s\n", link_state ? "Up" : "Down");
	printf("\t   Duplex: %s\n", duplex ? "Full" : "Half");
	printf("\t   Speed: %s (%u)\n", link_speed_name(link_speed), link_speed);
	printf("\t   Mode: %s (%u)\n", link_mode_name(link_mode), link_mode);

	return icsneoc2_error_success;
}

int main(void) {
	printf("Opening first available device offline...\n");
	icsneoc2_device_t* device = NULL;
	icsneoc2_open_options_t options = icsneoc2_open_options_default & ~ICSNEOC2_OPEN_OPTIONS_GO_ONLINE;
	icsneoc2_error_t res = icsneoc2_device_open_first(0, options, &device);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to open first device", res);
	}

	char description[255] = {0};
	size_t description_length = sizeof(description);
	res = icsneoc2_device_description_get(device, description, &description_length);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return print_error_code("Failed to get device description", res);
	}
	printf("Opened device: %s\n", description);

	printf("Going online to trigger Ethernet status broadcast...\n");
	res = icsneoc2_device_go_online(device, true);
	if(res != icsneoc2_error_success) {
		print_events();
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return print_error_code("Failed to go online", res);
	}

	const int listen_seconds = 6;
	time_t start_time = time(NULL);
	size_t status_count = 0;
	size_t total_count = 0;
	printf("Listening for Ethernet status messages for %d seconds...\n", listen_seconds);

	while(time(NULL) - start_time < listen_seconds) {
		icsneoc2_message_t* message = NULL;
		res = icsneoc2_device_message_get(device, &message, 100);
		if(res != icsneoc2_error_success) {
			print_events();
			icsneoc2_device_close(device);
			icsneoc2_device_free(device);
			return print_error_code("Failed to get message", res);
		}
		if(message == NULL) {
			sleep_ms(10);
			continue;
		}

		total_count++;
		bool is_ethernet_status = false;
		res = icsneoc2_message_is_ethernet_status(message, &is_ethernet_status);
		if(res != icsneoc2_error_success) {
			icsneoc2_message_free(message);
			print_events();
			icsneoc2_device_close(device);
			icsneoc2_device_free(device);
			return print_error_code("Failed to check for Ethernet status message", res);
		}

		if(is_ethernet_status) {
			res = print_ethernet_status_message(message, status_count);
			if(res != icsneoc2_error_success) {
				icsneoc2_message_free(message);
				print_events();
				icsneoc2_device_close(device);
				icsneoc2_device_free(device);
				return (int)res;
			}
			status_count++;
		}

		icsneoc2_message_free(message);
	}

	printf("Received %zu Ethernet status messages out of %zu total messages.\n", status_count, total_count);
	if(status_count == 0) {
		printf("No Ethernet status messages were seen. These messages are broadcast when the device goes online and may depend on device Ethernet support.\n");
	}

	print_events();

	printf("Closing device...\n");
	res = icsneoc2_device_close(device);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_free(device);
		return print_error_code("Failed to close device", res);
	}

	icsneoc2_device_free(device);
	return 0;
}
