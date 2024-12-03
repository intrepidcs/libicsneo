#include <icsneo/icsneo.h>

#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <unistd.h>
#endif


/**
 * @brief Sleeps for a specified number of milliseconds.
 *
 * Sleeps for a specified number of milliseconds using Sleep() on Windows and sleep() on *nix.
 *
 * @param ms The number of milliseconds to sleep.
 */
void sleep_ms(uint32_t ms) {
#if defined(_WIN32) || defined(_WIN64)
    Sleep(ms);
#else
    sleep(ms);
#endif
}

/**
 * @brief Prints an error message with the given string and error code.
 *
 * If the error code is not icsneo_error_success, prints the error string for the given error code
 * and returns the error code.
 *
 * @param message The message to print.
 * @param error The error code to print.
 * @return error as int
 */
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
        options &= ~icsneo_open_options_sync_rtc;
        res = icsneo_set_open_options(device, options);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to set open options", res);
        }

        printf("Opening device: %s...\n", description);
        res = icsneo_open(device);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to open device", res);
        };

        printf("Waiting 3 seconds for messages...\n");
        sleep_ms(3000);

        icsneo_message_t* messages[20000] = {0};
        uint32_t message_count = 20000;
        res = icsneo_get_messages(device, messages, &message_count, 3000);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to get messages from device", res);
        };
        printf("Received %u messages\n", message_count);

        printf("Closing device: %s...\n", description);
        res = icsneo_close(device);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to close device", res);
        };
    }
    
    return 0;
}
