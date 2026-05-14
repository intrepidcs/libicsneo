#include <icsneo/icsneoc2.h>

#include <stdio.h>

static int print_error_code(const char* message, icsneoc2_error_t error) {
	char error_str[64] = {0};
	size_t error_str_len = sizeof(error_str);
	icsneoc2_error_t res = icsneoc2_error_code_get(error, error_str, &error_str_len);
	if(res != icsneoc2_error_success) {
		printf("%s: failed to get string for error code %u\n", message, error);
		return (int)res;
	}
	printf("%s: %s (%u)\n", message, error_str, error);
	return (int)error;
}

int main(void) {
	icsneoc2_supported_device_t* supported_devices = NULL;
	icsneoc2_error_t res = icsneoc2_supported_devices_enumerate(&supported_devices);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to enumerate supported devices", res);
	}

	if(!supported_devices) {
		printf("No supported devices found.\n");
		return 0;
	}

	printf("Supported devices:\n");
	for(icsneoc2_supported_device_t* cur = supported_devices; cur != NULL; cur = icsneoc2_supported_devices_next(cur)) {
		icsneoc2_devicetype_t device_type = 0;
		res = icsneoc2_supported_device_get(cur, &device_type);
		if(res != icsneoc2_error_success) {
			icsneoc2_supported_devices_free(supported_devices);
			return print_error_code("Failed to get supported device type", res);
		}

		char name[128] = {0};
		size_t name_len = sizeof(name);
		res = icsneoc2_device_type_name_get(device_type, name, &name_len);
		if(res != icsneoc2_error_success) {
			icsneoc2_supported_devices_free(supported_devices);
			return print_error_code("Failed to get supported device type name", res);
		}

		printf("  %s (%u)\n", name, (unsigned int)device_type);
	}

	res = icsneoc2_supported_devices_free(supported_devices);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to free supported device list", res);
	}

	return 0;
}
