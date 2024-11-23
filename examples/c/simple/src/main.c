#include <icsneo/icsneo.h>

#include <stdio.h>


int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    icsneo_device_t* devices[255] = {0};
    uint32_t devices_count = 255;

    if (icsneo_find(devices, &devices_count, NULL) != icsneo_error_success) {
        printf("Failed to find devices\n");
        return 1;
    };

    printf("Found %u devices\n", devices_count);

    for (uint32_t i = 0; i < devices_count; i++) {
        icsneo_device_t* device = devices[i];

        const char description[255] = {0};
        uint32_t description_length = 255;
        icsneo_error_t res = icsneo_device_describe(device, description, &description_length);
        if (res != icsneo_error_success) {
            printf("Failed to get device description %d\n", res);
            return 1;
        };

        icsneo_open_options_t options = icsneo_open_options_none;
        res = icsneo_get_open_options(device, &options);
        if (res != icsneo_error_success) {
            printf("Failed to get open options %d\n", res);
            return 1;
        }
        // Disable Syncing RTC
        options &= ~icsneo_open_options_sync_rtc;
        res = icsneo_set_open_options(device, options);
        if (res != icsneo_error_success) {
            printf("Failed to set open options %d\n", res);
            return 1;
        }

        printf("Opening device: %s...\n", description);
        res = icsneo_open(device);
        if (res != icsneo_error_success) {
            printf("Failed to open device %d\n", res);
            return 1;
        };

        printf("Closing device: %s...\n", description);
        res = icsneo_close(device);
        if (res != icsneo_error_success) {
            printf("Failed to close device %d\n", res);
            return 1;
        };
    }
    
    return 0;
}
