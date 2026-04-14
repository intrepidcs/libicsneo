#include "icsneo/icsneoc2.h"
#include "icsneo/icsneoc2messages.h"
#include "icsneoc2_internal.h"
#include "icsneo/device/device.h"
#include "icsneo/device/devicefinder.h"
#include "icsneo/icsneocpp.h"
#include "icsneo/communication/io.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <optional>
#include <sstream>
#include <fstream>

using namespace icsneo;

bool safe_str_copy(char* dest, size_t* dest_size, std::string_view src) {
    if(!dest || !dest_size || *dest_size == 0) {
        return false;
    }
    // Need to save room for the null terminator
    *dest_size -= 1;
    try {
        *dest_size = src.copy(dest, *dest_size, 0);
		// Null terminate the buffer
		dest[*dest_size] = '\0';
		// If we truncated the string, return false to indicate not all data was copied.
		if(*dest_size != src.length()) {
            return false;
        }
        return true;
    } catch (std::out_of_range& ex) {
		// if pos > size()
		(void)ex;
		// Null terminate the buffer
		dest[0] = '\0';
		*dest_size = 0;
        return false;
    }
}

icsneoc2_error_t icsneoc2_error_code_get(icsneoc2_error_t error_code, char* value, size_t* value_length) {
	static const char* error_strings[] = {
		"Success",                          // icsneoc2_error_success
		"Invalid function parameters",      // icsneoc2_error_invalid_parameters
		"Open failed",                      // icsneoc2_error_open_failed
		"Going online failed",              // icsneoc2_error_go_online_failed
		"Enable message polling failed",    // icsneoc2_error_enable_message_polling_failed
		"Synchronizing RTC failed",         // icsneoc2_error_sync_rtc_failed
		"Getting messages failed",          // icsneoc2_error_get_messages_failed
		"Invalid type",                     // icsneoc2_error_invalid_type
		"RTC failure",                      // icsneoc2_error_rtc_failure
		"Getting settings failed",          // icsneoc2_error_get_settings_failure
		"Setting settings failed",          // icsneoc2_error_set_settings_failure
		"Transmitting message failed",      // icsneoc2_error_transmit_message_failed
		"String copy failed",               // icsneoc2_error_string_copy_failed
		"Invalid device",                   // icsneoc2_error_invalid_device
		"Invalid message",                  // icsneoc2_error_invalid_message
		"Out of memory",                    // icsneoc2_error_out_of_memory
		"Disk format failed",               // icsneoc2_error_format_disk_failed
		"Script start failed",              // icsneoc2_error_script_start_failed
		"Script stop failed",               // icsneoc2_error_script_stop_failed
		"Script clear failed",              // icsneoc2_error_script_clear_failed
		"Script upload failed",             // icsneoc2_error_script_upload_failed
		"Script load prepare failed",       // icsneoc2_error_script_load_prepare_failed
		"Close failed",                     // icsneoc2_error_close_failed
		"Reconnect failed",                 // icsneoc2_error_reconnect_failed
	};
	static_assert(std::size(error_strings) == icsneoc2_error_maxsize,
		"error_strings is out of sync with _icsneoc2_error_t enum - update both together");

	if(error_code >= icsneoc2_error_maxsize) {
		return icsneoc2_error_invalid_parameters;
	}

	if(!value || !value_length) {
		return icsneoc2_error_invalid_parameters;
	}

	return safe_str_copy(value, value_length, error_strings[error_code]) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_device_type_name_get(icsneoc2_devicetype_t device_type, char* value, size_t* value_length) {
	if(!value || !value_length) {
		return icsneoc2_error_invalid_parameters;
	}

	return safe_str_copy(value, value_length, DeviceType::GetGenericProductName(static_cast<DeviceType::Enum>(device_type))) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_device_enumerate(icsneoc2_devicetype_t device_type, icsneoc2_device_info_t** device_info) {
	if(!device_info) {
		return icsneoc2_error_invalid_parameters;
	}
	auto found_devices = DeviceFinder::FindAll();

	icsneoc2_device_info_t* head = nullptr;
	icsneoc2_device_info_t* tail = nullptr;
	for(auto& dev : found_devices) {
		if(device_type != 0 && static_cast<icsneoc2_devicetype_t>(dev->getType().getDeviceType()) != device_type) {
			continue;
		}
		auto* node = new (std::nothrow) icsneoc2_device_info_t;
		if(!node) {
			icsneoc2_enumeration_free(head);
			return icsneoc2_error_out_of_memory;
		}
		node->device = dev;
		node->next = nullptr;
		if(!head) {
			head = node;
		} else {
			tail->next = node;
		}
		tail = node;
	}

	*device_info = head;
	return icsneoc2_error_success;
}

void icsneoc2_enumeration_free(icsneoc2_device_info_t* devices) {
	while(devices) {
		auto* next = devices->next;
		delete devices;
		devices = next;
	}
}

icsneoc2_device_info_t* icsneoc2_device_info_next(const icsneoc2_device_info_t* device_info) {
	if(!device_info) {
		return nullptr;
	}
	return device_info->next;
}

icsneoc2_error_t icsneoc2_device_info_serial_get(const icsneoc2_device_info_t* device_info, char* serial, size_t* serial_length) {
	if(!device_info || !device_info->device || !serial || !serial_length) {
		return icsneoc2_error_invalid_parameters;
	}
	return safe_str_copy(serial, serial_length, device_info->device->getSerial())
		? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_device_info_type_get(const icsneoc2_device_info_t* device_info, icsneoc2_devicetype_t* type) {
	if(!device_info || !device_info->device || !type) {
		return icsneoc2_error_invalid_parameters;
	}
	*type = static_cast<icsneoc2_devicetype_t>(device_info->device->getType().getDeviceType());
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_info_type_name_get(const icsneoc2_device_info_t* device_info, char* typeName, size_t* type_name_length) {
	if(!device_info || !device_info->device || !typeName || !type_name_length) {
		return icsneoc2_error_invalid_parameters;
	}
	return safe_str_copy(typeName, type_name_length,
		DeviceType::GetGenericProductName(device_info->device->getType().getDeviceType()))
		? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_device_info_description_get(const icsneoc2_device_info_t* device_info, char* description, size_t* description_length) {
	if(!device_info || !device_info->device || !description || !description_length) {
		return icsneoc2_error_invalid_parameters;
	}
	return safe_str_copy(description, description_length, device_info->device->describe())
		? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_device_is_valid(const icsneoc2_device_t* device) {
	if(!device) {
		return icsneoc2_error_invalid_parameters;
	}

	if(!device->device) {
		return icsneoc2_error_invalid_device;
	}

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_is_open(const icsneoc2_device_t* device, bool* is_open) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!is_open) {
		return icsneoc2_error_invalid_parameters;
	}
	auto dev = device->device;
	*is_open = dev->isOpen();

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_create(const icsneoc2_device_info_t* device_info, icsneoc2_device_t** device) {
	if(!device_info || !device_info->device || !device) {
		return icsneoc2_error_invalid_parameters;
	}
	auto* new_device = new (std::nothrow) icsneoc2_device_t;
	if(!new_device) {
		return icsneoc2_error_out_of_memory;
	}
	new_device->device = device_info->device;
	*device = new_device;
	return icsneoc2_error_success;
}

static icsneoc2_error_t open_device_with_options(std::shared_ptr<Device> dev, icsneoc2_open_options_t options) {
	if(!dev) {
		return icsneoc2_error_invalid_device;
	}
	if(!dev->enableMessagePolling(std::make_optional<MessageFilter>())) {
		return icsneoc2_error_enable_message_polling_failed;
	}
	if(!dev->isOpen() && !dev->open()) {
		return icsneoc2_error_open_failed;
	}
	if((options & ICSNEOC2_OPEN_OPTIONS_SYNC_RTC) && !dev->setRTC(std::chrono::system_clock::now())) {
		dev->close();
		return icsneoc2_error_sync_rtc_failed;
	}
	if((options & ICSNEOC2_OPEN_OPTIONS_GO_ONLINE) && !dev->goOnline()) {
		dev->close();
		return icsneoc2_error_go_online_failed;
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_open(const icsneoc2_device_t* device, icsneoc2_open_options_t options) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	return open_device_with_options(device->device, options);
}

icsneoc2_error_t icsneoc2_device_open_serial(const char* serial, icsneoc2_open_options_t options, icsneoc2_device_t** device) {
	if(!serial || !device) {
		return icsneoc2_error_invalid_parameters;
	}
	icsneoc2_device_info_t* devs = nullptr;
	auto res = icsneoc2_device_enumerate(0, &devs);
	if(res != icsneoc2_error_success) {
		return res;
	}
	std::string_view target(serial);
	for(auto* cur = devs; cur; cur = cur->next) {
		if(cur->device && cur->device->getSerial() == target) {
			if (res = icsneoc2_device_create(cur, device); res != icsneoc2_error_success) {
				icsneoc2_enumeration_free(devs);
				return res;
			}
			res = open_device_with_options((*device)->device, options);
			if (res != icsneoc2_error_success) {
				icsneoc2_device_free(*device);
				*device = nullptr;
			}
			icsneoc2_enumeration_free(devs);
			return res;
		}
	}
	icsneoc2_enumeration_free(devs);
	return icsneoc2_error_invalid_device;
}

icsneoc2_error_t icsneoc2_device_open_first(icsneoc2_devicetype_t device_type, icsneoc2_open_options_t options, icsneoc2_device_t** device) {
	if(!device) {
		return icsneoc2_error_invalid_parameters;
	}
	icsneoc2_device_info_t* devs = nullptr;
	auto res = icsneoc2_device_enumerate(device_type, &devs);
	if(res != icsneoc2_error_success) {
		return res;
	}
	for(auto* cur = devs; cur; cur = cur->next) {
		if(cur->device && !cur->device->isOpen()) {
			if (res = icsneoc2_device_create(cur, device); res != icsneoc2_error_success) {
				icsneoc2_enumeration_free(devs);
				return res;
			}
			res = open_device_with_options((*device)->device, options);
			if (res != icsneoc2_error_success) {
				icsneoc2_device_free(*device);
				*device = nullptr;
			}
			icsneoc2_enumeration_free(devs);
			return res;
		}
	}
	icsneoc2_enumeration_free(devs);
	return icsneoc2_error_invalid_device;
}

icsneoc2_error_t icsneoc2_device_reconnect(icsneoc2_device_t* device, icsneoc2_open_options_t options, uint32_t timeout_ms) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	// If the device is currently open, close it first before trying to reconnect
	if (device->device->isOpen()) {
		res = icsneoc2_device_close(device);
		if(res != icsneoc2_error_success) {
			return res;
		}
	}
	
	const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
	while(std::chrono::steady_clock::now() < timeout) {
		icsneoc2_device_t* new_device = nullptr;
		res = icsneoc2_device_open_serial(device->device->getSerial().c_str(), options, &new_device);
		if(res == icsneoc2_error_success) {
			device->device = new_device->device;
			delete new_device;
			return icsneoc2_error_success;
		}
		// Avoid busy looping
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return icsneoc2_error_reconnect_failed;
}

icsneoc2_error_t icsneoc2_device_close(icsneoc2_device_t* device) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	auto dev = device->device;
	if(!dev->isOpen()) {
		return icsneoc2_error_success;
	}
	
	return dev->close() ? icsneoc2_error_success : icsneoc2_error_close_failed;
}

icsneoc2_error_t icsneoc2_device_free(icsneoc2_device_t* device) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if (device->device->isOpen()) {
		res = icsneoc2_device_close(device);
		if(res != icsneoc2_error_success) {
			return res;
		}
	}
	delete device;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_description_get(const icsneoc2_device_t* device, char* value, size_t* value_length) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	// Copy the string into value
	return safe_str_copy(value, value_length, device->device->describe()) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_device_type_get(const icsneoc2_device_t* device, icsneoc2_devicetype_t* value) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	auto dev = device->device;
	*value = static_cast<icsneoc2_devicetype_t>(dev->getType().getDeviceType());

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_serial_get(const icsneoc2_device_t* device, char* value, size_t* value_length) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	auto dev = device->device;
	// Copy the string into value
	return safe_str_copy(value, value_length, dev->getSerial()) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_device_pcb_serial_get(const icsneoc2_device_t* device, uint8_t* value, size_t* value_length) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!value_length) {
		return icsneoc2_error_invalid_parameters;
	}
	auto pcbSerial = device->device->getPCBSerial();
	if(!pcbSerial.has_value()) {
		return icsneoc2_error_invalid_type;
	}
	const auto& data = *pcbSerial;
	if(value) {
		size_t copyLen = std::min(*value_length, data.size());
		std::copy(data.begin(), data.begin() + copyLen, value);
	}
	*value_length = data.size();
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_mac_address_get(const icsneoc2_device_t* device, uint8_t* value, size_t* value_length) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!value_length) {
		return icsneoc2_error_invalid_parameters;
	}
	auto macAddress = device->device->getMACAddress();
	if(!macAddress.has_value()) {
		return icsneoc2_error_invalid_type;
	}
	const auto& data = *macAddress;
	if(value) {
		size_t copyLen = std::min(*value_length, data.size());
		std::copy(data.begin(), data.begin() + copyLen, value);
	}
	*value_length = data.size();
	return icsneoc2_error_success;
}


icsneoc2_error_t icsneoc2_device_go_online(const icsneoc2_device_t* device, bool go_online) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	auto dev = device->device;
	// Go online
	if(go_online && dev->goOnline()) {
		return icsneoc2_error_success;
	}
	// Go offline
	if(!go_online && dev->goOffline()) {
		return icsneoc2_error_success;
	}

	return icsneoc2_error_go_online_failed;
}

icsneoc2_error_t icsneoc2_device_is_online(const icsneoc2_device_t* device, bool* is_online) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!is_online) {
		return icsneoc2_error_invalid_parameters;
	}
	auto dev = device->device;
	*is_online = dev->isOnline();

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_is_online_supported(const icsneoc2_device_t* device, bool* is_online_supported) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!is_online_supported) {
		return icsneoc2_error_invalid_parameters;
	}
	auto dev = device->device;
	*is_online_supported = dev->isOnlineSupported();

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_message_polling_limit_set(const icsneoc2_device_t* device, uint32_t limit) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	auto dev = device->device;
	dev->setPollingMessageLimit(static_cast<size_t>(limit));

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_message_polling_limit_get(const icsneoc2_device_t* device, uint32_t* limit) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!limit) {
		return icsneoc2_error_invalid_parameters;
	}
	auto dev = device->device;
	*limit = static_cast<uint32_t>(dev->getPollingMessageLimit());

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_timestamp_resolution_get(icsneoc2_device_t* device, uint32_t* resolution) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!resolution) {
		return icsneoc2_error_invalid_parameters;
	}
	auto dev = device->device;
	*resolution = static_cast<uint32_t>(dev->getTimestampResolution());

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_message_get(const icsneoc2_device_t* device, icsneoc2_message_t** message, uint32_t timeout_ms) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}

	std::vector<std::shared_ptr<Message>> msgs(1);
	if(!device->device->getMessages(msgs, 1, std::chrono::milliseconds(timeout_ms))) {
		return icsneoc2_error_get_messages_failed;
	}

	if(!msgs.empty()) {
		*message = new (std::nothrow) icsneoc2_message_t;
		if(!*message) {
			return icsneoc2_error_out_of_memory;
		}
		(*message)->message = msgs[0];
	} else {
		*message = nullptr;
	}

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_message_transmit(const icsneoc2_device_t* device, const icsneoc2_message_t* message) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!message) {
		return icsneoc2_error_invalid_parameters;
	}
	auto dev = device->device;
	auto success = dev->transmit(std::static_pointer_cast<Frame>(message->message));

	return success ? icsneoc2_error_success : icsneoc2_error_transmit_message_failed;
}

