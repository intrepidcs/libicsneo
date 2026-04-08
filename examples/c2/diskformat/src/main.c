#include <icsneo/icsneoc2.h>

#include <stdio.h>
#include <inttypes.h>

/**
 * Prints an error message with the given string and error code.
 *
 * @param message The message to print.
 * @param error The error code to print.
 * @return error as int
 */
static int print_error_code(const char* message, icsneoc2_error_t error) {
	char error_str[64];
	size_t error_str_len = sizeof(error_str);
	icsneoc2_error_code_get(error, error_str, &error_str_len);
	printf("%s: \"%s\" (%u)\n", message, error_str, error);
	return (int)error;
}

/**
 * Progress callback invoked periodically during disk formatting.
 *
 * @param sectors_formatted Number of sectors formatted so far.
 * @param total_sectors Total number of sectors to format.
 * @param user_data Unused opaque pointer.
 * @return icsneoc2_disk_format_directive_continue to keep formatting.
 */
static icsneoc2_disk_format_directive_t format_progress(uint64_t sectors_formatted, uint64_t total_sectors, void* user_data) {
	(void)user_data;
	double pct = total_sectors > 0 ? (100.0 * (double)sectors_formatted / (double)total_sectors) : 0.0;
	printf("\r  Progress: %" PRIu64 " / %" PRIu64 " sectors  (%d%%)", sectors_formatted, total_sectors, (int)pct);
	fflush(stdout);
	return icsneoc2_disk_format_directive_continue;
}

int main() {
	/* Open the first available device (no online needed for formatting) */
	printf("Opening first available device...\n");
	icsneoc2_device_t* device = NULL;
	icsneoc2_error_t res = icsneoc2_device_open_first(0, ICSNEOC2_OPEN_OPTIONS_NONE, &device);
	if(res != icsneoc2_error_success) {
		return print_error_code("\tFailed to open first device", res);
	}

	/* Get a description of the opened device */
	char description[255] = {0};
	size_t description_length = sizeof(description);
	res = icsneoc2_device_description_get(device, description, &description_length);
	if(res != icsneoc2_error_success) {
		icsneoc2_device_free(device);
		return print_error_code("\tFailed to get device description", res);
	}
	printf("\tOpened device: %s\n", description);

	/* Check disk formatting support */
	bool supported = false;
	res = icsneoc2_device_supports_disk_formatting(device, &supported);
	if(res != icsneoc2_error_success) {
		print_error_code("\tFailed to check disk formatting support", res);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return -1;
	}
	if(!supported) {
		printf("\terror: %s does not support disk formatting\n", description);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return -1;
	}

	size_t disk_count = 0;
	icsneoc2_device_disk_count_get(device, &disk_count);
	printf("\tDisk count: %zu\n", disk_count);

	/* Query disk details */
	printf("\tQuerying disk details... ");
	fflush(stdout);
	icsneoc2_disk_details_t* details = NULL;
	res = icsneoc2_device_disk_details_get(device, &details);
	if(res != icsneoc2_error_success) {
		printf("FAIL\n");
		print_error_code("\tFailed to get disk details", res);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return -1;
	}
	printf("OK\n");

	/* Display current state */
	icsneoc2_disk_layout_t layout = 0;
	icsneoc2_disk_details_layout_get(details, &layout);
	printf("\t  Layout        : %s\n", layout == icsneoc2_disk_layout_raid0 ? "RAID0" : "Spanned");

	size_t detail_count = 0;
	icsneoc2_disk_details_count_get(details, &detail_count);
	for(size_t i = 0; i < detail_count; i++) {
		icsneoc2_disk_format_flags_t flags = 0;
		icsneoc2_disk_details_flags_get(details, i, &flags);
		printf("\t  Disk [%zu]:\n", i);
		printf("\t    Present     : %s\n", (flags & ICSNEOC2_DISK_FORMAT_FLAGS_PRESENT) ? "yes" : "no");
		printf("\t    Initialized : %s\n", (flags & ICSNEOC2_DISK_FORMAT_FLAGS_INITIALIZED) ? "yes" : "no");
		printf("\t    Formatted   : %s\n", (flags & ICSNEOC2_DISK_FORMAT_FLAGS_FORMATTED) ? "yes" : "no");
		if(flags & ICSNEOC2_DISK_FORMAT_FLAGS_PRESENT) {
			uint64_t sectors = 0, bps = 0;
			icsneoc2_disk_details_size_get(details, i, &sectors, &bps);
			printf("\t    Size        : %" PRIu64 " MB (%" PRIu64 " sectors x %" PRIu64 " bytes)\n",
				(sectors * bps) / (1024 * 1024), sectors, bps);
		}
	}

	/* Build format config: mark present disks for formatting */
	bool any_present = false;
	for(size_t i = 0; i < detail_count; i++) {
		icsneoc2_disk_format_flags_t flags = 0;
		icsneoc2_disk_details_flags_get(details, i, &flags);
		if(flags & ICSNEOC2_DISK_FORMAT_FLAGS_PRESENT) {
			flags |= ICSNEOC2_DISK_FORMAT_FLAGS_FORMATTED;
			icsneoc2_disk_details_flags_set(details, i, flags);
			any_present = true;
		}
	}
	icsneoc2_disk_details_full_format_set(details, false); /* Quick format */

	if(!any_present) {
		printf("\n\terror: no disks are present in the device\n");
		icsneoc2_disk_details_free(details);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return -1;
	}

	/* Confirm */
	printf("\n\tThis will format the disk(s) in %s.\n", description);
	printf("\tAll existing data will be lost. Continue? [y/N]: ");
	char confirm[8] = {0};
	if(scanf("%7s", confirm) != 1 || (confirm[0] != 'y' && confirm[0] != 'Y')) {
		printf("\tAborted.\n");
		icsneoc2_disk_details_free(details);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return 0;
	}

	/* Format */
	printf("\n\tStarting format...\n");
	res = icsneoc2_device_format_disk(device, details, format_progress, NULL);
	printf("\n"); /* newline after progress line */

	if(res != icsneoc2_error_success) {
		print_error_code("\tFormat failed", res);
		icsneoc2_disk_details_free(details);
		icsneoc2_device_close(device);
		icsneoc2_device_free(device);
		return -1;
	}
	printf("\tFormat complete!\n");
	icsneoc2_disk_details_free(details);

	/* Verify */
	printf("\n\tVerifying disk state after format... ");
	fflush(stdout);
	icsneoc2_disk_details_t* post_details = NULL;
	res = icsneoc2_device_disk_details_get(device, &post_details);
	if(res != icsneoc2_error_success) {
		printf("FAIL (could not re-query disk details)\n");
	} else {
		printf("OK\n");
		size_t post_count = 0;
		icsneoc2_disk_details_count_get(post_details, &post_count);
		for(size_t i = 0; i < post_count; i++) {
			icsneoc2_disk_format_flags_t flags = 0;
			icsneoc2_disk_details_flags_get(post_details, i, &flags);
			printf("\t  Disk [%zu] formatted: %s\n", i, (flags & ICSNEOC2_DISK_FORMAT_FLAGS_FORMATTED) ? "yes" : "no");
		}
		icsneoc2_disk_details_free(post_details);
	}

	printf("\tClosing device: %s...\n", description);
	icsneoc2_device_close(device);
	icsneoc2_device_free(device);
	return 0;
}
