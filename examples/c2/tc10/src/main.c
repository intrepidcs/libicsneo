/*
 * TC10 example.
 *
 * Sends TC10 wake/sleep requests, queries TC10 status, or lists connected
 * devices and the Automotive Ethernet networks they support. Loosely based
 * on examples/python/tc10/tc10.py. If --serial is omitted, the first
 * available device is used.
 */

#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2messages.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	const char* serial;        /* may be NULL */
	const char** networks;     /* points into argv */
	size_t networks_count;
	bool send_wake;
	bool send_sleep;
	bool status;
	bool list;
} args_t;

static int print_error_code(const char* message, icsneoc2_error_t error);
static void str_tolower(char* s);
static bool resolve_netid(const char* name, icsneoc2_netid_t* out);
static void print_usage(const char* prog);
static int parse_args(int argc, char** argv, args_t* out);
static icsneoc2_device_info_t* find_device(icsneoc2_device_info_t* list, const char* serial);
static int list_devices(icsneoc2_device_info_t* list);
static const char* tc10_wake_status_str(icsneoc2_tc10_wake_status_t s);
static const char* tc10_sleep_status_str(icsneoc2_tc10_sleep_status_t s);

int main(int argc, char** argv) {
	args_t args;
	if(parse_args(argc, argv, &args) != 0) {
		return 1;
	}

	icsneoc2_device_info_t* found_devices = NULL;
	icsneoc2_error_t res = icsneoc2_device_enumerate(0, &found_devices);
	if(res != icsneoc2_error_success) {
		return print_error_code("Failed to enumerate devices", res);
	}
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
	res = icsneoc2_device_info_description_get(info, description, &description_len);
	if(res != icsneoc2_error_success) {
		icsneoc2_enumeration_free(found_devices);
		return print_error_code("Failed to get device description", res);
	}

	icsneoc2_device_t* device = NULL;
	res = icsneoc2_device_create(info, &device);
	if(res != icsneoc2_error_success) {
		icsneoc2_enumeration_free(found_devices);
		return print_error_code("Failed to create device", res);
	}

	printf("Opening device %s\n", description);
	res = icsneoc2_device_open(device, icsneoc2_open_options_default);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_free(device);
		icsneoc2_enumeration_free(found_devices);
		return print_error_code("Failed to open device", res);
	}

	bool supports_tc10 = false;
	res = icsneoc2_device_supports_tc10(device, &supports_tc10);
	if(res != icsneoc2_error_success || !supports_tc10) {
		fprintf(stderr, "error: device does not support TC10 (%s)\n", description);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		icsneoc2_enumeration_free(found_devices);
		return 1;
	}

	int rc = 0;
	for(size_t i = 0; i < args.networks_count; ++i) {
		const char* name = args.networks[i];
		icsneoc2_netid_t netid = icsneoc2_netid_invalid;
		if(!resolve_netid(name, &netid)) {
			fprintf(stderr, "error: unknown network '%s'\n", name);
			rc = 1;
			break;
		}

		if(args.send_wake) {
			printf("requesting TC10 wake on network %s\n", name);
			res = icsneoc2_device_tc10_wake_request(device, netid);
			if(res != icsneoc2_error_success) {
				rc = print_error_code("Failed to send TC10 wake", res);
				break;
			}
		} else if(args.send_sleep) {
			printf("requesting TC10 sleep on network %s\n", name);
			res = icsneoc2_device_tc10_sleep_request(device, netid);
			if(res != icsneoc2_error_success) {
				rc = print_error_code("Failed to send TC10 sleep", res);
				break;
			}
		} else { /* args.status */
			icsneoc2_tc10_sleep_status_t sleep_s = icsneoc2_tc10_sleep_status_no_sleep_received;
			icsneoc2_tc10_wake_status_t wake_s = icsneoc2_tc10_wake_status_no_wake_received;
			res = icsneoc2_device_tc10_status_get(device, netid, &sleep_s, &wake_s);
			if(res != icsneoc2_error_success) {
				rc = print_error_code("Failed to get TC10 status", res);
				break;
			}
			printf("TC10 status on network %s: wake=%s sleep=%s\n", name,
				tc10_wake_status_str(wake_s), tc10_sleep_status_str(sleep_s));
		}
	}

	printf("Closing device %s\n", description);
	icsneoc2_device_close(device);
	icsneoc2_device_free(device);
	icsneoc2_enumeration_free(found_devices);
	return rc;
}