icsneoc2_error_t icsneoc2_network_type_name_get(icsneoc2_network_type_t network_type, char* value, size_t* value_length) {
	if(!value || !value_length) {
		return icsneoc2_error_invalid_parameters;
	}
	auto network_type_str = std::string(Network::GetTypeString(static_cast<Network::Type>(network_type)));
	// Copy the string into value
	return safe_str_copy(value, value_length, network_type_str) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_event_get(icsneoc2_event_t** event, const icsneoc2_device_t* device) {
	if(!event) {
		return icsneoc2_error_invalid_parameters;
	}

	if(device && !device->device) {
		return icsneoc2_error_invalid_device;
	}

	const auto* dev = device ? device->device.get() : nullptr;
	EventFilter filter(dev);
	auto events = icsneo::GetEvents(filter, 1);

	if(events.empty()) {
		*event = nullptr;
		return icsneoc2_error_success;
	}

	*event = new (std::nothrow) icsneoc2_event_t;
	if(!*event) {
		return icsneoc2_error_out_of_memory;
	}
	(*event)->event = events.front();

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_event_free(icsneoc2_event_t* event) {
	if(!event) {
		return icsneoc2_error_invalid_parameters;
	}
	delete event;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_event_description_get(const icsneoc2_event_t* event, char* value, size_t* value_length) {
	if(!event || !value || !value_length) {
		return icsneoc2_error_invalid_parameters;
	}
	// Copy the string into value
	return safe_str_copy(value, value_length, event->event.describe()) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_device_rtc_get(const icsneoc2_device_t* device, int64_t* unix_epoch) {
	if(!unix_epoch) {
		return icsneoc2_error_invalid_parameters;
	}
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(auto rtc_time = device->device->getRTC(); rtc_time != std::nullopt) {
		*unix_epoch = std::chrono::duration_cast<std::chrono::seconds>(rtc_time->time_since_epoch()).count();
	} else {
		*unix_epoch = 0;
		return icsneoc2_error_rtc_failure;
	}
	return icsneoc2_error_success;
}
icsneoc2_error_t icsneoc2_device_rtc_set(const icsneoc2_device_t* device, int64_t unix_epoch) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!device->device->setRTC(std::chrono::system_clock::time_point(std::chrono::seconds(unix_epoch)))) {
		return icsneoc2_error_sync_rtc_failed;
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_supports_tc10(const icsneoc2_device_t* device, bool* supported) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	*supported = device->device->supportsTC10();

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_digital_io_get(const icsneoc2_device_t* device, icsneoc2_io_type_t type, uint32_t number, bool* value) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!value) {
		return icsneoc2_error_invalid_parameters;
	}

	// Convert icsneoc2 IO type to C++ IO enum (they match numerically)
	auto cpp_io_type = static_cast<icsneo::IO>(type);

	// Get the digital IO value
	const std::optional<bool> val = device->device->getDigitalIO(cpp_io_type, number);
	if(!val.has_value()) {
		return icsneoc2_error_invalid_type;
	}

	*value = *val;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_digital_io_set(const icsneoc2_device_t* device, icsneoc2_io_type_t type, uint32_t number, bool value) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}

	// Convert icsneoc2 IO type to C++ IO enum (they match numerically)
	auto cpp_io_type = static_cast<icsneo::IO>(type);

	// Set the digital IO value
	if(!device->device->setDigitalIO(cpp_io_type, number, value)) {
		return icsneoc2_error_set_settings_failure;
	}

	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_version_get(char* value, size_t* value_length) {
	if(!value || !value_length) {
		return icsneoc2_error_invalid_parameters;
	}

	// Get version from libicsneo
	auto version = icsneo::GetVersion();

	// Format version string (e.g., "v1.0.0")
	std::ostringstream version_stream;
	version_stream << 'v' << version.major << '.' << version.minor << '.' << version.patch;
	if(version.metadata[0] != '\0') {
		version_stream << '+' << version.metadata;
	}

	auto version_str = version_stream.str();
	return safe_str_copy(value, value_length, version_str) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_serial_num_to_string(uint32_t num, char* str, size_t* str_length) {
	if(!str || !str_length) {
		return icsneoc2_error_invalid_parameters;
	}

	// Convert using Device static method
	auto result = Device::SerialNumToString(num);
	return safe_str_copy(str, str_length, result) ? icsneoc2_error_success : icsneoc2_error_string_copy_failed;
}

icsneoc2_error_t icsneoc2_serial_string_to_num(char* str, size_t str_length, uint32_t* num) {
	if(!str || !num) {
		return icsneoc2_error_invalid_parameters;
	}

	// Convert using Device static method
	*num = Device::SerialStringToNum(std::string(str, str_length));
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_disk_count_get(const icsneoc2_device_t* device, size_t* count) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!count) {
		return icsneoc2_error_invalid_parameters;
	}
	*count = device->device->getDiskCount();
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_supports_disk_formatting(const icsneoc2_device_t* device, bool* supported) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!supported) {
		return icsneoc2_error_invalid_parameters;
	}
	*supported = device->device->supportsDiskFormatting();
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_disk_details_get(const icsneoc2_device_t* device, icsneoc2_disk_details_t** disk_details) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!disk_details) {
		return icsneoc2_error_invalid_parameters;
	}
	auto details = device->device->getDiskDetails();
	if(!details) {
		return icsneoc2_error_invalid_device;
	}
	auto* out = new (std::nothrow) icsneoc2_disk_details_t;
	if(!out) {
		return icsneoc2_error_out_of_memory;
	}
	out->details = details;
	*disk_details = out;
	return icsneoc2_error_success;
}

