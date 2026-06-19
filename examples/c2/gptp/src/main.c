/*
 * gPTP settings configurator example.
 *
 * Lists connected devices and their gPTP support, then lets the user
 * configure the gPTP profile, role, port, and clock syntonization on any
 * device that supports it. If --serial is omitted, the first available
 * gPTP-capable device is used.
 */

#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2settings.h>
#include <icsneo/icsneoc2messages.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	const char* serial;
	bool list;
} args_t;

/* Maps an icsneoc2_netid_t to its gPTP port index (0 = not a gPTP port). */
static uint8_t netid_to_gptp_port(icsneoc2_netid_t netid) {
	switch(netid) {
		case icsneoc2_netid_ae_01: return 1;
		case icsneoc2_netid_ae_02: return 2;
		case icsneoc2_netid_ae_03: return 3;
		case icsneoc2_netid_ae_04: return 4;
		case icsneoc2_netid_ae_05: return 5;
		case icsneoc2_netid_ae_06: return 6;
		case icsneoc2_netid_ae_07: return 7;
		case icsneoc2_netid_ae_08: return 8;
		case icsneoc2_netid_ae_09: return 9;
		case icsneoc2_netid_ae_10: return 10;
		case icsneoc2_netid_ae_11: return 11;
		case icsneoc2_netid_ae_12: return 12;
		case icsneoc2_netid_ethernet_01: return 13;
		case icsneoc2_netid_ethernet_02: return 14;
		case icsneoc2_netid_ethernet_03: return 15;
		case icsneoc2_netid_ae_13: return 16;
		case icsneoc2_netid_ae_14: return 17;
		case icsneoc2_netid_ae_15: return 18;
		case icsneoc2_netid_ae_16: return 19;
		default: return 0;
	}
}

static const char* gptp_profile_str(icsneoc2_gptp_profile_t p) {
	switch(p) {
		case icsneoc2_gptp_profile_standard:   return "Standard";
		case icsneoc2_gptp_profile_automotive: return "Automotive";
		default:                               return "Unknown";
	}
}

static const char* gptp_role_str(icsneoc2_gptp_role_t r) {
	switch(r) {
		case icsneoc2_gptp_role_disabled: return "Disabled";
		case icsneoc2_gptp_role_passive:  return "Passive";
		case icsneoc2_gptp_role_master:   return "Master";
		case icsneoc2_gptp_role_slave:    return "Slave";
		default:                          return "Unknown";
	}
}

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

static void print_usage(const char* prog) {
	printf("Usage:\n");
	printf("  %s --list\n", prog);
	printf("  %s [--serial SERIAL]\n", prog);
	printf("\n");
	printf("  --list            List connected devices and their gPTP support.\n");
	printf("  --serial SERIAL   Serial number of the device to configure.\n");
	printf("  -h, --help        Show this message.\n");
}

static int parse_args(int argc, char** argv, args_t* out) {
	memset(out, 0, sizeof(*out));
	for(int i = 1; i < argc; ++i) {
		const char* a = argv[i];
		if(strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
			print_usage(argv[0]);
			exit(0);
		} else if(strcmp(a, "--list") == 0) {
			out->list = true;
		} else if(strcmp(a, "--serial") == 0) {
			if(i + 1 >= argc) {
				fprintf(stderr, "error: --serial requires a value\n");
				return 1;
			}
			out->serial = argv[++i];
		} else {
			fprintf(stderr, "error: unknown argument '%s'\n", a);
			print_usage(argv[0]);
			return 1;
		}
	}
	return 0;
}

static icsneoc2_device_info_t* find_device(icsneoc2_device_info_t* list, const char* serial) {
	for(icsneoc2_device_info_t* cur = list; cur != NULL; cur = icsneoc2_device_info_next(cur)) {
		if(serial == NULL)
			return cur;
		char dev_serial[64] = {0};
		size_t dev_serial_len = sizeof(dev_serial);
		if(icsneoc2_device_info_serial_get(cur, dev_serial, &dev_serial_len) != icsneoc2_error_success)
			continue;
		if(strcmp(dev_serial, serial) == 0)
			return cur;
	}
	return NULL;
}

