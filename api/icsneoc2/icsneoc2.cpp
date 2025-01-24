#include <icsneo/icsneoc2.h>
#include <icsneo/device/device.h>
#include "icsneo/device/devicefinder.h"
#include "icsneo/icsneocpp.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/message/linmessage.h"
#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/communication/packet/canpacket.h"

#include <string>
#include <vector>
#include <list>
#include <algorithm>

using namespace icsneo;

typedef struct icsneoc2_device_t {
    std::shared_ptr<Device> device;
    // Received messages from the device, we can automatically free them without the user.
    std::vector<std::shared_ptr<icsneoc2_message_t>> messages;
    // Seperate buffer for transmit messages for simplicity. User is responsible for freeing.
    // This needs to be a list so that pointers aren't invalidated on push_back or erase.
    std::list<std::shared_ptr<icsneoc2_message_t>> tx_messages;
    std::vector<icsneoc2_event_t> events;

    icsneoc2_open_options_t options;

    icsneoc2_device_t(std::shared_ptr<Device>& device, icsneoc2_open_options_t options) : device(device), options(options) {}

    // Take care of cleaning up the device buffers like when closing the device
    void clean_up() {
        messages.clear();
        messages.shrink_to_fit();
        events.clear();
        events.shrink_to_fit();
        tx_messages.clear();
    }
} icsneoc2_device_t;

// Any new members to this struct should be initialized in icsneoc2_device_messages_get()
typedef struct icsneoc2_message_t {
    std::shared_ptr<Message> message;
    // Indicates this message is a transmit message and should be in the tx_messages vector
    bool is_tx;

    icsneoc2_message_t(std::shared_ptr<Message>& message, bool is_tx) : message(message), is_tx(is_tx) {}
} icsneoc2_message_t;

typedef struct icsneoc2_event_t {
    APIEvent event;
} icsneoc2_event_t;


static std::vector<std::shared_ptr<icsneoc2_device_t>> g_devices;
static std::vector<icsneoc2_event_t> g_events;

/**
 * Safely copies a std::string to a char array.
 *
 * @param dest The buffer to copy the string into
 * @param dest_size* The size of the buffer. Will be modified to the length of the string
 * @param src The string to copy
 *
 * @return true if the string was successfully copied, false otherwise
 *
 * @note This function always null terminates the buffer, even if the string is too long.
 *       This is done for security reasons, not performance.
 */
