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
    sleep(ms / 1000);
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
    icsneo_error_t res = icsneo_get_error_code(error, error_str, &error_length);
    if (res != icsneo_error_success) {
        printf("%s: Failed to get string for error code %d with error code %d\n", message, error, res);
        return res;
    }
    printf("%s: \"%s\" (%u)\n", message, error_str, error);
    return (int)error;
}

/**
 * @brief Processes a list of messages from a device.
 *
 * This function iterates over a given array of messages received from a specified device. 
 * For each message in the array, it retrieves and prints the message type and bus type.
 * If an error occurs while retrieving these details, an error message is printed.
 *
 * @param device A pointer to the icsneo_device_t structure representing the device.
 * @param messages An array of pointers to icsneo_message_t structures containing the messages to process.
 * @param messages_count The number of messages in the messages array.
 *
 * @return An icsneo_error_t value indicating success or failure of the message processing.
 */
int process_messages(icsneo_device_t* device, icsneo_message_t** messages, uint32_t messages_count);

void print_device_events(icsneo_device_t* device, const char* device_description);

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    icsneo_device_t* devices[255] = {0};
    uint32_t devices_count = 255;

    icsneo_error_t res = icsneo_device_find_all(devices, &devices_count, NULL);
    if (res != icsneo_error_success) {
        return print_error_code("Failed to find devices", res);
    };

    printf("Found %u devices\n", devices_count);
    // Loop over each device
    for (uint32_t i = 0; i < devices_count; i++) {
        icsneo_device_t* device = devices[i];
        // Get description of the device
        const char description[255] = {0};
        uint32_t description_length = 255;
        res = icsneo_device_get_description(device, description, &description_length);
        if (res != icsneo_error_success) {
            print_device_events(device, description);
            return print_error_code("Failed to get device description", res);
        };
        // Get timestamp resolution of the device
        uint32_t timestamp_resolution = 0;
        res = icsneo_device_get_timestamp_resolution(device, &timestamp_resolution);
        if (res != icsneo_error_success) {
            print_device_events(device, description);
            return print_error_code("Failed to get timestamp resolution", res);
        }
        printf("%s timestamp resolution: %uns\n", description, timestamp_resolution);
        // Get/Set open options
        icsneo_open_options_t options = icsneo_open_options_none;
        res = icsneo_device_get_open_options(device, &options);
        if (res != icsneo_error_success) {
            print_device_events(device, description);
            return print_error_code("Failed to get open options", res);
        }
        // Disable Syncing RTC
        options &= ~icsneo_open_options_sync_rtc;
        res = icsneo_device_set_open_options(device, options);
        if (res != icsneo_error_success) {
            print_device_events(device, description);
            return print_error_code("Failed to set open options", res);
        }
        // Open the device
        printf("Opening device: %s...\n", description);
        res = icsneo_device_open(device);
        if (res != icsneo_error_success) {
            print_device_events(device, description);
            return print_error_code("Failed to open device", res);
        };
        // Wait for the bus to collect some messages, requires an active bus to get messages
        printf("Waiting 1 second for messages...\n");
        sleep_ms(1000);
        // Get the messages
        icsneo_message_t* messages[20000] = {0};
        uint32_t message_count = 20000;
        printf("Getting messages from device with timeout of 3000ms on %s...\n", description);
        res = icsneo_device_get_messages(device, messages, &message_count, 3000);
        if (res != icsneo_error_success) {
            print_device_events(device, description);
            return print_error_code("Failed to get messages from device", res);
        };
        // Process the messages
        res = process_messages(device, messages, message_count);
        if (res != icsneo_error_success) {
            print_device_events(device, description);
            return print_error_code("Failed to process messages", res);
        }
        // Finally, close the device.
        printf("Closing device: %s...\n", description);
        res = icsneo_device_close(device);
        if (res != icsneo_error_success) {
            print_device_events(device, description);
            return print_error_code("Failed to close device", res);
        };
        // Print device events
        print_device_events(device, description);
    }
    
    return 0;
}

