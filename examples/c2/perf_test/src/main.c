/*
 * PerfTest setting example.
 *
 * Opens the first available device, enables the device-global PerfTest mode
 * via icsneoc2_settings_perf_test_enabled_set(), reads messages for a few
 * seconds while PerfTest is active, then disables PerfTest again. Each step
 * reads the value back with icsneoc2_settings_perf_test_enabled_get() to
 * confirm the change took effect.
 */

#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2settings.h>
#include <icsneo/icsneoc2messages.h>

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

static int print_error_code(const char* message, icsneoc2_error_t error) {
	char error_str[64] = {0};
	size_t error_str_len = sizeof(error_str);
	icsneoc2_error_t res = icsneoc2_error_code_get(error, error_str, &error_str_len);
	if(res != icsneoc2_error_success) {
		fprintf(stderr, "%s: failed to get string for error code %u\n", message, error);
		return (int)res;
	}
	fprintf(stderr, "%s: \"%s\" (%u)\n", message, error_str, error);
	return (int)error;
}

/* Sets PerfTest, applies it to the device, then reads it back to confirm. */
static icsneoc2_error_t set_and_verify_perf_test(icsneoc2_device_t* device, bool enable) {
	icsneoc2_error_t res = icsneoc2_settings_perf_test_enabled_set(device, enable);
	if(res != icsneoc2_error_success)
		return res;

	res = icsneoc2_settings_apply(device);
	if(res != icsneoc2_error_success)
		return res;

	/* Re-read settings from the device so the read-back reflects what was
	 * actually persisted, not just the local settings buffer. */
	res = icsneoc2_settings_refresh(device);
	if(res != icsneoc2_error_success)
		return res;

	bool value = !enable;
	res = icsneoc2_settings_perf_test_enabled_get(device, &value);
	if(res != icsneoc2_error_success)
		return res;

	printf("\tPerfTest now reads back as: %s\n", value ? "enabled" : "disabled");
	if(value != enable) {
		fprintf(stderr, "\tERROR: expected PerfTest %s but device reports %s\n",
			enable ? "enabled" : "disabled", value ? "enabled" : "disabled");
		return icsneoc2_error_get_settings_failure;
	}
	return icsneoc2_error_success;
}

/* Continuously drains and counts messages for the given wall-clock duration. */
static icsneoc2_error_t read_messages_for(icsneoc2_device_t* device, unsigned seconds) {
	size_t total = 0;
	printf("\tReading messages for %u seconds...\n", seconds);
	time_t start = time(NULL);
	while((time_t)(time(NULL) - start) < (time_t)seconds) {
		/* Short timeout so an idle bus still lets us re-check the clock,
		 * while a flooding bus is drained as fast as messages arrive. */
		icsneoc2_message_t* message = NULL;
		icsneoc2_error_t res = icsneoc2_device_message_get(device, &message, 50);
		if(res != icsneoc2_error_success)
			return res;
		if(message == NULL)
			continue;
		++total;
		icsneoc2_message_free(message);
	}
	printf("\tReceived %zu messages in %u seconds.\n", total, seconds);
	return icsneoc2_error_success;
}

int main(void) {
	printf("Opening first available device...\n");
	icsneoc2_device_t* device = NULL;
	icsneoc2_error_t res = icsneoc2_device_open_first(0, icsneoc2_open_options_default, &device);
	if(res != icsneoc2_error_success)
		return print_error_code("\tFailed to open first device", res);

	char description[255] = {0};
	size_t description_length = sizeof(description);
	res = icsneoc2_device_description_get(device, description, &description_length);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_free(device);
		return print_error_code("\tFailed to get device description", res);
	}
	printf("\tOpened device: %s\n", description);

	/* Pull the current settings down from the device before reading/modifying them. */
	res = icsneoc2_settings_refresh(device);
	if(res != icsneoc2_error_success)
		goto cleanup;

	bool initial = false;
	res = icsneoc2_settings_perf_test_enabled_get(device, &initial);
	if(res != icsneoc2_error_success) {
		print_error_code("\tFailed to read initial PerfTest state (device may not support it)", res);
		goto cleanup;
	}
	printf("\tInitial PerfTest state: %s\n", initial ? "enabled" : "disabled");

	printf("Enabling PerfTest...\n");
	res = set_and_verify_perf_test(device, true);
	if(res != icsneoc2_error_success) {
		print_error_code("\tFailed to enable PerfTest", res);
		goto cleanup;
	}

	res = read_messages_for(device, 3);
	if(res != icsneoc2_error_success) {
		print_error_code("\tFailed while reading messages", res);
		goto cleanup;
	}

	printf("Disabling PerfTest...\n");
	res = set_and_verify_perf_test(device, false);
	if(res != icsneoc2_error_success) {
		print_error_code("\tFailed to disable PerfTest", res);
		goto cleanup;
	}

	printf("PerfTest enable/read/disable cycle completed successfully.\n");

cleanup:
	icsneoc2_device_close(device);
	icsneoc2_device_free(device);
	return (int)res;
}