bool safe_str_copy(const char* dest, uint32_t* const dest_size, std::string src) {
    if (!dest || !dest_size) {
        return false;
    }
    // zero terminate the entire string, this is done for security not for performance
    memset(const_cast<char*>(dest), 0, *dest_size);
    // Need to save room for the null terminator
    *dest_size -= 1;
    try {
        auto copied = src.copy(const_cast<char*>(dest), static_cast<size_t>(*dest_size), 0);
        *dest_size = static_cast<uint32_t>(copied);
        // Somehow we didn't copy the number of bytes we needed to? This check probably isn't needed.
        if (copied != src.length()) {
            return false;
        }
        return true;
    } catch (std::out_of_range& ex) {
        // if pos > size()
        (void)ex;
        return false;
    }
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_error_code_get(icsneoc2_error_t error_code, const char* value, uint32_t* value_length) {
    if (!value || !value_length) {
        return icsneoc2_error_invalid_parameters;
    }

    std::string error("Unknown");
    switch (error_code) {
        case icsneoc2_error_success:
            error = "Success";
            break;
        case icsneoc2_error_invalid_parameters:
            error = "Invalid function parameters";
            break;
        case icsneoc2_error_open_failed:
            error = "Open failed";
            break;
        case icsneoc2_error_go_online_failed:
            error = "Going online failed";
            break;
        case icsneoc2_error_enable_message_polling_failed:
            error = "Enable message polling failed";
            break;
        case icsneoc2_error_sync_rtc_failed:
            error = "Syncronizing RTC failed";
            break;
        case icsneoc2_error_get_messages_failed:
            error = "Getting messages failed";
            break;
        case icsneoc2_error_invalid_type:
            error = "Invalid type";
            break;
        case icsneoc2_error_rtc_failure:
            error = "RTC failure";
            break;
        case icsneoc2_error_set_settings_failure:
            error = "Setting settings failed";
            break;
        case icsneoc2_error_transmit_messages_failed:
            error = "Transmitting messages failed";
            break;
        case icsneoc2_error_string_copy_failed:
            error = "String copy failed";
            break;
        case icsneoc2_error_invalid_device:
            error = "Invalid device";
            break;
        case icsneoc2_error_invalid_message:
            error = "Invalid message";
            break;
        // Don't default, let the compiler warn us if we forget to handle an error code
    }
    // Copy the string into value
    return safe_str_copy(value, value_length, error) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_type_name_get(icsneoc2_devicetype_t device_type, const char* value, uint32_t* value_length) {
    if (!value || !value_length) {
        return icsneoc2_error_invalid_parameters;
    }

    auto device_type_str = DeviceType::GetGenericProductName(device_type);
    // Copy the string into value
    return safe_str_copy(value, value_length, device_type_str) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_find_all(icsneoc2_device_t** devices, uint32_t* devices_count, void* reserved) {
    (void)reserved;
    if (!devices ||!devices_count) {
        return icsneoc2_error_invalid_parameters;
    }
    // Find devices
    auto found_devices = DeviceFinder::FindAll();
    // Remove all devices that have been closed or are no longer connected
    for (auto it = g_devices.begin(); it != g_devices.end();) {
        if (!it->get()->device) {
            it = g_devices.erase(it);
        } else {
            ++it;
        }
    }
    // Add new devices
    for (auto& found_device : found_devices) {
        if (std::none_of(g_devices.begin(), g_devices.end(), 
            [&](const auto& device) { 
                return device->device == found_device; 
            })) {
                auto default_options = icsneoc2_open_options_go_online | icsneoc2_open_options_enable_message_polling | icsneoc2_open_options_sync_rtc | icsneoc2_open_options_enable_auto_update;
                auto device = std::make_shared<icsneoc2_device_t>(found_device, default_options);
                g_devices.push_back(device);
        }
    }
    // Determine how many we can return to the caller
    auto min_size = std::minmax(static_cast<uint32_t>(found_devices.size()), *devices_count).first;
    *devices_count = min_size;

    // Return the devices to the caller
    for (uint32_t i = 0; i < min_size; i++) {
        devices[i] = g_devices[i].get();
    }
    // Winner winner chicken dinner
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_valid(icsneoc2_device_t* device) {
    if (!device) {
        return icsneoc2_error_invalid_parameters;
    }

    if (!std::any_of(g_devices.begin(), g_devices.end(), [&](const auto& dev) {
        return dev.get() == device;
    })) {
        return icsneoc2_error_invalid_device;
    }
    if (!device->device) {
        return icsneoc2_error_invalid_device;
    }

    return !device->device ? icsneoc2_error_invalid_device : icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_open(icsneoc2_device_t* device, bool* is_open) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!is_open) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    *is_open = dev->isOpen();

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_disconnected(icsneoc2_device_t* device, bool* is_disconnected) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!is_disconnected) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    *is_disconnected = dev->isDisconnected();

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_open_options_get(icsneoc2_device_t* device, icsneoc2_open_options_t* options) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!options) {
        return icsneoc2_error_invalid_parameters;
    }
    *options = device->options;

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_open_options_set(icsneoc2_device_t* device, icsneoc2_open_options_t options) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }

    device->options = options;

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_open(icsneoc2_device_t* device) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Open the device
    auto dev = device->device;
    // Nothing to do if we are already open
    if (dev->isOpen()) {
        return icsneoc2_error_success;
    }
    // TODO: OpenFlags and OpenStatusHandler
    // Open the device
    if (!dev->open()) {
        return icsneoc2_error_open_failed;
    }
    // Sync RTC
    if ((device->options & icsneoc2_open_options_sync_rtc) == icsneoc2_open_options_sync_rtc && !dev->setRTC(std::chrono::system_clock::now())) {
        dev->close();
        return icsneoc2_error_sync_rtc_failed;
    }
    // Enable message polling
    if ((device->options & icsneoc2_open_options_enable_message_polling) == icsneoc2_open_options_enable_message_polling && !dev->enableMessagePolling()) {
        dev->close();
        return icsneoc2_error_enable_message_polling_failed;
    }
    // Go online
    if ((device->options & icsneoc2_open_options_go_online) == icsneoc2_open_options_go_online && !dev->goOnline()) {
        dev->close();
        return icsneoc2_error_go_online_failed;
    }
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_close(icsneoc2_device_t* device) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    auto dev = device->device;
    if (!dev->isOpen()) {
        return icsneoc2_error_success;
    }
    dev->close();
    // Clear out old messages and events
    device->clean_up();
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_description_get(icsneoc2_device_t* device, const char* value, uint32_t* value_length) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    auto dev = device->device;
    // Copy the string into value
    return safe_str_copy(value, value_length, dev->describe()) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_type_get(icsneoc2_device_t* device, icsneoc2_devicetype_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    auto dev = device->device;
    *value = dev->getType().getDeviceType();
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_serial_get(icsneoc2_device_t* device, const char* value, uint32_t* value_length) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    auto dev = device->device;
    // Copy the string into value
    return safe_str_copy(value, value_length, dev->getSerial()) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}


ICSNEOC2_API icsneoc2_error_t icsneoc2_device_go_online(icsneoc2_device_t* device, bool go_online) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    auto dev = device->device;
    // Go online
    if (go_online && dev->goOnline()) {
        return icsneoc2_error_success;
    }
    // Go offline
    if (!go_online && dev->goOffline()) {
        return icsneoc2_error_success;
    }

    return icsneoc2_error_go_online_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_online(icsneoc2_device_t* device, bool* is_online) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!is_online) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    *is_online = dev->isOnline();
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_online_supported(icsneoc2_device_t* device, bool* is_online_supported) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!is_online_supported) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    *is_online_supported = dev->isOnlineSupported();
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_polling_set(icsneoc2_device_t* device, bool enable) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    auto dev = device->device;
    // Enable message polling
    if (enable && dev->enableMessagePolling()) {
        return icsneoc2_error_success;
    } 
    // Disable message polling
    if (!enable && dev->disableMessagePolling()) {
        return icsneoc2_error_success;
    }
    
    return icsneoc2_error_enable_message_polling_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_polling_get(icsneoc2_device_t* device, bool* is_enabled) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!is_enabled) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    *is_enabled = dev->isMessagePollingEnabled();
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_polling_set_limit(icsneoc2_device_t* device, uint32_t limit) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    auto dev = device->device;
    dev->setPollingMessageLimit(static_cast<size_t>(limit));
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_polling_limit_get(icsneoc2_device_t* device, uint32_t* limit) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!limit) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    *limit = static_cast<uint32_t>(dev->getPollingMessageLimit());
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_count_get(icsneoc2_device_t* device, uint32_t* count)  {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!count) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    *count = static_cast<uint32_t>(dev->getCurrentMessageCount());
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_timestamp_resolution_get(icsneoc2_device_t* device, uint32_t* resolution) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!resolution) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    *resolution = static_cast<uint32_t>(dev->getTimestampResolution());
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_messages_get(icsneoc2_device_t* device, icsneoc2_message_t** messages, uint32_t* messages_count, uint32_t timeout_ms) {    
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!messages || !messages_count) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the device is valid
    res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    auto dev = device->device;
    // Wait for messages
    auto start_time = std::chrono::steady_clock::now();
    while (timeout_ms && std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_time)
                   .count() < timeout_ms &&
           dev->getCurrentMessageCount() == 0) {
        // Lets make sure we don't busy loop, we don't have any messages yet
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        continue;
    }
    // Get the messages
    auto results = dev->getMessages();
    auto& queried_messages = results.first;
    auto& success = results.second;
    if (!success) {
        return icsneoc2_error_get_messages_failed;
    }
    // Find the minimum number of messages
    uint32_t min_size = std::minmax(static_cast<uint32_t>(queried_messages.size()), *messages_count).first;
    *messages_count = min_size;

    // Copy the messages into our device message container
    device->messages.clear();
    for (auto& message : queried_messages) {
        auto message_t = std::make_shared<icsneoc2_message_t>(message, false);
        device->messages.push_back(message_t);
    }
    device->messages.shrink_to_fit();
    // Copy the messages into the output array
    for (uint32_t i = 0; i < min_size; i++) {
        messages[i] = device->messages[i].get();
    }
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_messages_transmit(icsneoc2_device_t* device, icsneoc2_message_t** messages, uint32_t* messages_count) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!messages || !messages_count) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    uint32_t i = 0;
    bool success = false;
    for (; i < *messages_count; i++) {
        // Make sure the message is valid
        bool is_msg_valid = false;
        res = icsneoc2_message_is_valid(device, messages[i], &is_msg_valid);
        if (res != icsneoc2_error_success) {
            return res;
        } else if (!is_msg_valid) {
            return icsneoc2_error_invalid_message;
        }
        success = dev->transmit(std::static_pointer_cast<icsneo::BusMessage>(messages[i]->message));
        if (!success) {
            break;
        }
    }
    *messages_count = i;

    return success ? icsneoc2_error_success : icsneoc2_error_transmit_messages_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_is_valid(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* is_valid) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message || !is_valid) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the device is valid
    res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // See if the message is a valid received message
    bool is_rx_msg = std::any_of(device->messages.begin(), device->messages.end(), [&](const auto& msg) {
        return msg.get() == message;
    });
    // See if the message is a valid transmit message
    bool is_tx_msg = std::any_of(device->tx_messages.begin(), device->tx_messages.end(), [&](const auto& msg) {
        return msg.get() == message;
    });
    
    *is_valid = (is_rx_msg || is_tx_msg) && message->message.get() != nullptr;
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_type_get(icsneoc2_device_t* device, icsneoc2_message_t* message, icsneoc2_msg_type_t* msg_type) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message || !msg_type) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    
    // Assign the message type
    *msg_type = static_cast<icsneoc2_msg_type_t>(message->message->getMsgType());

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_type_name_get(icsneoc2_msg_type_t msg_type, const char* value, uint32_t* value_length) {
    if (!value || !value_length) {
        return icsneoc2_error_invalid_parameters;
    }
    // Copy the string into value
    return safe_str_copy(value, value_length, Message::getMsgTypeName(static_cast<icsneo::MessageType>(msg_type))) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_bus_type_get(icsneoc2_device_t* device, icsneoc2_message_t* message, icsneoc2_msg_bus_type_t* bus_type) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message || !bus_type) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }

    // Make sure the message is a bus message
    icsneoc2_msg_type_t msg_type = static_cast<icsneoc2_msg_type_t>(message->message->getMsgType());
    if (msg_type != icsneoc2_msg_type_bus) {
        return icsneoc2_error_invalid_type;
    }
    // We can static cast here because we are relying on the type being correct at this point
    auto bus_message = static_cast<BusMessage*>(message->message.get());
    *bus_type = static_cast<icsneoc2_msg_type_t>(bus_message->getBusType());
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_bus_type_name_get(icsneoc2_msg_bus_type_t bus_type, const char* value, uint32_t* value_length) {
    if (!value || !value_length) {
        return icsneoc2_error_invalid_parameters;
    }
    auto bus_type_str = std::string(Network::GetTypeString(static_cast<Network::Type>(bus_type)));
    // Copy the string into value
    return safe_str_copy(value, value_length, bus_type_str) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_is_transmit(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message || !value) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    
    // Make sure the message is a bus message
    icsneoc2_msg_type_t msg_type = static_cast<icsneoc2_msg_type_t>(message->message->getMsgType());
    if (msg_type != icsneoc2_msg_type_bus) {
        return icsneoc2_error_invalid_type;
    }
    // We can static cast here because we are relying on the type being correct at this point
    auto bus_message = static_cast<BusMessage*>(message->message.get());
    *value = bus_message->transmitted;
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_netid_get(icsneoc2_device_t* device, icsneoc2_message_t* message, icsneoc2_netid_t* netid) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message || !netid) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }

    // Make sure the message is a bus message
    icsneoc2_msg_type_t msg_type = static_cast<icsneoc2_msg_type_t>(message->message->getMsgType());
    if (msg_type != icsneoc2_msg_type_bus) {
        return icsneoc2_error_invalid_type;
    }
    // We can static cast here because we are relying on the type being correct at this point
    auto bus_message = static_cast<BusMessage*>(message->message.get());
    *netid = static_cast<icsneoc2_netid_t>(bus_message->network.getNetID());
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_netid_name_get(icsneoc2_netid_t netid, const char* value, uint32_t* value_length) {
    if (!netid || !value || !value_length) {
        return icsneoc2_error_invalid_parameters;
    }
    auto netid_str = std::string(Network::GetNetIDString(static_cast<Network::NetID>(netid), true));
    // Copy the string into value
    return safe_str_copy(value, value_length, netid_str) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_netid_set(icsneoc2_device_t* device, icsneoc2_message_t* message, icsneoc2_netid_t netid) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    // Make sure the message has the data field, internal and bus currently have this.
    icsneoc2_msg_type_t msg_type = static_cast<icsneoc2_msg_type_t>(message->message->getMsgType());
    if (msg_type != icsneoc2_msg_type_internal && msg_type != icsneoc2_msg_type_bus) {
        return icsneoc2_error_invalid_type;
    }
    auto* internal_message = dynamic_cast<InternalMessage*>(message->message.get());
    if (!internal_message) {
        return icsneoc2_error_invalid_type;
    }
    internal_message->network = Network(static_cast<_icsneoc2_netid_t>(netid), true);

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_data_set(icsneoc2_device_t* device, icsneoc2_message_t* message, uint8_t* data, uint32_t data_length) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message | !data) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    // Make sure the message has the data field, internal and bus currently have this.
    icsneoc2_msg_type_t msg_type = static_cast<icsneoc2_msg_type_t>(message->message->getMsgType());
    if (msg_type != icsneoc2_msg_type_internal && msg_type != icsneoc2_msg_type_bus) {
        return icsneoc2_error_invalid_type;
    }
    auto* internal_message = dynamic_cast<InternalMessage*>(message->message.get());
    if (!internal_message) {
        return icsneoc2_error_invalid_type;
    }
    internal_message->data.clear();
    internal_message->data.resize(data_length);
    internal_message->data.shrink_to_fit();
    std::copy(data, data + data_length, internal_message->data.begin());

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_data_get(icsneoc2_device_t* device, icsneoc2_message_t* message, uint8_t* data, uint32_t* data_length) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message || !data || !data_length) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    // Make sure the message has the data field, internal and bus currently have this.
    icsneoc2_msg_type_t msg_type = static_cast<icsneoc2_msg_type_t>(message->message->getMsgType());
    if (msg_type != icsneoc2_msg_type_internal && msg_type != icsneoc2_msg_type_bus) {
        return icsneoc2_error_invalid_type;
    }
    auto* data_message = dynamic_cast<InternalMessage*>(message->message.get());
    if (!data_message) {
        return icsneoc2_error_invalid_type;
    }
    auto min_length = std::minmax(static_cast<uint32_t>(data_message->data.size()), *data_length).first;
    *data_length = min_length;
    std::copy(data_message->data.begin(), data_message->data.begin() + min_length, data);
    
    return icsneoc2_error_success;
}


ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_arbid_get(icsneoc2_device_t* device, icsneoc2_message_t* message, uint32_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message || !value) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    const auto* can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    *value = can_message->arbid;

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_arbid_set(icsneoc2_device_t* device, icsneoc2_message_t* message, uint32_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    auto* const can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    can_message->arbid = value;

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_dlc_get(icsneoc2_device_t* device, icsneoc2_message_t* message, int32_t* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    if (!value) {
        return icsneoc2_error_invalid_parameters;
    }
    const auto* can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    *value = static_cast<uint32_t>(can_message->dlcOnWire);
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_dlc_set(icsneoc2_device_t* device, icsneoc2_message_t* message, int32_t value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    auto* const can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    if (value < 0) {
        auto dlc_res = CAN_LengthToDLC(static_cast<uint8_t>(can_message->data.size()), can_message->isCANFD);
        can_message->dlcOnWire = dlc_res.value_or(0);
        return dlc_res.has_value() ? icsneoc2_error_success : icsneoc2_error_invalid_parameters;
    } else {
        can_message->dlcOnWire = static_cast<uint8_t>(value);
        return icsneoc2_error_success;
    }
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_is_remote(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    if (!value) {
        return icsneoc2_error_invalid_parameters;
    }
    const auto* can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    *value = can_message->isRemote;
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_set_remote(icsneoc2_device_t* device, icsneoc2_message_t* message, bool value) {
    if (!message) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    auto* const can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    can_message->isRemote = value;

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_is_extended(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    if (!value) {
        return icsneoc2_error_invalid_parameters;
    }
    const auto* can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    *value = can_message->isExtended;
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_extended_set(icsneoc2_device_t* device, icsneoc2_message_t* message, bool value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    auto* const can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    can_message->isExtended = value;

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_is_canfd(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    if (!value) {
        return icsneoc2_error_invalid_parameters;
    }
    const auto* can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    *value = can_message->isCANFD;
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_canfd_set(icsneoc2_device_t* device, icsneoc2_message_t* message, bool value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    auto* const can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    can_message->isCANFD = value;

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_baudrate_switch_get(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!message || !value) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    const auto* can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    *value = can_message->baudrateSwitch;
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_baudrate_switch_set(icsneoc2_device_t* device, icsneoc2_message_t* message, bool value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    auto* const can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    can_message->baudrateSwitch = value;

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_error_state_indicator_get(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    if (!value) {
        return icsneoc2_error_invalid_parameters;
    }
    const auto* can_message = dynamic_cast<CANMessage*>(message->message.get());
    if (!can_message) {
        return icsneoc2_error_invalid_type;
    }

    *value = can_message->errorStateIndicator;
    
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_create(icsneoc2_device_t* device, icsneoc2_message_t** messages, uint32_t messages_count) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!messages) {
        return icsneoc2_error_invalid_parameters;
    }
    auto dev = device->device;
    // Get the device messages
    for (uint32_t i = 0; i < messages_count; i++) {
        auto can_message = std::static_pointer_cast<Message>(std::make_shared<CANMessage>());
        auto message = std::make_shared<icsneoc2_message_t>(can_message, true);
        device->tx_messages.push_back(message);
        messages[i] = message.get();
    }

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_free(icsneoc2_device_t* device, icsneoc2_message_t* message) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    // Make sure the message is valid
    bool is_msg_valid = false;
    res = icsneoc2_message_is_valid(device, message, &is_msg_valid);
    if (res != icsneoc2_error_success) {
        return res;
    } else if (!is_msg_valid) {
        return icsneoc2_error_invalid_message;
    }
    bool removed = false;
    for (auto it = device->tx_messages.begin(); it != device->tx_messages.end(); it++) {
        if (it->get() == message) {
            device->tx_messages.erase(it);
            removed = true;
            message = nullptr;
            break;
        }
    }    
    return removed ? icsneoc2_error_success : icsneoc2_error_invalid_parameters;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_events_get(icsneoc2_event_t** events, uint32_t* events_count) {
    if (!events || !events_count) {
        return icsneoc2_error_invalid_parameters;
    }
    // Clear the device events
    g_events.clear();
    // Get the global events
    auto global_events = EventManager::GetInstance().get();
    // Get the mininum number of events
    auto min_size = std::minmax(static_cast<uint32_t>(global_events.size()), *events_count).first;
    *events_count = min_size;
    // Copy the events into the global event container
    for (uint32_t i = 0; i < min_size; i++) {
        auto e = icsneoc2_event_t {
            global_events[i],
        };
        g_events.push_back(e);
    }
    g_events.shrink_to_fit();
    // Copy the global events references into the events array
    for (uint32_t i = 0; i < min_size; i++) {
        events[i] = &g_events[i];
    }

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_events_get(icsneoc2_device_t* device, icsneoc2_event_t** events, uint32_t* events_count) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!events || !events_count) {
        return icsneoc2_error_invalid_parameters;
    }
    // Setup the event filter
    EventFilter filter(device->device.get());
    // Clear the device events
    device->events.clear();
    // Get the mininum number of events
    auto min_size = std::minmax(static_cast<uint32_t>(icsneo::EventCount(filter)), *events_count).first;
    *events_count = min_size;
    // GetEvents uses 0 as unlimited, where the API can't allocate anything.
    if (min_size == 0) {
        return icsneoc2_error_success;
    }
    // Copy the events into the device event container
    auto device_events = icsneo::GetEvents(filter, min_size);
    for (uint32_t i = 0; i < min_size; i++) {
        auto e = icsneoc2_event_t {
            device_events[i],
        };
        device->events.push_back(e);
    }
    device->events.shrink_to_fit();
    // Copy the device events references into the events array
    for (uint32_t i = 0; i < min_size; i++) {
        events[i] = &device->events[i];
    }

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_event_description_get(icsneoc2_event_t* event, const char* value, uint32_t* value_length) {
    if (!event || !value || !value_length) {
        return icsneoc2_error_invalid_parameters;
    }
    // TODO: Check if event is valid
    auto ev = event->event;
    // Copy the string into value
    return safe_str_copy(value, value_length, ev.describe()) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_rtc_get(icsneoc2_device_t* device, int64_t* unix_epoch) {
    if (!unix_epoch) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (auto rtc_time = device->device->getRTC(); rtc_time != std::nullopt) {
        *unix_epoch = std::chrono::duration_cast<std::chrono::seconds>(rtc_time->time_since_epoch()).count();
    } else {
        *unix_epoch = 0;
        return icsneoc2_error_rtc_failure;
    }
    return icsneoc2_error_success;
}
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_rtc_set(icsneoc2_device_t* device, int64_t unix_epoch) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!device->device->setRTC(std::chrono::system_clock::time_point(std::chrono::seconds(unix_epoch)))) {
        return icsneoc2_error_sync_rtc_failed;
    }
    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_load_default_settings(icsneoc2_device_t* device, bool save) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!device->device->settings->applyDefaults(!save)) {
        return icsneoc2_error_set_settings_failure;
    }

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_baudrate_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint64_t* baudrate) {
    if (!baudrate) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    *baudrate = device->device->settings->getBaudrateFor(Network(netid));
    if (*baudrate < 0) {
        return icsneoc2_error_invalid_type;
    }

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_baudrate_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint64_t baudrate, bool save) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!device->device->settings->setBaudrateFor(Network(netid), baudrate)) {
        return icsneoc2_error_set_settings_failure;
    }
    if (save) {
        if (!device->device->settings->apply()) {
            return icsneoc2_error_set_settings_failure;
        }
    }

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_canfd_baudrate_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint64_t* baudrate) {
    if (!baudrate) {
        return icsneoc2_error_invalid_parameters;
    }
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    *baudrate = device->device->settings->getFDBaudrateFor(Network(netid));
    if (*baudrate < 0) {
        return icsneoc2_error_invalid_type;
    }

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_canfd_baudrate_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint64_t baudrate, bool save) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    if (!device->device->settings->setFDBaudrateFor(Network(netid), baudrate)) {
        return icsneoc2_error_set_settings_failure;
    }
    if (save) {
        if (!device->device->settings->apply()) {
            return icsneoc2_error_set_settings_failure;
        }
    }

    return icsneoc2_error_success;
}

ICSNEOC2_API icsneoc2_error_t icsneoc2_device_supports_tc10(icsneoc2_device_t* device, bool* supported) {
    // Make sure the device is valid
    auto res = icsneoc2_device_is_valid(device);
    if (res != icsneoc2_error_success) {
        return res;
    }
    *supported = device->device->supportsTC10();

    return icsneoc2_error_success;
}