void icsneoc2_disk_details_free(icsneoc2_disk_details_t* disk_details) {
	delete disk_details;
}

icsneoc2_error_t icsneoc2_disk_details_count_get(const icsneoc2_disk_details_t* disk_details, size_t* count) {
	if(!disk_details || !disk_details->details || !count) {
		return icsneoc2_error_invalid_parameters;
	}
	*count = disk_details->details->disks.size();
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_disk_details_layout_get(const icsneoc2_disk_details_t* disk_details, icsneoc2_disk_layout_t* layout) {
	if(!disk_details || !disk_details->details || !layout) {
		return icsneoc2_error_invalid_parameters;
	}
	*layout = static_cast<icsneoc2_disk_layout_t>(disk_details->details->layout);
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_disk_details_layout_set(const icsneoc2_disk_details_t* disk_details, icsneoc2_disk_layout_t layout) {
	if(!disk_details || !disk_details->details) {
		return icsneoc2_error_invalid_parameters;
	}
	disk_details->details->layout = static_cast<DiskLayout>(layout);
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_disk_details_flags_get(const icsneoc2_disk_details_t* disk_details, size_t index, icsneoc2_disk_format_flags_t* flags) {
	if(!disk_details || !disk_details->details || !flags) {
		return icsneoc2_error_invalid_parameters;
	}
	if(index >= disk_details->details->disks.size()) {
		return icsneoc2_error_invalid_parameters;
	}
	const auto& disk = disk_details->details->disks[index];
	icsneoc2_disk_format_flags_t result = 0;
	if(disk.present) result |= ICSNEOC2_DISK_FORMAT_FLAGS_PRESENT;
	if(disk.initialized) result |= ICSNEOC2_DISK_FORMAT_FLAGS_INITIALIZED;
	if(disk.formatted) result |= ICSNEOC2_DISK_FORMAT_FLAGS_FORMATTED;
	*flags = result;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_disk_details_flags_set(const icsneoc2_disk_details_t* disk_details, size_t index, icsneoc2_disk_format_flags_t flags) {
	if(!disk_details || !disk_details->details || !flags) {
		return icsneoc2_error_invalid_parameters;
	}
	if(index >= disk_details->details->disks.size()) {
		return icsneoc2_error_invalid_parameters;
	}
	auto& disk = disk_details->details->disks[index];
	disk.present = (flags & ICSNEOC2_DISK_FORMAT_FLAGS_PRESENT) != 0;
	disk.initialized = (flags & ICSNEOC2_DISK_FORMAT_FLAGS_INITIALIZED) != 0;
	disk.formatted = (flags & ICSNEOC2_DISK_FORMAT_FLAGS_FORMATTED) != 0;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_disk_details_size_get(const icsneoc2_disk_details_t* disk_details, size_t index, uint64_t* sectors, uint64_t* bytes_per_sector) {
	if(!disk_details || !disk_details->details || !sectors || !bytes_per_sector) {
		return icsneoc2_error_invalid_parameters;
	}
	if(index >= disk_details->details->disks.size()) {
		return icsneoc2_error_invalid_parameters;
	}
	const auto& disk = disk_details->details->disks[index];
	*sectors = disk.sectors;
	*bytes_per_sector = disk.bytesPerSector;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_disk_details_full_format_get(const icsneoc2_disk_details_t* disk_details, bool* full_format) {
	if(!disk_details || !disk_details->details || !full_format) {
		return icsneoc2_error_invalid_parameters;
	}
	*full_format = disk_details->details->fullFormat;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_disk_details_full_format_set(const icsneoc2_disk_details_t* disk_details, bool full_format) {
	if(!disk_details || !disk_details->details) {
		return icsneoc2_error_invalid_parameters;
	}
	disk_details->details->fullFormat = full_format;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_format_disk(const icsneoc2_device_t* device, icsneoc2_disk_details_t* disk_details, icsneoc2_disk_format_progress_fn progress_callback, void* user_data) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!disk_details || !disk_details->details) {
		return icsneoc2_error_invalid_parameters;
	}

	Device::DiskFormatProgress handler;
	if(progress_callback) {
		handler = [progress_callback, user_data](uint64_t sectorsFormatted, uint64_t sectorsTotal) -> Device::DiskFormatDirective {
			auto directive = progress_callback(sectorsFormatted, sectorsTotal, user_data);
			if(directive == icsneoc2_disk_format_directive_stop) {
				return Device::DiskFormatDirective::Stop;
			}
			return Device::DiskFormatDirective::Continue;
		};
	}

	if(!device->device->formatDisk(*disk_details->details, handler)) {
		return icsneoc2_error_format_disk_failed;
	}
	return icsneoc2_error_success;
}

static icsneoc2_error_t get_supported_networks(const icsneoc2_device_t* device, const std::vector<Network>& nets, icsneoc2_netid_t* networks, size_t* count) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!count) {
		return icsneoc2_error_invalid_parameters;
	}
	if(!networks) {
		*count = nets.size();
		return icsneoc2_error_success;
	}
	if(*count < nets.size()) {
		return icsneoc2_error_invalid_parameters;
	}
	size_t to_copy = std::min<size_t>(*count, nets.size());
	for(size_t i = 0; i < to_copy; i++) {
		networks[i] = static_cast<icsneoc2_netid_t>(nets[i].getNetID());
	}
	*count = to_copy;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_supported_rx_networks_get(const icsneoc2_device_t* device, icsneoc2_netid_t* networks, size_t* count) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	return get_supported_networks(device, device->device->getSupportedRXNetworks(), networks, count);
}

icsneoc2_error_t icsneoc2_device_supported_tx_networks_get(const icsneoc2_device_t* device, icsneoc2_netid_t* networks, size_t* count) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	return get_supported_networks(device, device->device->getSupportedTXNetworks(), networks, count);
}

icsneoc2_error_t icsneoc2_device_supports_coremini_script(const icsneoc2_device_t* device, bool* supported) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!supported) {
		return icsneoc2_error_invalid_parameters;
	}
	*supported = device->device->supportsCoreminiScript();
	return icsneoc2_error_success;
}

static Disk::MemoryType to_memory_type(icsneoc2_memory_type_t type) {
	switch(type) {
		case icsneoc2_memory_type_flash:
			return Disk::MemoryType::Flash;
		case icsneoc2_memory_type_sd:
		default:
			return Disk::MemoryType::SD;
	}
}

icsneoc2_error_t icsneoc2_device_script_start(const icsneoc2_device_t* device, icsneoc2_memory_type_t memory_type) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!device->device->startScript(to_memory_type(memory_type))) {
		return icsneoc2_error_script_start_failed;
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_script_stop(const icsneoc2_device_t* device) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!device->device->stopScript()) {
		return icsneoc2_error_script_stop_failed;
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_script_clear(const icsneoc2_device_t* device, icsneoc2_memory_type_t memory_type) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!device->device->clearScript(to_memory_type(memory_type))) {
		return icsneoc2_error_script_clear_failed;
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_script_prepare_load(const icsneoc2_device_t* device, int8_t* status) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!status) {
		return icsneoc2_error_invalid_parameters;
	}
	*status = device->device->prepareScriptLoad();
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_coremini_upload_file(const icsneoc2_device_t* device, const char* path, icsneoc2_memory_type_t memory_type) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!path) {
		return icsneoc2_error_invalid_parameters;
	}
	std::ifstream ifs(path, std::ios::binary);
	if(!ifs.is_open()) {
		return icsneoc2_error_invalid_parameters;
	}
	if(!device->device->uploadCoremini(ifs, to_memory_type(memory_type))) {
		return icsneoc2_error_script_upload_failed;
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_coremini_upload(const icsneoc2_device_t* device, const uint8_t* data, size_t length, icsneoc2_memory_type_t memory_type) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!data || length == 0) {
		return icsneoc2_error_invalid_parameters;
	}
	std::string buf(reinterpret_cast<const char*>(data), length);
	std::istringstream iss(buf, std::ios::binary);
	if(!device->device->uploadCoremini(iss, to_memory_type(memory_type))) {
		return icsneoc2_error_script_upload_failed;
	}
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_device_script_status_get(const icsneoc2_device_t* device, icsneoc2_script_status_t** script_status) {
	auto res = icsneoc2_device_is_valid(device);
	if(res != icsneoc2_error_success) {
		return res;
	}
	if(!script_status) {
		return icsneoc2_error_invalid_parameters;
	}
	auto status = device->device->getScriptStatus();
	if(!status) {
		return icsneoc2_error_invalid_device;
	}
	auto* out = new (std::nothrow) icsneoc2_script_status_t;
	if(!out) {
		return icsneoc2_error_out_of_memory;
	}
	out->status = status;
	*script_status = out;
	return icsneoc2_error_success;
}