void print_device_events(icsneo_device_t* device, const char* device_description) {
    // Get device events
    icsneo_event_t* events[1024] = {0};
    uint32_t events_count = 1024;
    icsneo_error_t res = icsneo_device_get_events(device, events, &events_count);
    if (res != icsneo_error_success) {
        (void)print_error_code("Failed to get device events", res);
        return;
    }
    // Loop over each event and describe it.
    for (uint32_t i = 0; i < events_count; i++) {
        const char event_description[255] = {0};
        uint32_t event_description_length = 255;
        res = icsneo_event_get_description(events[i], event_description, &event_description_length);
        if (res != icsneo_error_success) {
            print_error_code("Failed to get event description", res);
            continue;
        }
        printf("\t%s: Event %u: %s\n", device_description, i, event_description);
    }

    // Get global events
    icsneo_event_t* global_events[1024] = {0};
    uint32_t global_events_count = 1024;
    res = icsneo_get_events(global_events, &global_events_count);
    if (res != icsneo_error_success) {
        (void)print_error_code("Failed to get global events", res);
        return;
    }
    // Loop over each event and describe it.
    for (uint32_t i = 0; i < global_events_count; i++) {
        const char event_description[255] = {0};
        uint32_t event_description_length = 255;
        res = icsneo_event_get_description(global_events[i], event_description, &event_description_length);
        if (res != icsneo_error_success) {
            print_error_code("Failed to get global event description", res);
            continue;
        }
        printf("\t%s: Global Event %u: %s\n", device_description, i, event_description);
    }
    printf("%s: Received %u events and %u global events\n", device_description, events_count, global_events_count);
}

int process_messages(icsneo_device_t* device, icsneo_message_t** messages, uint32_t messages_count) {
    printf("Received %u messages\n", messages_count);
    // Print the type and bus type of each message
    for (uint32_t i = 0; i < messages_count; i++) {
        icsneo_message_t* message = messages[i];
        icsneo_msg_type_t msg_type = 0;
        icsneo_error_t res = icsneo_message_get_type(device, message, &msg_type);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to get message type", res);
        }
        icsneo_msg_bus_type_t bus_type = 0;
        res = icsneo_message_get_bus_type(device, message, &bus_type);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to get message bus type", res);
        }
        const char bus_name[128] = {0};
        uint32_t bus_name_length = 128;
        res += icsneo_get_bus_type_name(&bus_type, bus_name, &bus_name_length);
        if (res != icsneo_error_success) {
            return print_error_code("Failed to get message bus type name", res);
        }

        printf("\t%d) Message type: %u bus type: %s (%u)\n", i, msg_type, bus_name, bus_type);
        if (bus_type == icsneo_msg_bus_type_can) {
            uint32_t arbid = 0;
            uint32_t dlc = 0;
            bool is_remote = false;
            bool is_canfd = false;
            bool is_extended = false;
            uint8_t data[64] = {0};
            uint32_t data_length = 64;
            
            uint32_t result = icsneo_can_message_get_arbid(device, message, &arbid);
            result += icsneo_can_message_get_dlc_on_wire(device, message, &dlc);
            result += icsneo_can_message_is_remote(device, message, &is_remote);
            result += icsneo_can_message_is_canfd(device, message, &is_canfd);
            result += icsneo_can_message_is_extended(device, message, &is_extended);
            result += icsneo_message_get_data(device, message, data, &data_length);

            if (result != icsneo_error_success) {
                printf("\tFailed get get CAN parameters (error: %u) for index %u\n", result, i);
                continue;
            }
            printf("\t  ArbID: 0x%x\t DLC: %u\t Remote: %d\t CANFD: %d\t Extended: %d\t Data length: %u\n", arbid, dlc, is_remote, is_canfd, is_extended, data_length);
            printf("\t  Data: [");
            for (uint32_t x = 0; x < data_length; x++) {
                printf(" 0x%x", data[x]);
            }
            printf(" ]\n");
            continue;
        }
    }

    return icsneo_error_success;
}
