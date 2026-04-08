#include <icsneo/icsneoc2.h>

#include <stdio.h>

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
		icsneoc2_device_free(device);
		return print_error_code("Failed to get device description", res);
	}
	printf("Opened device: %s\n", description);

	/* Wait for the device to disconnect */
	printf("Waiting for device to disconnect (unplug device from PC)...\n");
	for(;;) {
		bool is_open = true;
		res = icsneoc2_device_is_open(device, &is_open);
		if(res != icsneoc2_error_success) {
			print_events();
			icsneoc2_device_free(device);
			return print_error_code("Failed to check open status", res);
		}
		if(!is_open) {
			printf("Device disconnected!\n");
			break;
		}
		sleep_ms(500);
	}

	/* Attempt to reconnect */
	uint32_t timeout_ms = 20000; // 20 second timeout
	printf("Attempting to reconnect (%u second timeout)...\n", timeout_ms / 1000);
	res = icsneoc2_device_reconnect(device, icsneoc2_open_options_default, timeout_ms);
	if(res != icsneoc2_error_success) {
		print_events();
		icsneoc2_device_free(device);
		return print_error_code("Failed to reconnect", res);
	}

	printf("Reconnected successfully!\n");

	/* Verify by getting the description again */
	description_length = 255;
	res = icsneoc2_device_description_get(device, description, &description_length);
	if(res == icsneoc2_error_success) {
		printf("Device: %s\n", description);
	}

	/* Print any events */
	print_events();

	/* Close the device */
	printf("Closing device...\n");
	res = icsneoc2_device_close(device);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_free(device);
		return print_error_code("Failed to close device", res);
	}
	icsneoc2_device_free(device);

	printf("Done.\n");
	return 0;
}
