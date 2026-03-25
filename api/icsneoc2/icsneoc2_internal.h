// This header is for internal icsneoc2 use only, it should not be included by users of the API.
#pragma once

#include "icsneo/icsneoc2.h"
#include "icsneo/device/device.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/scriptstatusmessage.h"
#include "icsneo/api/event.h"
#include "icsneo/disk/diskdetails.h"

#include <memory>
#include <vector>

using namespace icsneo;

typedef struct icsneoc2_message_t {
	std::shared_ptr<Message> message;
} icsneoc2_message_t;

typedef struct icsneoc2_event_t {
	APIEvent event;
} icsneoc2_event_t;

typedef struct icsneoc2_device_info_t {
	std::shared_ptr<Device> device;
	icsneoc2_device_info_t* next;
} icsneoc2_device_info_t;

typedef struct icsneoc2_device_t {
	std::shared_ptr<Device> device;
} icsneoc2_device_t;

typedef struct icsneoc2_disk_details_t {
	std::shared_ptr<DiskDetails> details;
} icsneoc2_disk_details_t;

typedef struct icsneoc2_script_status_t {
	std::shared_ptr<ScriptStatusMessage> status;
} icsneoc2_script_status_t;

/**
 * Safely copies a std::string to a char array.
 *
 * @param dest The buffer to copy the string into
 * @param dest_size* The size of the buffer. Will be modified to the length of the string without the null terminator.
 * @param src The string to copy
 *
 * @return true if the string was successfully copied, false otherwise
 *
 * @note This function always null terminates the buffer, even if the string is too long. 
 * In the case of truncation, dest_size will reflect the truncated length (not including the null terminator).
 */
bool safe_str_copy(char* dest, size_t* dest_size, std::string_view src);