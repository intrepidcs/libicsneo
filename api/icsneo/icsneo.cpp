#include <icsneo/icsneo.h>
#include <icsneo/device/device.h>
#include "icsneo/device/devicefinder.h"

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/canmessage.h"
#include "icsneo/communication/message/linmessage.h"
#include "icsneo/communication/message/ethernetmessage.h"

#include <string>
#include <vector>
#include <algorithm>

using namespace icsneo;

typedef struct icsneo_device_t {
    std::shared_ptr<Device> device;
    std::vector<std::shared_ptr<icsneo_message_t>> messages;

    icsneo_open_options_t options;
} icsneo_device_t;

typedef struct icsneo_message_t {
    std::shared_ptr<Message> message;
} icsneo_message_t;


static std::vector<std::shared_ptr<icsneo_device_t>> g_devices;

ICSNEO_API icsneo_error_t icsneo_error_code(icsneo_error_t error_code, const char* value, uint32_t* value_length) {
    if (!value || !value_length) {
        return icsneo_error_invalid_parameters;
    }

    std::string error;
    switch (error_code) {
        case icsneo_error_success:
            error = "success";
            break;
        case icsneo_error_invalid_parameters:
            error = "invalid parameters";
            break;
        case icsneo_error_open_failed:
            error = "open failed";
            break;
        case icsneo_error_go_online_failed:
            error = "go online failed";
            break;
        case icsneo_error_enable_message_polling_failed:
            error = "enable message polling failed";
            break;
        case icsneo_error_sync_rtc_failed:
            error = "sync RTC failed";
            break;
        // Don't default, let the compiler warn us if we forget to handle an error code
    }
    // Find the minimum length of the error string and set value_length
    auto min_length = std::minmax(static_cast<uint32_t>(error.length()), *value_length).first;
    *value_length = min_length;
    // Copy the string into value
    strncpy(const_cast<char *>(value), error.c_str(), min_length);

    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_device_type_from_type(icsneo_devicetype_t device_type, const char* value, uint32_t* value_length) {
    if (!value || !value_length) {
        return icsneo_error_invalid_parameters;
    }

    auto device_type_str = DeviceType::getGenericProductName(device_type);
    // Find the minimum length of the device type string and set value_length
    auto min_length = std::minmax(static_cast<uint32_t>(device_type_str.length()), *value_length).first;
    *value_length = min_length;
    // Copy the string into value
    strncpy(const_cast<char *>(value), device_type_str.c_str(), min_length);

    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_find(icsneo_device_t** devices, uint32_t* devices_count, void* reserved) {
    if (!devices || !devices_count) {
        return icsneo_error_invalid_parameters;
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
            auto device = std::make_shared<icsneo_device_t>();
            device->device = found_device;
            device->options = icsneo_open_options_go_online | icsneo_open_options_enable_message_polling | icsneo_open_options_sync_rtc | icsneo_open_options_enable_auto_update;
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
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_device_is_valid(icsneo_device_t* device) {
    if (!device) {
        return icsneo_error_invalid_parameters;
    }

    return !device->device ? icsneo_error_invalid_parameters : icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_get_open_options(icsneo_device_t* device, icsneo_open_options_t* options) {
    if (!device || !options) {
        return icsneo_error_invalid_parameters;
    }
    *options = device->options;

    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_set_open_options(icsneo_device_t* device, icsneo_open_options_t options) {
    if (!device) {
        return icsneo_error_invalid_parameters;
    }

    device->options = options;

    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_open(icsneo_device_t* device) {
    if (!device || !device->device) {
        return icsneo_error_invalid_parameters;
    }
    // Open the device
    auto dev = device->device;
    // Nothing to do if we are already open
    if (dev->isOpen()) {
        return icsneo_error_success;
    }
    // TODO: OpenFlags and OpenStatusHandler
    // Open the device
    if (!dev->open()) {
        return icsneo_error_open_failed;
    }
    // Sync RTC
    if ((device->options & icsneo_open_options_sync_rtc) == icsneo_open_options_sync_rtc && !dev->setRTC(std::chrono::system_clock::now())) {
        dev->close();
        return icsneo_error_sync_rtc_failed;
    }
    // Enable message polling
    if ((device->options & icsneo_open_options_enable_message_polling) == icsneo_open_options_enable_message_polling && !dev->enableMessagePolling()) {
        dev->close();
        return icsneo_error_enable_message_polling_failed;
    }
    // Go online
    if ((device->options & icsneo_open_options_go_online) == icsneo_open_options_go_online && !dev->goOnline()) {
        dev->close();
        return icsneo_error_go_online_failed;
    }
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_close(icsneo_device_t* device) {
    if (!device || !device->device) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    if (!dev->isOpen()) {
        return icsneo_error_success;
    }
    dev->close();
    // Clear out old messages
    device->messages.clear();
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_device_describe(icsneo_device_t* device, const char* value, uint32_t* value_length) {
    if (!device || !device->device) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    // Get and set the length
    auto min_length = std::minmax(static_cast<uint32_t>(dev->describe().length()), *value_length).first;
    *value_length = min_length;
    // Copy the string into value
    strncpy(const_cast<char *>(value), dev->describe().c_str(), min_length);

    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_device_type(icsneo_device_t* device, icsneo_devicetype_t* value) {
    if (!device || !device->device) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    *value = dev->getType().getDeviceType();
    
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_device_serial(icsneo_device_t* device, const char* value, uint32_t* value_length) {
    if (!device || !device->device) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    // Get and set the length
    auto min_length = std::minmax(static_cast<uint32_t>(dev->getSerial().length()), *value_length).first;
    *value_length = min_length;
    // Copy the string into value
    strncpy(const_cast<char *>(value), dev->getSerial().c_str(), min_length);
    
    return icsneo_error_success;
}


ICSNEO_API icsneo_error_t icsneo_go_online(icsneo_device_t* device, bool go_online) {
    if (!device) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    // Go online
    if (go_online && dev->goOnline()) {
        return icsneo_error_success;
    }
    // Go offline
    if (!go_online && dev->goOffline()) {
        return icsneo_error_success;
    }

    return icsneo_error_go_online_failed;
}

ICSNEO_API icsneo_error_t icsneo_is_online(icsneo_device_t* device, bool* is_online) {
    if (!device || !is_online) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    *is_online = dev->isOnline();
    
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_set_message_polling(icsneo_device_t* device, bool enable) {
    if (!device) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    // Enable message polling
    if (enable && dev->enableMessagePolling()) {
        return icsneo_error_success;
    } 
    // Disable message polling
    if (!enable && dev->disableMessagePolling()) {
        return icsneo_error_success;
    }
    
    return icsneo_error_enable_message_polling_failed;
}

ICSNEO_API icsneo_error_t icsneo_get_message_polling(icsneo_device_t* device, bool* is_enabled) {
    if (!device || !is_enabled) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    *is_enabled = dev->isMessagePollingEnabled();
    
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_set_message_polling_limit(icsneo_device_t* device, uint32_t limit) {
    if (!device) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    dev->setPollingMessageLimit(static_cast<size_t>(limit));
    
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_get_message_polling_limit(icsneo_device_t* device, uint32_t* limit) {
    if (!device || !limit) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    *limit = static_cast<uint32_t>(dev->getPollingMessageLimit());
    
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_get_message_count(icsneo_device_t* device, uint32_t* count)  {
    if (!device || !count) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    *count = static_cast<uint32_t>(dev->getCurrentMessageCount());
    
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_get_timestamp_resolution(icsneo_device_t* device, uint32_t* resolution) {
    if (!device || !resolution) {
        return icsneo_error_invalid_parameters;
    }
    auto dev = device->device;
    *resolution = static_cast<uint32_t>(dev->getTimestampResolution());
    
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_get_messages(icsneo_device_t* device, icsneo_message_t** messages, uint32_t* messages_count, uint32_t timeout_ms) {    
    if (!device || !messages || !messages_count) {
        return icsneo_error_invalid_parameters;
    }
    // TODO: Check if device is valid
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
        return icsneo_error_get_messages_failed;
    }
    // Find the minimum number of messages
    uint32_t min_size = std::minmax(static_cast<uint32_t>(queried_messages.size()), *messages_count).first;
    *messages_count = min_size;

    // Copy the messages into our global message container
    device->messages.clear();
    for (auto& message : queried_messages) {
        auto message_t = std::make_shared<icsneo_message_t>();
        message_t->message = message;
        device->messages.push_back(message_t);
    }

    // Copy the messages into the output array
    for (uint32_t i = 0; i < min_size; i++) {
        messages[i] = device->messages[i].get();
    }
    
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_is_message_valid(icsneo_device_t* device, icsneo_message_t* message, bool* is_valid) {
    if (!device || !message || !is_valid) {
        return icsneo_error_invalid_parameters;
    }
    // TODO: Check if device is valid
    *is_valid = std::find_if(device->messages.begin(), device->messages.end(), [&](const auto& msg) {
        return msg->message == message->message;
    }) == device->messages.end();
    
    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_message_get_type(icsneo_device_t* device, icsneo_message_t* message, icsneo_msg_type_t* msg_type) {
    if (!device || !message || !msg_type) {
        return icsneo_error_invalid_parameters;
    }
    // TODO: Check if message is valid
    
    // Assign the message type
    *msg_type = message->message->getMsgType();

    return icsneo_error_success;
}

ICSNEO_API icsneo_error_t icsneo_message_get_bus_type(icsneo_device_t* device, icsneo_message_t* message, icsneo_msg_bus_type_t* bus_type) {
    if (!device || !message || !bus_type) {
        return icsneo_error_invalid_parameters;
    }
    // TODO: Check if message is valid

    // Make sure the message is a bus message
    if (message->message->getMsgType() != icsneo_msg_type_bus) {
        return icsneo_error_invalid_type;
    }
    // We can static cast here because we are relying on the type being correct at this point
    auto bus_message = static_cast<BusMessage*>(message->message.get());
    *bus_type = bus_message->getBusType();
    
    return icsneo_error_success;
}