/*
 * Print a list of available Ethernet/AE networks with their gPTP port indices.
 * Returns the number of ports printed.
 */
static size_t print_gptp_ports(icsneoc2_device_t* device) {
	icsneoc2_netid_t nets[128] = {0};
	size_t count = sizeof(nets) / sizeof(nets[0]);
	if(icsneoc2_device_supported_tx_networks_get(device, nets, &count) != icsneoc2_error_success)
		return 0;

	size_t printed = 0;
	for(size_t i = 0; i < count; ++i) {
		uint8_t port = netid_to_gptp_port(nets[i]);
		if(port == 0)
			continue;
		char name[64] = {0};
		size_t name_len = sizeof(name);
		icsneoc2_netid_name_get(nets[i], name, &name_len);
		printf("    [%u] %s\n", (unsigned)port, name);
		++printed;
	}
	return printed;
}

static int list_devices(icsneoc2_device_info_t* list) {
	size_t index = 0;
	for(icsneoc2_device_info_t* cur = list; cur != NULL; cur = icsneoc2_device_info_next(cur)) {
		char serial[64] = {0};
		size_t serial_len = sizeof(serial);
		char description[256] = {0};
		size_t description_len = sizeof(description);
		icsneoc2_device_info_serial_get(cur, serial, &serial_len);
		icsneoc2_device_info_description_get(cur, description, &description_len);
		printf("[%zu] %s (%s)\n", index++, description, serial);

		icsneoc2_device_t* device = NULL;
		if(icsneoc2_device_create(cur, &device) != icsneoc2_error_success)
			continue;

		icsneoc2_open_options_t opts = icsneoc2_open_options_default;
		opts &= ~ICSNEOC2_OPEN_OPTIONS_SYNC_RTC;
		opts &= ~ICSNEOC2_OPEN_OPTIONS_GO_ONLINE;
		if(icsneoc2_device_open(device, opts) != icsneoc2_error_success) {
			icsneoc2_device_free(device);
			continue;
		}

		bool supported = false;
		icsneoc2_device_supports_gptp(device, &supported);
		printf("    gPTP supported: %s\n", supported ? "yes" : "no");
		if(supported) {
			printf("    gPTP ports:\n");
			print_gptp_ports(device);
		}

		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
	}
	return 0;
}

static void print_current_settings(icsneoc2_device_t* device) {
	icsneoc2_gptp_profile_t profile = icsneoc2_gptp_profile_standard;
	icsneoc2_gptp_role_t role = icsneoc2_gptp_role_disabled;
	uint8_t port = 0;
	bool clock_syntonization = false;

	printf("\nCurrent gPTP settings:\n");
	if(icsneoc2_settings_gptp_profile_get(device, &profile) == icsneoc2_error_success)
		printf("  Profile:             %s\n", gptp_profile_str(profile));
	if(icsneoc2_settings_gptp_role_get(device, &role) == icsneoc2_error_success)
		printf("  Role:                %s\n", gptp_role_str(role));
	if(icsneoc2_settings_gptp_enabled_port_get(device, &port) == icsneoc2_error_success)
		printf("  Enabled port:        %u%s\n", (unsigned)port, port == 0 ? " (disabled)" : "");
	if(icsneoc2_settings_gptp_clock_syntonization_enabled_get(device, &clock_syntonization) == icsneoc2_error_success)
		printf("  Clock syntonization: %s\n", clock_syntonization ? "enabled" : "disabled");
}

static long prompt_long(const char* prompt, long default_val, long min, long max) {
	char buf[32] = {0};
	printf("%s [%ld]: ", prompt, default_val);
	fflush(stdout);
	if(fgets(buf, sizeof(buf), stdin) == NULL || buf[0] == '\n')
		return default_val;
	char* end = NULL;
	long val = strtol(buf, &end, 10);
	if(end == buf || val < min || val > max) {
		printf("Invalid input, using %ld.\n", default_val);
		return default_val;
	}
	return val;
}

