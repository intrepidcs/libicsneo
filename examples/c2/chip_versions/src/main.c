#include <icsneo/icsneoc2.h>

#include <stdio.h>
#include <inttypes.h>

int print_error_code(const char* message, icsneoc2_error_t error) {
	char error_str[64];
	size_t error_str_len = sizeof(error_str);
	icsneoc2_error_t res = icsneoc2_error_code_get(error, error_str, &error_str_len);
	if(res != icsneoc2_error_success) {
		printf("%s: Failed to get string for error code %u with error code %u\n", message, error, res);
		return (int)res;
	}
	printf("%s: \"%s\" (%u)\n", message, error_str, error);
	return (int)error;
}

int main() {
	icsneoc2_error_t res;

	icsneoc2_device_t* device = NULL;
	res = icsneoc2_device_open_first(0, icsneoc2_open_options_default, &device);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to open first device", res);
	}

	char description[128] = {0};
	size_t description_length = sizeof(description);
	icsneoc2_device_description_get(device, description, &description_length);
	printf("Opened device: %s\n\n", description);

	size_t count = 0;
	icsneoc2_chip_versions_t* chip_versions = NULL;
	res = icsneoc2_device_chip_versions_enumerate(device, &chip_versions, true, &count);
	if(res != icsneoc2_error_success) {
		print_error_code("Failed to enumerate chip versions", res);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return 1;
	}

	printf("Found %zu chip version(s):\n", count);
	for(icsneoc2_chip_versions_t* cur = chip_versions; cur; cur = icsneoc2_chip_versions_next(cur)) {
		char name[64] = {0};
		size_t name_length = sizeof(name);
		uint8_t major = 0, minor = 0, maintenance = 0, build = 0;
		res = icsneoc2_chip_versions_props_get(cur, name, &name_length, &major, &minor, &maintenance, &build);
		if(res != icsneoc2_error_success) {
			print_error_code("Failed to get chip version properties", res);
			continue;
		}
		printf("  %s: %u.%u.%u.%u\n", name, major, minor, maintenance, build);
	}

	icsneoc2_chip_versions_free(chip_versions);
	icsneoc2_device_close(device);
	icsneoc2_device_free(device);
	return 0;
}
