#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2messages.h>
#include <icsneo/icsneoc2settings.h>

#include <stdio.h>
#include <stdlib.h>

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
	icsneoc2_termination_group_t* groups = NULL;
	res = icsneoc2_settings_termination_groups_enumerate(device, &groups, &count);
	if(res != icsneoc2_error_success) {
		print_error_code("Failed to enumerate termination groups", res);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return 1;
	}

	if(count == 0) {
		printf("This device does not support software switchable termination.\n");
	} else {
		printf("Found %zu termination group(s) (only one network per group may be terminated at a time):\n", count);
	}

	size_t group_index = 0;
	for(icsneoc2_termination_group_t* group = groups; group; group = icsneoc2_termination_group_next(group), group_index++) {
		// First call with a NULL buffer to learn how many networks are in this group.
		size_t network_count = 0;
		icsneoc2_termination_group_networks_get(group, NULL, &network_count);

		icsneoc2_netid_t* netids = (icsneoc2_netid_t*)malloc(network_count * sizeof(icsneoc2_netid_t));
		if(!netids) {
			print_error_code("Out of memory", icsneoc2_error_out_of_memory);
			break;
		}

		// Second call to fill the buffer.
		res = icsneoc2_termination_group_networks_get(group, netids, &network_count);
		if(res != icsneoc2_error_success) {
			print_error_code("Failed to get termination group networks", res);
			free(netids);
			continue;
		}

		printf("  Group %zu:", group_index);
		for(size_t i = 0; i < network_count; i++) {
			char name[64] = {0};
			size_t name_length = sizeof(name);
			if(icsneoc2_netid_name_get(netids[i], name, &name_length) == icsneoc2_error_success) {
				printf(" %s", name);
			} else {
				printf(" %u", netids[i]);
			}
			if(i + 1 < network_count) {
				printf(",");
			}
		}
		printf("\n");
		free(netids);
	}

	icsneoc2_settings_termination_groups_free(groups);
	icsneoc2_device_close(device);
	icsneoc2_device_free(device);
	return 0;
}