void icsneoc2_script_status_free(icsneoc2_script_status_t* script_status) {
	delete script_status;
}

icsneoc2_error_t icsneoc2_script_status_is_coremini_running(const icsneoc2_script_status_t* script_status, bool* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->isCoreminiRunning;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_is_encrypted(const icsneoc2_script_status_t* script_status, bool* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->isEncrypted;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_sector_overflows_get(const icsneoc2_script_status_t* script_status, uint32_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->sectorOverflows;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_remaining_sector_buffers_get(const icsneoc2_script_status_t* script_status, uint32_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->numRemainingSectorBuffers;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_last_sector_get(const icsneoc2_script_status_t* script_status, uint32_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->lastSector;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_read_bin_size_get(const icsneoc2_script_status_t* script_status, uint32_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->readBinSize;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_min_sector_get(const icsneoc2_script_status_t* script_status, uint32_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->minSector;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_max_sector_get(const icsneoc2_script_status_t* script_status, uint32_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->maxSector;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_current_sector_get(const icsneoc2_script_status_t* script_status, uint32_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->currentSector;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_coremini_create_time_get(const icsneoc2_script_status_t* script_status, uint64_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->coreminiCreateTime;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_file_checksum_get(const icsneoc2_script_status_t* script_status, uint16_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->fileChecksum;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_coremini_version_get(const icsneoc2_script_status_t* script_status, uint16_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->coreminiVersion;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_coremini_header_size_get(const icsneoc2_script_status_t* script_status, uint16_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->coreminiHeaderSize;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_diagnostic_error_code_get(const icsneoc2_script_status_t* script_status, uint8_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->diagnosticErrorCode;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_diagnostic_error_code_count_get(const icsneoc2_script_status_t* script_status, uint8_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->diagnosticErrorCodeCount;
	return icsneoc2_error_success;
}

icsneoc2_error_t icsneoc2_script_status_max_coremini_size_kb_get(const icsneoc2_script_status_t* script_status, uint16_t* value) {
	if(!script_status || !script_status->status || !value) {
		return icsneoc2_error_invalid_parameters;
	}
	*value = script_status->status->maxCoreminiSizeKB;
	return icsneoc2_error_success;
}