static int configure_device(icsneoc2_device_t* device) {
	icsneoc2_error_t res;

	res = icsneoc2_settings_refresh(device);
	if(res != icsneoc2_error_success)
		return print_error_code("Failed to refresh settings", res);

	print_current_settings(device);

	printf("\nAvailable gPTP ports (0 = disabled):\n");
	printf("    [0] Disabled\n");
	print_gptp_ports(device);

	printf("\nConfigure gPTP:\n");

	long profile = prompt_long("  Profile (0=Standard, 1=Automotive)", 1, 0, 1);
	res = icsneoc2_settings_gptp_profile_set(device, (icsneoc2_gptp_profile_t)profile);
	if(res != icsneoc2_error_success)
		return print_error_code("Failed to set profile", res);

	long role = prompt_long("  Role (0=Disabled, 1=Passive, 2=Master, 3=Slave)", 3, 0, 3);
	res = icsneoc2_settings_gptp_role_set(device, (icsneoc2_gptp_role_t)role);
	if(res != icsneoc2_error_success)
		return print_error_code("Failed to set role", res);

	long port = prompt_long("  Enabled port index", 0, 0, 19);
	res = icsneoc2_settings_gptp_enabled_port_set(device, (uint8_t)port);
	if(res != icsneoc2_error_success)
		return print_error_code("Failed to set enabled port", res);

	long syntonization = prompt_long("  Clock syntonization (0=disabled, 1=enabled)", 0, 0, 1);
	res = icsneoc2_settings_gptp_clock_syntonization_enabled_set(device, syntonization != 0);
	if(res != icsneoc2_error_success)
		return print_error_code("Failed to set clock syntonization", res);

	printf("\nNote: icsneoc2_settings_apply() persists settings on the device.\n");
	res = icsneoc2_settings_apply(device);
	if(res != icsneoc2_error_success)
		return print_error_code("Failed to apply settings", res);

	printf("\nUpdated gPTP settings:\n");
	print_current_settings(device);
	return 0;
}

int main(int argc, char** argv) {
	args_t args;
	if(parse_args(argc, argv, &args) != 0)
		return 1;

	icsneoc2_device_info_t* found_devices = NULL;
	icsneoc2_error_t res = icsneoc2_device_enumerate(0, &found_devices);
	if(res != icsneoc2_error_success)
		return print_error_code("Failed to enumerate devices", res);
	if(found_devices == NULL) {
		fprintf(stderr, "error: no devices found\n");
		return 1;
	}

	if(args.list) {
		int rc = list_devices(found_devices);
		icsneoc2_enumeration_free(found_devices);
		return rc;
	}

	icsneoc2_device_info_t* info = find_device(found_devices, args.serial);
	if(info == NULL) {
		fprintf(stderr, "error: unable to find device %s\n", args.serial ? args.serial : "(any)");
		icsneoc2_enumeration_free(found_devices);
		return 1;
	}

	char description[256] = {0};
	size_t description_len = sizeof(description);
	icsneoc2_device_info_description_get(info, description, &description_len);

	icsneoc2_device_t* device = NULL;
	res = icsneoc2_device_create(info, &device);
	if(res != icsneoc2_error_success) {
		icsneoc2_enumeration_free(found_devices);
		return print_error_code("Failed to create device", res);
	}

	printf("Opening %s\n", description);
	res = icsneoc2_device_open(device, icsneoc2_open_options_default);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_free(device);
		icsneoc2_enumeration_free(found_devices);
		return print_error_code("Failed to open device", res);
	}

	bool supported = false;
	icsneoc2_device_supports_gptp(device, &supported);
	if(!supported) {
		fprintf(stderr, "error: device does not support gPTP (%s)\n", description);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		icsneoc2_enumeration_free(found_devices);
		return 1;
	}

	int rc = configure_device(device);

	printf("Closing %s\n", description);
	icsneoc2_device_close(device);
	icsneoc2_device_free(device);
	icsneoc2_enumeration_free(found_devices);
	return rc;
}