static int print_error_code(const char* message, icsneoc2_error_t error) {
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

static void str_tolower(char* s) {
	for(; *s; ++s) {
		*s = (char)tolower((unsigned char)*s);
	}
}

/*
 * Resolve a network name (e.g. "ETHERNET_01", "ae_01") to its icsneoc2_netid_t
 * by iterating over all known netids and comparing names case-insensitively.
 */
static bool resolve_netid(const char* name, icsneoc2_netid_t* out) {
	char want[64];
	strncpy(want, name, sizeof(want) - 1);
	want[sizeof(want) - 1] = '\0';
	str_tolower(want);

	for(uint16_t i = 0; i < icsneoc2_netid_maxsize; ++i) {
		char buf[64];
		size_t buf_len = sizeof(buf);
		if(icsneoc2_netid_name_get((icsneoc2_netid_t)i, buf, &buf_len) != icsneoc2_error_success) {
			continue;
		}
		str_tolower(buf);
		if(strcmp(buf, want) == 0) {
			*out = (icsneoc2_netid_t)i;
			return true;
		}
	}
	return false;
}

static void print_usage(const char* prog) {
	printf("Usage:\n");
	printf("  %s --list\n", prog);
	printf("  %s [--serial SERIAL] --networks NET1 [NET2 ...] (--send-wake | --send-sleep | --status)\n", prog);
	printf("\n");
	printf("  --list                List connected devices and the Automotive Ethernet networks they support.\n");
	printf("  --serial SERIAL       Serial number of the device. If omitted, the first available device is used.\n");
	printf("  --networks NET ...    One or more network names (e.g. ETHERNET_01 AE_01). Consumes args until the next flag.\n");
	printf("  --send-wake           Trigger TC10 wake on the selected networks.\n");
	printf("  --send-sleep          Trigger TC10 sleep on the selected networks.\n");
	printf("  --status              Query TC10 wake/sleep status on the selected networks.\n");
	printf("  -h, --help            Show this message.\n");
}

static int parse_args(int argc, char** argv, args_t* out) {
	memset(out, 0, sizeof(*out));
	for(int i = 1; i < argc; ++i) {
		const char* a = argv[i];
		if(strcmp(a, "-h") == 0 || strcmp(a, "--help") == 0) {
			print_usage(argv[0]);
			exit(0);
		} else if(strcmp(a, "--serial") == 0) {
			if(i + 1 >= argc) {
				fprintf(stderr, "error: --serial requires a value\n");
				return 1;
			}
			out->serial = argv[++i];
		} else if(strcmp(a, "--send-wake") == 0) {
			out->send_wake = true;
		} else if(strcmp(a, "--send-sleep") == 0) {
			out->send_sleep = true;
		} else if(strcmp(a, "--status") == 0) {
			out->status = true;
		} else if(strcmp(a, "--list") == 0) {
			out->list = true;
		} else if(strcmp(a, "--networks") == 0) {
			if(i + 1 >= argc || argv[i + 1][0] == '-') {
				fprintf(stderr, "error: --networks requires at least one network name\n");
				return 1;
			}
			out->networks = (const char**)&argv[i + 1];
			size_t count = 0;
			while(i + 1 < argc && argv[i + 1][0] != '-') {
				++count;
				++i;
			}
			out->networks_count = count;
		} else {
			fprintf(stderr, "error: unknown argument '%s'\n", a);
			print_usage(argv[0]);
			return 1;
		}
	}

	if(out->list) {
		if(out->send_wake || out->send_sleep || out->status || out->networks_count > 0 || out->serial) {
			fprintf(stderr, "error: --list cannot be combined with other options\n");
			print_usage(argv[0]);
			return 1;
		}
		return 0;
	}

	if(out->networks_count == 0) {
		fprintf(stderr, "error: --networks is required\n");
		print_usage(argv[0]);
		return 1;
	}
	int action_count = (out->send_wake ? 1 : 0) + (out->send_sleep ? 1 : 0) + (out->status ? 1 : 0);
	if(action_count != 1) {
		fprintf(stderr, "error: exactly one of --send-wake, --send-sleep, or --status is required\n");
		print_usage(argv[0]);
		return 1;
	}
	return 0;
}

static const char* tc10_wake_status_str(icsneoc2_tc10_wake_status_t s) {
	switch(s) {
		case icsneoc2_tc10_wake_status_no_wake_received: return "no_wake_received";
		case icsneoc2_tc10_wake_status_wake_received:    return "wake_received";
		default:                                         return "unknown";
	}
}

static const char* tc10_sleep_status_str(icsneoc2_tc10_sleep_status_t s) {
	switch(s) {
		case icsneoc2_tc10_sleep_status_no_sleep_received: return "no_sleep_received";
		case icsneoc2_tc10_sleep_status_sleep_received:    return "sleep_received";
		case icsneoc2_tc10_sleep_status_sleep_failed:      return "sleep_failed";
		case icsneoc2_tc10_sleep_status_sleep_aborted:     return "sleep_aborted";
		default:                                           return "unknown";
	}
}

/*
 * Find a device matching the provided serial, or the first available device if serial is NULL.
 * Returns NULL on failure (caller is responsible for the enumeration list).
 */
static icsneoc2_device_info_t* find_device(icsneoc2_device_info_t* list, const char* serial) {
	if(serial == NULL) {
		return list;
	}
	for(icsneoc2_device_info_t* cur = list; cur != NULL; cur = icsneoc2_device_info_next(cur)) {
		char dev_serial[64] = {0};
		size_t dev_serial_len = sizeof(dev_serial);
		if(icsneoc2_device_info_serial_get(cur, dev_serial, &dev_serial_len) != icsneoc2_error_success) {
			continue;
		}
		if(strcmp(dev_serial, serial) == 0) {
			return cur;
		}
	}
	return NULL;
}

/*
 * List connected devices and the Automotive Ethernet networks each one supports.
 */
static int list_devices(icsneoc2_device_info_t* list) {
	size_t index = 0;
	for(icsneoc2_device_info_t* cur = list; cur != NULL; cur = icsneoc2_device_info_next(cur)) {
		char serial[64] = {0};
		size_t serial_len = sizeof(serial);
		(void)icsneoc2_device_info_serial_get(cur, serial, &serial_len);

		char description[256] = {0};
		size_t description_len = sizeof(description);
		(void)icsneoc2_device_info_description_get(cur, description, &description_len);

		printf("[%zu] %s (%s)\n", index++, description, serial);

		icsneoc2_device_t* device = NULL;
		icsneoc2_error_t res = icsneoc2_device_create(cur, &device);
		if(res != icsneoc2_error_success) {
			print_error_code("    Failed to create device", res);
			continue;
		}

		icsneoc2_open_options_t options = icsneoc2_open_options_default;
		options &= ~ICSNEOC2_OPEN_OPTIONS_SYNC_RTC;
		options &= ~ICSNEOC2_OPEN_OPTIONS_GO_ONLINE;
		res = icsneoc2_device_open(device, options);
		if(res != icsneoc2_error_success) {
			print_error_code("    Failed to open device", res);
			icsneoc2_device_free(device);
			continue;
		}

		bool supports_tc10 = false;
		(void)icsneoc2_device_supports_tc10(device, &supports_tc10);
		printf("    TC10 supported: %s\n", supports_tc10 ? "yes" : "no");

		size_t count = 0;
		res = icsneoc2_device_supported_tx_networks_get(device, NULL, &count);
		if(res != icsneoc2_error_success || count == 0) {
			icsneoc2_device_close(device);
			icsneoc2_device_free(device);
			continue;
		}

		icsneoc2_netid_t* nets = (icsneoc2_netid_t*)calloc(count, sizeof(icsneoc2_netid_t));
		if(nets == NULL) {
			fprintf(stderr, "    error: out of memory\n");
			icsneoc2_device_close(device);
			icsneoc2_device_free(device);
			continue;
		}

		res = icsneoc2_device_supported_tx_networks_get(device, nets, &count);
		if(res != icsneoc2_error_success) {
			print_error_code("    Failed to get supported networks", res);
			free(nets);
			icsneoc2_device_close(device);
			icsneoc2_device_free(device);
			continue;
		}

		printf("    Automotive Ethernet networks:\n");
		bool any = false;
		for(size_t i = 0; i < count; ++i) {
			icsneoc2_network_type_t type = icsneoc2_network_type_invalid;
			if(icsneoc2_netid_network_type_get(nets[i], &type) != icsneoc2_error_success) {
				continue;
			}
			if(type != icsneoc2_network_type_automotive_ethernet) {
				continue;
			}
			char name[64] = {0};
			size_t name_len = sizeof(name);
			if(icsneoc2_netid_name_get(nets[i], name, &name_len) != icsneoc2_error_success) {
				continue;
			}
			printf("      %s\n", name);
			any = true;
		}
		if(!any) {
			printf("      (none)\n");
		}

		free(nets);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
	}
	return 0;
}
