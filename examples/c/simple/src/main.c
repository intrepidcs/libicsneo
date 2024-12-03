#include <icsneo/icsneo.h>

#include <stdio.h>

int print_error_code(const char* message, icsneo_error_t error) {
    char error_str[256] = {0};
    uint32_t error_length = 256;
    icsneo_error_t res = icsneo_error_code(error, error_str, &error_length);
    if (res != icsneo_error_success) {
        printf("%s: Failed to get string for error code %d with error code %d\n", message, error, res);
        return res;
    }
    printf("%s: %s\n", message, error_str);
    return (int)error;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    icsneo_device_t* devices[255] = {0};
    uint32_t devices_count = 255;

    icsneo_error_t res = icsneo_find(devices, &devices_count, NULL);
    if (res != icsneo_error_success) {
        return print_error_code("Failed to find devices", res);
    };

    printf("Found %u devices\n", devices_count);

    for (uint32_t i = 0; i < devices_count; i++) {
        icsneo_device_t* device = devices[i];

        const char description[255] = {0};
        uint32_t description_length = 255;
        res = icsneo_device_describe(device, description, &description_length);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to get device description", res);
        };

        icsneo_open_options_t options = icsneo_open_options_none;
        res = icsneo_get_open_options(device, &options);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to get open options", res);
        }
        // Disable Syncing RTC
        //options &= ~icsneo_open_options_sync_rtc;
        res = icsneo_set_open_options(device, options);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to set open options", res);
        }

        printf("Opening device: %s...\n", description);
        res = icsneo_open(device);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to open device", res);
        };

        printf("Closing device: %s...\n", description);
        res = icsneo_close(device);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to close device", res);
        };
    }
    
    return 0;
}
