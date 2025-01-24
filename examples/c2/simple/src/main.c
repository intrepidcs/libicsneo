#include <icsneo/icsneoc2.h>

#include <stdio.h>
#include <time.h>

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
 * If the error code is not icsneoc2_error_success, prints the error string for the given error code
 * and returns the error code.
 *
 * @param message The message to print.
 * @param error The error code to print.
 * @return error as int
 */
int print_error_code(const char* message, icsneoc2_error_t error) {
    char error_str[256] = {0};
    uint32_t error_length = 256;
    icsneoc2_error_t res = icsneoc2_error_code_get(error, error_str, &error_length);
    if (res != icsneoc2_error_success) {
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
 * @param device A pointer to the icsneoc2_device_t structure representing the device.
 * @param messages An array of pointers to icsneoc2_message_t structures containing the messages to process.
 * @param messages_count The number of messages in the messages array.
 *
 * @return An icsneoc2_error_t value indicating success or failure of the message processing.
 */
int process_messages(icsneoc2_device_t* device, icsneoc2_message_t** messages, uint32_t messages_count);

/**
 * @brief Prints device and global events for a given device.
 *
 * This function retrieves and prints all current events associated with the specified device,
 * as well as any global events not tied to a specific device. For each event, it retrieves
 * and prints a description. If retrieving events or their descriptions fails, an error
 * message is printed. The function also prints a summary of the count of device-specific
 * and global events processed.
 *
 * @param device A pointer to the icsneoc2_device_t structure representing the device to get events from.
 * @param device_description A description of the device used in the output.
 */
void print_device_events(icsneoc2_device_t* device, const char* device_description);

/**
 * @brief Transmits a series of CAN messages from a device.
 * 
 * This function creates and transmits 100 CAN messages with incrementing payload data.
 * Each message is configured with specific attributes such as network ID, arbitration
 * ID, CANFD status, extended status, and baudrate switch. After successfully transmitting
 * each message, it is freed from memory.
 * 
 * @param device A pointer to the icsneoc2_device_t structure representing the device to transmit messages from.
 * 
 * @return An icsneoc2_error_t value indicating success or failure of the message transmission process.
 */
int transmit_can_messages(icsneoc2_device_t* device);

/**
 * @brief Get the RTC (Real time clock) of a device and print it.
 * 
 * @param[in] device The device to get the RTC of.
 * @param[in] description A description of the device for printing purpose.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t get_and_print_rtc(icsneoc2_device_t* device, const char* description);

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    icsneoc2_device_t* devices[255] = {0};
    uint32_t devices_count = 255;

    printf("Finding devices...\n");
    icsneoc2_error_t res = icsneoc2_device_find_all(devices, &devices_count, NULL);
    if (res != icsneoc2_error_success) {
        return print_error_code("\tFailed to find devices", res);
    };
    printf("OK, %u device%s found\n", devices_count, devices_count == 1 ? "" : "s");
    // List off the devices
    for (uint32_t i = 0; i < devices_count; i++) {
        icsneoc2_device_t* device = devices[i];
        // Get description of the device
        const char description[255] = {0};
        uint32_t description_length = 255;
        res = icsneoc2_device_description_get(device, description, &description_length);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to get device description", res);
        };
        printf("%.*s @ Handle %p\n", description_length, description, device);
        // Get/Set open options
        icsneoc2_open_options_t options = icsneoc2_open_options_none;
        res = icsneoc2_device_open_options_get(device, &options);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to get open options", res);
        }
        // Disable Syncing RTC and going online
        options &= ~icsneoc2_open_options_sync_rtc;
        options &= ~icsneoc2_open_options_go_online;
        printf("\tDevice open options: 0x%x\n", options);
        res = icsneoc2_device_open_options_set(device, options);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to set open options", res);
        }
        // Open the device
        printf("\tOpening device: %s...\n", description);
        res = icsneoc2_device_open(device);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to open device", res);
        };
        // Get timestamp resolution of the device
        printf("\tGetting timestamp resolution... ");
        uint32_t timestamp_resolution = 0;
        res = icsneoc2_device_timestamp_resolution_get(device, &timestamp_resolution);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to get timestamp resolution", res);
        }
        printf("%uns\n", timestamp_resolution);
        // Get baudrates for HSCAN
        printf("\tGetting HSCAN Baudrate... ");
        uint64_t baudrate = 0;
        res = icsneoc2_device_baudrate_get(device, icsneoc2_netid_hscan, &baudrate);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to get baudrate", res);
        };
        printf("%llumbit/s\n", baudrate);
        // Get FDbaudrates for HSCAN
        printf("\tGetting FD HSCAN Baudrate... ");
        uint64_t fd_baudrate = 0;
        res = icsneoc2_device_canfd_baudrate_get(device, icsneoc2_netid_hscan, &fd_baudrate);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to get FD baudrate", res);
        };
        printf("%llumbit/s\n", fd_baudrate);
        // Set baudrates for HSCAN
        // save_to_device: If this is set to true, the baudrate will be saved on the device 
        // and will persist through a power cycle
        bool save_to_device = false;
        printf("\tSetting HSCAN Baudrate... ");
        res = icsneoc2_device_baudrate_set(device, icsneoc2_netid_hscan, baudrate, save_to_device);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to set baudrate", res);
        };
        printf("Ok\n");
        // Set FDbaudrates for HSCAN
        printf("\tSetting FD HSCAN Baudrate... ");
        res = icsneoc2_device_canfd_baudrate_set(device, icsneoc2_netid_hscan, fd_baudrate, save_to_device);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to set FD baudrate", res);
        };
        printf("Ok\n");
        // Get RTC
        printf("\tGetting RTC... ");
        res = get_and_print_rtc(device, description);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to get RTC", res);
        }
        // Set RTC
        printf("\tSetting RTC to current time... ");
        time_t current_time = time(NULL);
        res = icsneoc2_device_rtc_set(device, current_time);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to set RTC", res);
        }
        printf("Ok\n");
        // Get RTC
        printf("\tGetting RTC... ");
        res = get_and_print_rtc(device, description);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to get RTC", res);
        }
        // Go online, start acking traffic
        printf("\tGoing online... ");
        res = icsneoc2_device_go_online(device, true);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to go online", res);
        }
        // Redundant check to show how to check if the device is online, if the previous
        // icsneoc2_device_go_online call was successful we can assume we are online already
        bool is_online = false;
        res = icsneoc2_device_is_online(device, &is_online);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to check if online", res);
        }
        printf("%s\n", is_online ? "Online" : "Offline");
        // Transmit CAN messages
        res = transmit_can_messages(device);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to transmit CAN messages", res);
        }
        // Wait for the bus to collect some messages, requires an active bus to get messages
        printf("\tWaiting 1 second for messages...\n");
        sleep_ms(1000);
        // Get the messages
        icsneoc2_message_t* messages[20000] = {0};
        uint32_t message_count = 20000;
        printf("\tGetting messages from device with timeout of 3000ms on %s...\n", description);
        res = icsneoc2_device_messages_get(device, messages, &message_count, 3000);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to get messages from device", res);
        };
        // Process the messages
        res = process_messages(device, messages, message_count);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to process messages", res);
        }
        // Finally, close the device.
        printf("\tClosing device: %s...\n", description);
        res = icsneoc2_device_close(device);
        if (res != icsneoc2_error_success) {
            print_device_events(device, description);
            return print_error_code("\tFailed to close device", res);
        };
        // Print device events
        print_device_events(device, description);

    }
    printf("\n");    
    return 0;
}

icsneoc2_error_t get_and_print_rtc(icsneoc2_device_t* device, const char* description) {
    time_t unix_epoch = 0;
    icsneoc2_error_t res = icsneoc2_device_rtc_get(device, &unix_epoch);
    if (res != icsneoc2_error_success) {
        return res;
    }
    char rtc_time[32] = {0};
    strftime(rtc_time, sizeof(rtc_time), "%Y-%m-%d %H:%M:%S", localtime(&unix_epoch));
    printf("RTC: %lld %s\n", unix_epoch, rtc_time);

    return icsneoc2_error_success;
}

void print_device_events(icsneoc2_device_t* device, const char* device_description) {
    // Get device events
    icsneoc2_event_t* events[1024] = {0};
    uint32_t events_count = 1024;
    icsneoc2_error_t res = icsneoc2_device_events_get(device, events, &events_count);
    if (res != icsneoc2_error_success) {
        (void)print_error_code("\tFailed to get device events", res);
        return;
    }
    // Loop over each event and describe it.
    for (uint32_t i = 0; i < events_count; i++) {
        const char event_description[255] = {0};
        uint32_t event_description_length = 255;
        res = icsneoc2_event_description_get(events[i], event_description, &event_description_length);
        if (res != icsneoc2_error_success) {
            print_error_code("\tFailed to get event description", res);
            continue;
        }
        printf("\t%s: Event %u: %s\n", device_description, i, event_description);
    }

    // Get global events
    icsneoc2_event_t* global_events[1024] = {0};
    uint32_t global_events_count = 1024;
    res = icsneoc2_events_get(global_events, &global_events_count);
    if (res != icsneoc2_error_success) {
        (void)print_error_code("\tFailed to get global events", res);
        return;
    }
    // Loop over each event and describe it.
    for (uint32_t i = 0; i < global_events_count; i++) {
        const char event_description[255] = {0};
        uint32_t event_description_length = 255;
        res = icsneoc2_event_description_get(global_events[i], event_description, &event_description_length);
        if (res != icsneoc2_error_success) {
            print_error_code("\tFailed to get global event description", res);
            continue;
        }
        printf("\t%s: Global Event %u: %s\n", device_description, i, event_description);
    }
    printf("\t%s: Received %u events and %u global events\n", device_description, events_count, global_events_count);
}

int process_messages(icsneoc2_device_t* device, icsneoc2_message_t** messages, uint32_t messages_count) {
    // Print the type and bus type of each message
    uint32_t tx_count = 0;
    for (uint32_t i = 0; i < messages_count; i++) {
        icsneoc2_message_t* message = messages[i];
        // Get the message type
        icsneoc2_msg_type_t msg_type = 0;
        icsneoc2_error_t res = icsneoc2_message_type_get(device, message, &msg_type);
        if (res != icsneoc2_error_success) {
            return print_error_code("\tFailed to get message type", res);
        }
        // Get the message type name
        char msg_type_name[128] = {0};
        uint32_t msg_type_name_length = 128;
        res = icsneoc2_message_type_name_get(msg_type, msg_type_name, &msg_type_name_length);
        if (res != icsneoc2_error_success) {
            return print_error_code("\tFailed to get message type name", res);
        }
        // Check if the message is a bus message, ignore otherwise
        if (msg_type != icsneoc2_msg_type_bus) {        
            printf("Ignoring message type: %u (%s)\n", msg_type, msg_type_name);
            continue;
        }
        icsneoc2_msg_bus_type_t bus_type = 0;
        res = icsneoc2_message_bus_type_get(device, message, &bus_type);
        if (res != icsneoc2_error_success) {
            return print_error_code("\tFailed to get message bus type", res);
        }
        const char bus_name[128] = {0};
        uint32_t bus_name_length = 128;
        res = icsneoc2_bus_type_name_get(bus_type, bus_name, &bus_name_length);
        if (res != icsneoc2_error_success) {
            return print_error_code("\tFailed to get message bus type name", res);
        }
        bool is_tx = false;
        res = icsneoc2_message_is_transmit(device, message, &is_tx);
        if (res != icsneoc2_error_success) {
            return print_error_code("\tFailed to get message is transmit", res);
        }
        if (is_tx) {
            tx_count++;
            continue;
        }

        printf("\t%d) Message type: %u bus type: %s (%u)\n", i, msg_type, bus_name, bus_type);
        if (bus_type == icsneoc2_msg_bus_type_can) {
            uint32_t arbid = 0;
            int32_t dlc = 0;
            icsneoc2_netid_t netid = 0;
            bool is_remote = false;
            bool is_canfd = false;
            bool is_extended = false;
            uint8_t data[64] = {0};
            uint32_t data_length = 64;
            const char netid_name[128] = {0};
            uint32_t netid_name_length = 128;
            uint32_t result = icsneoc2_message_netid_get(device, message, &netid);
            result += icsneoc2_netid_name_get(netid, netid_name, &netid_name_length);
            result += icsneoc2_message_can_arbid_get(device, message, &arbid);
            result += icsneoc2_message_can_dlc_get(device, message, &dlc);
            result += icsneoc2_message_can_is_remote(device, message, &is_remote);
            result += icsneoc2_message_can_is_canfd(device, message, &is_canfd);
            result += icsneoc2_message_can_is_extended(device, message, &is_extended);
            result += icsneoc2_message_data_get(device, message, data, &data_length);
            if (result != icsneoc2_error_success) {
                printf("\tFailed get get CAN parameters (error: %u) for index %u\n", result, i);
                continue;
            }
            printf("\t  NetID: %s (0x%x)\tArbID: 0x%x\t DLC: %u\t Remote: %d\t CANFD: %d\t Extended: %d\t Data length: %u\n", netid_name, netid, arbid, dlc, is_remote, is_canfd, is_extended, data_length);
            printf("\t  Data: [");
            for (uint32_t x = 0; x < data_length; x++) {
                printf(" 0x%x", data[x]);
            }
            printf(" ]\n");
        }
    }
    printf("\tReceived %u messages total, %u were TX messages\n", messages_count, tx_count);

    return icsneoc2_error_success;
}

int transmit_can_messages(icsneoc2_device_t* device) {
    uint64_t counter = 0;
    const uint32_t msg_count = 100;
    printf("\tTransmitting %d messages...\n", msg_count);
    for (uint32_t i = 0; i < msg_count; i++) {
        // Create the message
        icsneoc2_message_t* message = NULL;
        uint32_t message_count = 1;
        icsneoc2_error_t res = icsneoc2_message_can_create(device, &message, message_count);
        if (res != icsneoc2_error_success) {
            return print_error_code("\tFailed to create messages", res);
        }
        // Set the message attributes
        res = icsneoc2_message_netid_set(device, message, icsneoc2_netid_hscan);
        res += icsneoc2_message_can_arbid_set(device, message, 0x10);
        res += icsneoc2_message_can_canfd_set(device, message, true);
        res += icsneoc2_message_can_extended_set(device, message, true);
        res += icsneoc2_message_can_baudrate_switch_set(device, message, true);
        // Create the payload
        uint8_t data[8] = {0};
        data[0] = (uint8_t)(counter >> 56);
        data[1] = (uint8_t)(counter >> 48);
        data[2] = (uint8_t)(counter >> 40);
        data[3] = (uint8_t)(counter >> 32);
        data[4] = (uint8_t)(counter >> 24);
        data[5] = (uint8_t)(counter >> 16);
        data[6] = (uint8_t)(counter >> 8);
        data[7] = (uint8_t)(counter >> 0);
        res += icsneoc2_message_data_set(device, message, data, sizeof(data));
        res += icsneoc2_message_can_dlc_set(device, message, -1);
        if (res != icsneoc2_error_success) {
            return print_error_code("\tFailed to modify message", res);
        }
        res = icsneoc2_device_messages_transmit(device, &message, &message_count);
        res += icsneoc2_message_can_free(device, message);
        if (res != icsneoc2_error_success) {
            return print_error_code("\tFailed to transmit messages", res);
        }
        counter++;
    }
    
    return icsneoc2_error_success;
}
