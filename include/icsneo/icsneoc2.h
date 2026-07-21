#ifndef __ICSNEOC2_H_
#define __ICSNEOC2_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <icsneo/icsneoc2types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct icsneoc2_device_info_t icsneoc2_device_info_t;

typedef struct icsneoc2_device_t icsneoc2_device_t;

typedef struct icsneoc2_message_t icsneoc2_message_t;

typedef struct icsneoc2_event_t icsneoc2_event_t;

typedef struct icsneoc2_supported_device_t icsneoc2_supported_device_t;

typedef enum _icsneoc2_error_t {
	icsneoc2_error_success, // Function was successful
	icsneoc2_error_invalid_parameters, // Invalid parameters, typically because of a NULL reference.
	icsneoc2_error_open_failed, // Error opening the device.
	icsneoc2_error_go_online_failed, // Error going online.
	icsneoc2_error_enable_message_polling_failed, // Error enabling message polling.
	icsneoc2_error_sync_rtc_failed, // Error syncing RTC.
	icsneoc2_error_get_messages_failed, // Error getting messages.
	icsneoc2_error_invalid_type, // Generic invalid type error
	icsneoc2_error_rtc_failure, // Generic RTC error code
	icsneoc2_error_get_settings_failure, // Error getting settings
	icsneoc2_error_set_settings_failure, // Error setting settings
	icsneoc2_error_transmit_message_failed, // Failed to transmit message
	icsneoc2_error_string_copy_failed, // Failed to copy string to buffer
	icsneoc2_error_invalid_device, // Invalid device parameter
	icsneoc2_error_invalid_message, // Invalid message parameter
	icsneoc2_error_out_of_memory, // Out of memory
	icsneoc2_error_format_disk_failed, // Failed to format disk
	icsneoc2_error_script_start_failed, // Failed to start script
	icsneoc2_error_script_stop_failed, // Failed to stop script
	icsneoc2_error_script_clear_failed, // Failed to clear script
	icsneoc2_error_script_upload_failed, // Failed to upload coremini script
	icsneoc2_error_script_load_prepare_failed, // Failed to prepare script load
	icsneoc2_error_close_failed, // Failed to close device
	icsneoc2_error_reconnect_failed, // Failed to reconnect to device
	icsneoc2_error_invalid_data, // Failed to get/set data due to invalid data pointer or size
	icsneoc2_error_force_disk_config_update_failed, // Failed to force a disk config update
	// NOTE: Any new values added here should be updated in icsneoc2_error_code_get
	icsneoc2_error_maxsize
} _icsneoc2_error_t;

typedef uint32_t icsneoc2_error_t;

static const icsneoc2_open_options_t icsneoc2_open_options_default =
	ICSNEOC2_OPEN_OPTIONS_GO_ONLINE | ICSNEOC2_OPEN_OPTIONS_SYNC_RTC | ICSNEOC2_OPEN_OPTIONS_ENABLE_AUTO_UPDATE;

/**
 * Get the error string for an error code.
 *
 * @param[in] error_code The error code to get the description of.
 * @param[out] value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] value_length Size of the value buffer. Modified with the length of the description.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_error_code_get(icsneoc2_error_t error_code, char* value, size_t* value_length);

/**
 * Enumerate supported device types.
 * 
 * @param[out] supported_devices Pointer to receive the head of the supported device types linked list. Must be freed with icsneoc2_supported_devices_free().
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_supported_devices_enumerate(icsneoc2_supported_device_t** supported_devices);

/**
 * Free a supported device types list returned by icsneoc2_supported_devices_enumerate().
 *
 * @param[in] supported_devices The head of the supported device types linked list to free.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_supported_devices_free(icsneoc2_supported_device_t* supported_devices);

/**
 * Advance to the next supported device type in a list.
 *
 * @param[in] supported_device The current supported device type node.
 *
 * @return The next supported device type node, or NULL at the end of the list.
 */

icsneoc2_supported_device_t* icsneoc2_supported_devices_next(const icsneoc2_supported_device_t* supported_device);

/**
 * Get the device type from a supported device node.
 * 
 * @param[in] supported_device The supported device node.
 * @param[out] device_type Pointer to receive the device type.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_supported_device_get(const icsneoc2_supported_device_t* supported_device, icsneoc2_devicetype_t* device_type);

/**
 * Get the device type string for a icsneoc2_devicetype_t.
 *
 * @param[in] device_type The device type to get the description of.
 * @param[out] value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] value_length Size of the value buffer. Modified with the length of the description.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_type_name_get(icsneoc2_devicetype_t device_type, char* value, size_t* value_length);

/**
 * Enumerate connected devices. Returns head of an opaque linked list.
 * Pass 0 for device_type to enumerate all devices, or a specific type to filter.
 *
 * @param[in] device_type The device type to filter by, or 0 for all devices.
 * @param[out] devices Pointer to receive the head of the device info linked list.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 *
 * @see icsneoc2_enumeration_free
 */
icsneoc2_error_t icsneoc2_device_enumerate(icsneoc2_devicetype_t device_type, icsneoc2_device_info_t** devices);

/**
 * Free an enumeration list returned by icsneoc2_device_enumerate().
 * Safe to call after opening devices from the list.
 *
 * @param[in] devices The head of the device info linked list to free.
 */
void icsneoc2_enumeration_free(icsneoc2_device_info_t* devices);

/**
 * Advance to the next device in an enumeration list.
 *
 * @param[in] device_info The current device info node.
 *
 * @return The next device info node, or NULL at the end of the list.
 */
icsneoc2_device_info_t* icsneoc2_device_info_next(const icsneoc2_device_info_t* device_info);

/**
 * Get the serial number of an enumerated device.
 *
 * @param[in] device_info The device info node.
 * @param[out] serial Buffer to copy the serial string into. Null terminated.
 * @param[in,out] serial_length Size of the serial buffer. Modified with the length of the serial.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 */
icsneoc2_error_t icsneoc2_device_info_serial_get(const icsneoc2_device_info_t* device_info, char* serial, size_t* serial_length);

/**
 * Get the device type of an enumerated device.
 *
 * @param[in] device_info The device info node.
 * @param[out] type Pointer to receive the device type.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 */
icsneoc2_error_t icsneoc2_device_info_type_get(const icsneoc2_device_info_t* device_info, icsneoc2_devicetype_t* type);

/**
 * Get the human-readable type name of an enumerated device.
 *
 * @param[in] device_info The device info node.
 * @param[out] type_name Buffer to copy the type name into. Null terminated.
 * @param[in,out] type_name_length Size of the buffer. Modified with the length of the name.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 */
icsneoc2_error_t icsneoc2_device_info_type_name_get(const icsneoc2_device_info_t* device_info, char* type_name, size_t* type_name_length);

/**
 * Get the full description of an enumerated device (e.g. "neoVI FIRE 3 RS2043").
 *
 * @param[in] device_info The device info node.
 * @param[out] description Buffer to copy the description into. Null terminated.
 * @param[in,out] description_length Size of the buffer. Modified with the length of the description.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 */
icsneoc2_error_t icsneoc2_device_info_description_get(const icsneoc2_device_info_t* device_info, char* description, size_t* description_length);

/**
 * Check to make sure a device is valid.
 *
 * @param[in] device The device to check.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_is_valid(const icsneoc2_device_t* device);

/**
 * Check to make sure a device is open.
 *
 * @param[in] device The device to check.
 * @param[out] is_open true if the device is open, false otherwise
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_is_open(const icsneoc2_device_t* device, bool* is_open);

/**
 * Create a device handle from an enumeration node without opening it. Need to call icsneoc2_device_free() to free the handle when finished.
 * The device can then be opened with icsneoc2_device_open().
 * 
 * @param[in] device_info The device info node to create from.
 * @param[out] device Pointer to receive the created device handle.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_invalid_device otherwise.
 * 
 * @see icsneoc2_device_open icsneoc2_device_free
 */
icsneoc2_error_t icsneoc2_device_create(const icsneoc2_device_info_t* device_info, icsneoc2_device_t** device);

/**
 * Open a device from an enumeration node.
 *
 * After a successful call, icsneoc2_device_close() must be called to close the device.
 *
 * @param[in] device Pointer to the device to open.
 * @param[in] options Open options (e.g. icsneoc2_open_options_default).
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_open_failed otherwise.
 *
 * @see icsneoc2_device_close icsneoc2_device_free
 */
icsneoc2_error_t icsneoc2_device_open(const icsneoc2_device_t* device, icsneoc2_open_options_t options);

/**
 * Convenience: enumerate, find by serial, open, and free enumeration.
 *
 * @param[in] serial Serial number string to match (e.g. "RS2043").
 * @param[in] options Open options (e.g. icsneoc2_open_options_default).
 * @param[out] device Pointer to receive the opened device handle.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 *
 * @see icsneoc2_device_close icsneoc2_device_free
 */
icsneoc2_error_t icsneoc2_device_open_serial(const char* serial, icsneoc2_open_options_t options, icsneoc2_device_t** device);

/**
 * Convenience: enumerate, find first available device (optionally filtered by type), open, and free enumeration.
 * Pass 0 for device_type to match any device.
 * 
 *
 * @param[in] device_type The device type to match, or 0 for any.
 * @param[in] options Open options (e.g. icsneoc2_open_options_default).
 * @param[out] device Pointer to receive the opened device handle.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 *
 * @see icsneoc2_device_close icsneoc2_device_free
 */
icsneoc2_error_t icsneoc2_device_open_first(icsneoc2_devicetype_t device_type, icsneoc2_open_options_t options, icsneoc2_device_t** device);

/**
 * Reconnect to a device. This is useful if the device was disconnected and reconnected, or if the connection was lost for some reason.
 * 
 * @param[in] device The device to reconnect.
 * @param[in] options Open options (e.g. icsneoc2_open_options_default).
 * @param[in] timeout_ms The timeout in milliseconds to keep trying to reconnect before giving up.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_reconnect_failed if the timeout was reached without reconnecting, or icsneoc2_device_is_valid() errors otherwise.
 */
icsneoc2_error_t icsneoc2_device_reconnect(icsneoc2_device_t* device, icsneoc2_open_options_t options, uint32_t timeout_ms);

/**
 * Close a connection to a previously opened device.
 *
 * After a successful call to icsneoc2_device_open(), this function must be called to close the device.
 * An already closed device will still succeed. The device handle must be freed with icsneoc2_device_free() when finished.
 *
 * @param[in,out] device Pointer to the device to close.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_device_is_valid() errors otherwise.
 *
 * @see icsneoc2_device_open icsneoc2_device_is_valid icsneoc2_device_free
 */
icsneoc2_error_t icsneoc2_device_close(icsneoc2_device_t* device);

/**
 * Free a device handle created by icsneoc2_device_create(). Device should be closed before freeing.
 * 
 * @param[in] device The device handle to free.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_device_is_valid() errors otherwise.
 * 
 * @see icsneoc2_device_create icsneoc2_device_close
 */
icsneoc2_error_t icsneoc2_device_free(icsneoc2_device_t* device);

/**
 * Get the description of a device
 *
 * @param[in] device The device to get the description of.
 * @param[out] value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] value_length Size of the value buffer. Modified with the length of the description.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_description_get(const icsneoc2_device_t* device, char* value, size_t* value_length);

/**
 * Get the type of a device
 *
 * @param[in] device The device to get the type of.
 * @param[out] value Pointer to an icsneoc2_devicetype_t to copy the type into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_type_get(const icsneoc2_device_t* device, icsneoc2_devicetype_t* value);

/**
 * Get the serial of a device
 *
 * @param[in] device The device to get the serial of.
 * @param[out] value Pointer to a buffer to copy the serial into. Null terminated.
 * @param[in,out] value_length Size of the value buffer. Modified with the length of the serial.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_serial_get(const icsneoc2_device_t* device, char* value, size_t* value_length);

/**
 * Get the product name of a device.
 *
 * This is the device-specific marketing/product name (e.g. "neoVI FIRE 3"), which may differ from the
 * generic device-type name. Prefer this where a human-readable product name is needed.
 *
 * @param[in] device The device to get the product name of.
 * @param[out] value Pointer to a buffer to copy the product name into. Null terminated.
 * @param[in,out] value_length Size of the value buffer. Modified with the length of the product name.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_product_name_get(const icsneoc2_device_t* device, char* value, size_t* value_length);

/**
 * Get the PCB serial of a device.
 *
 * @param[in] device The device to get the PCB serial of.
 * @param[out] value Pointer to a buffer to copy the PCB serial into. If NULL, only value_length is written.
 * @param[in,out] value_length Size of the value buffer in bytes. Modified with the length of the PCB serial.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_type if the device does not have a PCB serial.
 */
icsneoc2_error_t icsneoc2_device_pcb_serial_get(const icsneoc2_device_t* device, uint8_t* value, size_t* value_length);

/**
 * Enumerate MAC addresses from a device.
 *
 * @param[in] device The device to query.
 * @param[out] mac_entries Pointer to receive the head of the MAC addresses linked list.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters on failure.
 */
icsneoc2_error_t icsneoc2_device_mac_addresses_enumerate(const icsneoc2_device_t* device, icsneoc2_mac_addr_entry_t** mac_entries);

/**
 * Get the network ID of a MAC address.
 * 
 * @param[in] mac_address The MAC address object to get the network ID of.
 * @param[out] network_id Pointer to an icsneoc2_netid_t to copy the network ID into.
 * Unrecognized values are normalized to icsneoc2_netid_invalid.
 * 
 * @return icsneoc2_error_t icsneoc2_error success if successful, icsneoc2_error_invalid_parameters on failure.
 */
icsneoc2_error_t icsneoc2_mac_network_id_get(const icsneoc2_mac_addr_entry_t* mac_address, icsneoc2_netid_t* network_id);

/**
 * Get the MAC Address bytes of a MAC address.
 * 
 * @param[in] mac_address The MAC address object to get the MAC address bytes of.
 * @param[out] value Pointer to a buffer to copy the MAC address into. If NULL, only value_length is written.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters on failure.
 */
icsneoc2_error_t icsneoc2_mac_address_get(const icsneoc2_mac_addr_entry_t* mac_address, uint8_t* value, size_t* value_length);

/**
 * Advance to the next MAC address in an enumeration list.
 * 
 * @param[in] mac_address The current MAC address node.
 * 
 * @return icsneoc2_mac_addr_entry_t The next MAC address node.
 */
icsneoc2_mac_addr_entry_t* icsneoc2_mac_addresses_next(const icsneoc2_mac_addr_entry_t* mac_address);

/**
 * Free all MAC addresses in enumeration handle returned by icsneoc2_device_mac_addresses_enumerate().
 *
 * @param[in] mac_address The head of the MAC address enumeration to free. May be NULL.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_mac_addresses_free(icsneoc2_mac_addr_entry_t* mac_address);

/**
 * Set the online state of a device.
 *
 * @param[in] device The device to set the online state of.
 * @param[in] go_online true to go online, false to go offline.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_go_online_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_go_online(const icsneoc2_device_t* device, bool go_online);

/**
 * Get the online state of a device.
 *
 * @param[in] device The device to get the online state of.
 * @param[out] is_online true if online, false if offline.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_is_online(const icsneoc2_device_t* device, bool* is_online);

/**
 * Get the online supported state of a device.
 *
 * @param[in] device The device to get the online supported state of.
 * @param[out] is_online_supported true if online supported, false if not.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_is_online_supported(const icsneoc2_device_t* device, bool* is_online_supported);

/**
 * Set the message polling limit of a device.
 *
 * This will truncate the message queue to the specified limit.
 *
 * @param[in] device The device to enforce the message polling limit.
 * @param[in] limit The limit to enforce.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_message_polling_limit_set(const icsneoc2_device_t* device, uint32_t limit);

/**
 * Get the message polling limit of a device.
 *
 * @see icsneoc2_device_message_polling_limit_set
 *
 * @param[in] device The device to get the message polling limit from.
 * @param[out] limit The limit to get.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_message_polling_limit_get(const icsneoc2_device_t* device, uint32_t* limit);

/**
 * Get the timestamp resolution (nanoseconds) of a device
 *
 * @param[in] device The device to get the timestamp resolution of.
 * @param[out] resolution Pointer to a uint32_t to copy the timestamp resolution in nanoseconds into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_timestamp_resolution_get(icsneoc2_device_t* device, uint32_t* resolution);

/**
 * Get a message from a device
 *
 * @param[in] device The device to get the message from.
 * @param[out] message Pointer to a icsneoc2_message_t to copy the message into.
 * @param[in] timeout_ms The timeout in milliseconds to wait for a message. A value of 0 indicates a non-blocking call.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 *
 * @see icsneoc2_message_free
 */
icsneoc2_error_t icsneoc2_device_message_get(const icsneoc2_device_t* device, icsneoc2_message_t** message, uint32_t timeout_ms);

/**
 * Transmit messages from a device
 *
 * @param[in] device The device to transmit the message from.
 * @param[in] messages Pointer to an icsneoc2_message_t to transmit.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_transmit_message_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_message_transmit(const icsneoc2_device_t* device, const icsneoc2_message_t* messages);

/**
 * Get the network type string for a icsneoc2_network_type_t.
 *
 * @param[in] network_type The network type to get the description of.
 * @param[out] value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] value_length Size of the value buffer. Modified with the length of the description.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_network_type_name_get(icsneoc2_network_type_t network_type, char* value, size_t* value_length);

/**
 * Get the last event to occur, with an optional device filter.
 *
 * @param[out] event Pointer to an icsneoc2_event_t to copy the event into.
 * @param[in] device Optional device filter to get event for.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 *
 * @see icsneoc2_event_free
 */
icsneoc2_error_t icsneoc2_event_get(icsneoc2_event_t** event, const icsneoc2_device_t* device);

/**
 * Pop the last event not related to a specific device.
 *
 * @param[out] event Pointer to an icsneoc2_event_t to copy the event into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_event_free(icsneoc2_event_t* event);

/**
 * Get the description of an event.
 *
 * @param[in] event The event to get the description of.
 * @param[out] value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] value_length Size of the value buffer. Modified with the length of the description.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_event_description_get(const icsneoc2_event_t* event, char* value, size_t* value_length);

/**
 * Get the RTC (Real time clock) of a device.
 *
 * @param[in] device The device to get the RTC of.
 * @param[out] unix_epoch Pointer to an int64_t to copy the RTC into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_rtc_get(const icsneoc2_device_t* device, int64_t* unix_epoch);

/**
 * Set the RTC (Real time clock) of a device.
 *
 * @param[in] device The device to set the RTC of.
 * @param[in] unix_epoch int64_t to set the RTC to.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_rtc_set(const icsneoc2_device_t* device, int64_t unix_epoch);

/**
 * Check if the device supports TC10.
 *
 * @param[in] device The device to check against.
 * @param[out] supported Pointer to a bool to copy the value into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_supports_tc10(const icsneoc2_device_t* device, bool* supported);

/**
 * Send a TC10 wake request to the device on a specific network. This is used to wake up ECUs that support TC10 wake on the specified network.
 * 
 * @param[in] device The device to send the wake request from.
 * @param[in] netid The network to send the wake request on.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_transmit_message_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_tc10_wake_request(const icsneoc2_device_t* device, icsneoc2_netid_t netid);

/**
 * Send a TC10 sleep request to the device on a specific network. This is used to put ECUs that support TC10 sleep on the specified network to sleep.
 *
 * @param[in] device The device to send the sleep request from.
 * @param[in] netid The network to send the sleep request on.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_transmit_message_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_tc10_sleep_request(const icsneoc2_device_t* device, icsneoc2_netid_t netid);

/**
 * Get the current TC10 sleep/wake status of a specific network.
 * 
 * @param[in] device The device to query.
 * @param[in] netid The network to query the TC10 status of.
 * @param[out] sleep_status Pointer to a icsneoc2_tc10_sleep_status_t to copy the sleep status into. May be NULL if sleep status is not needed.
 * @param[out] wake_status Pointer to a icsneoc2_tc10_wake_status_t to copy the wake status into. May be NULL if wake status is not needed.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_tc10_status_get(const icsneoc2_device_t* device, icsneoc2_netid_t netid, icsneoc2_tc10_sleep_status_t* sleep_status, icsneoc2_tc10_wake_status_t* wake_status);

/**
 * Check if the device supports gPTP.
 *
 * @param[in] device The device to check against.
 * @param[out] supported Pointer to a bool to copy the value into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_supports_gptp(const icsneoc2_device_t* device, bool* supported);

/**
 * Get the current gPTP status from the device.
 *
 * @param[in] device The device to query.
 * @param[in] timeout_ms Timeout in milliseconds to wait for the device response.
 * @param[out] status Pointer to an icsneoc2_gptp_status_t to copy the status into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_invalid_type otherwise.
 */
icsneoc2_error_t icsneoc2_device_gptp_status_get(const icsneoc2_device_t* device, uint32_t timeout_ms, icsneoc2_gptp_status_t* status);

/**
 * Check if the device supports rebooting.
 *
 * @param[in] device The device to check against.
 * @param[out] supported Pointer to a bool to copy the value into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_supports_reboot(const icsneoc2_device_t* device, bool* supported);

/**
 * Reboot the device. When safe is true the device boots the Linux rescue image and does not load
 * coremini ("safe boot"); otherwise it reboots normally. The device reboots in response, so no reply
 * is expected. Only supported on devices where icsneoc2_device_supports_reboot() reports true.
 *
 * @param[in] device The device to reboot.
 * @param[in] safe true to reboot into safe boot mode, false to reboot normally.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_transmit_message_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_reboot(const icsneoc2_device_t* device, bool safe);

/**
 * Get the current state of a digital I/O pin.
 *
 * @param[in] device The device to query.
 * @param[in] type The I/O pin type.
 * @param[in] number The pin number (for misc/emisc types).
 * @param[out] value Pointer to store the current pin state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_digital_io_get(const icsneoc2_device_t* device, icsneoc2_io_type_t type, uint32_t number, bool* value);

/**
 * Set the state of a digital I/O pin.
 *
 * @param[in] device The device to configure.
 * @param[in] type The I/O pin type.
 * @param[in] number The pin number (for misc/emisc types).
 * @param[in] value The desired pin state.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_set_settings_failure otherwise.
 */
icsneoc2_error_t icsneoc2_device_digital_io_set(const icsneoc2_device_t* device, icsneoc2_io_type_t type, uint32_t number, bool value);

/**
 * Get the version of the icsneoc2 API.
 *
 * @param[out] value Pointer to a buffer to copy the version string into. Null terminated.
 * @param[in,out] value_length Size of the value buffer. Modified with the length of the version string.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_version_get(char* value, size_t* value_length);

/**
 * Convert a device serial number to a string.
 *
 * @param[in] num The serial number to convert.
 * @param[out] str Pointer to a buffer to copy the serial string into. Null terminated.
 * @param[in,out] str_length Size of the str buffer. Modified with the length of the serial string.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_string_copy_failed otherwise.
 */
icsneoc2_error_t icsneoc2_serial_num_to_string(uint32_t num, char* str, size_t* str_length);

/**
 * Convert a device serial string to a number.
 *
 * @param[in] str The serial string to convert (e.g., "GS1128").
 * @param[in] str_length The length of the serial string. This should match strlen(str).
 * @param[out] num Pointer to store the converted serial number.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_serial_string_to_num(char* str, size_t str_length, uint32_t* num);

/**
 * Get the number of disk slots on a device.
 *
 * @param[in] device The device to query.
 * @param[out] count Pointer to receive the number of disk slots.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_disk_count_get(const icsneoc2_device_t* device, size_t* count);

/**
 * Check if a device supports disk formatting.
 *
 * @param[in] device The device to query.
 * @param[out] supported Pointer to receive true if the device supports disk formatting, false otherwise.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_supports_disk_formatting(const icsneoc2_device_t* device, bool* supported);

/**
 * Query the current disk details from a device.
 *
 * On success, the caller owns the returned icsneoc2_disk_details_t and must free it
 * with icsneoc2_disk_details_free() when done.
 *
 * @param[in] device The device to query.
 * @param[out] disk_details Pointer to receive the allocated disk details handle.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 *
 * @see icsneoc2_disk_details_free
 */
icsneoc2_error_t icsneoc2_device_disk_details_get(const icsneoc2_device_t* device, icsneoc2_disk_details_t** disk_details);

/**
 * Free a disk details handle returned by icsneoc2_device_disk_details_get().
 *
 * @param[in] disk_details The disk details handle to free. May be NULL.
 */
void icsneoc2_disk_details_free(icsneoc2_disk_details_t* disk_details);

/**
 * Get the number of disks described in a disk details handle.
 *
 * @param[in] disk_details The disk details handle.
 * @param[out] count Pointer to receive the disk count.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_disk_details_count_get(const icsneoc2_disk_details_t* disk_details, size_t* count);

/**
 * Get the layout of the disks (e.g. spanned or RAID0).
 *
 * @param[in] disk_details The disk details handle.
 * @param[out] layout Pointer to receive the layout.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_disk_details_layout_get(const icsneoc2_disk_details_t* disk_details, icsneoc2_disk_layout_t* layout);

/**
 * Set the layout of the disks (e.g. spanned or RAID0).
 *
 * @param[in] disk_details The disk details handle.
 * @param[in] layout The layout to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_disk_details_layout_set(const icsneoc2_disk_details_t* disk_details, icsneoc2_disk_layout_t layout);

/**
 * Get the status flags for a specific disk by index.
 *
 * Flags are a bitmask of icsneoc2_disk_format_flag_present, icsneoc2_disk_format_flag_initialized,
 * and icsneoc2_disk_format_flag_formatted.
 *
 * @param[in] disk_details The disk details handle.
 * @param[in] index Zero-based disk index.
 * @param[out] flags Pointer to receive the flags bitmask.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters if out of range.
 */
icsneoc2_error_t icsneoc2_disk_details_flags_get(const icsneoc2_disk_details_t* disk_details, size_t index, icsneoc2_disk_format_flags_t* flags);

/**
 * Set the status flags for a specific disk by index.
 *
 * Flags are a bitmask of icsneoc2_disk_format_flag_present, icsneoc2_disk_format_flag_initialized,
 * and icsneoc2_disk_format_flag_formatted.
 *
 * @param[in] disk_details The disk details handle.
 * @param[in] index Zero-based disk index.
 * @param[in] flags Flags bitmask to apply.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters if out of range.
 */
icsneoc2_error_t icsneoc2_disk_details_flags_set(const icsneoc2_disk_details_t* disk_details, size_t index, icsneoc2_disk_format_flags_t flags);

/**
 * Get the size of a specific disk by index.
 *
 * @param[in] disk_details The disk details handle.
 * @param[in] index Zero-based disk index.
 * @param[out] sectors Pointer to receive the number of sectors.
 * @param[out] bytes_per_sector Pointer to receive the number of bytes per sector.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters if out of range.
 */
icsneoc2_error_t icsneoc2_disk_details_size_get(const icsneoc2_disk_details_t* disk_details, size_t index, uint64_t* sectors, uint64_t* bytes_per_sector);

/**
 * Get whether a full format is configured.
 *
 * @param[in] disk_details The disk details handle.
 * @param[out] full_format Pointer to receive true for full format, false for quick format.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_disk_details_full_format_get(const icsneoc2_disk_details_t* disk_details, bool* full_format);

/**
 * Set whether to perform a full format.
 *
 * @param[in] disk_details The disk details handle.
 * @param[in] full_format true for a full (slow) format, false for a quick format.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_disk_details_full_format_set(const icsneoc2_disk_details_t* disk_details, bool full_format);

/**
 * Format the disk(s) on a device.
 *
 * This is a blocking call that runs until formatting completes or the progress callback
 * returns icsneoc2_disk_format_directive_stop.
 *
 * @param[in] device The device whose disks should be formatted.
 * @param[in] disk_details A disk details handle describing the format configuration.
 *            Use icsneoc2_device_disk_details_get() to obtain a handle, then modify flags
 *            (set icsneoc2_disk_format_flag_formatted on disks you want to format).
 * @param[in] progress_callback Optional callback invoked periodically with progress.
 *            Return icsneoc2_disk_format_directive_continue to keep going, or
 *            icsneoc2_disk_format_directive_stop to cancel. May be NULL.
 * @param[in] user_data Opaque pointer passed through to the progress callback.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_format_disk_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_format_disk(const icsneoc2_device_t* device, icsneoc2_disk_details_t* disk_details, icsneoc2_disk_format_progress_fn progress_callback, void* user_data);

/**
 * Force a disk layout/configuration change on a device without formatting.
 *
 * Unlike icsneoc2_device_format_disk(), this applies the configuration described by the
 * disk details handle (such as the disk layout) and enables the change without erasing data.
 *
 * @param[in] device The device whose disk configuration should be updated.
 * @param[in] disk_details A disk details handle describing the desired configuration.
 *            Use icsneoc2_device_disk_details_get() to obtain a handle, then modify it
 *            (e.g. set the layout with icsneoc2_disk_details_layout_set()).
 *            In this context the per-disk ICSNEOC2_DISK_FORMAT_FLAGS_FORMATTED flag
 *            selects whether that disk is enabled in the layout (no formatting occurs).
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful,
 *         icsneoc2_error_force_disk_config_update_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_force_disk_config_update(const icsneoc2_device_t* device, icsneoc2_disk_details_t* disk_details);

/**
 * Get the list of networks this device supports for receiving.
 *
 * If networks is NULL, only the count is written. Otherwise, up to *count network IDs
 * are copied into the caller-provided buffer and *count is set to the number written.
 *
 * @param[in] device The device to query.
 * @param[out] networks Caller-allocated array to receive network IDs, or NULL to query count only.
 * @param[in,out] count On input, the maximum number of entries the buffer can hold.
 *                      On output, the number of entries written (or total available if networks is NULL).
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_supported_rx_networks_get(const icsneoc2_device_t* device, icsneoc2_netid_t* networks, size_t* count);

/**
 * Get the list of networks this device supports for transmitting.
 *
 * If networks is NULL, only the count is written. Otherwise, up to *count network IDs
 * are copied into the caller-provided buffer and *count is set to the number written.
 *
 * @param[in] device The device to query.
 * @param[out] networks Caller-allocated array to receive network IDs, or NULL to query count only.
 * @param[in,out] count On input, the maximum number of entries the buffer can hold.
 *                      On output, the number of entries written (or total available if networks is NULL).
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_supported_tx_networks_get(const icsneoc2_device_t* device, icsneoc2_netid_t* networks, size_t* count);

/**
 * Check whether this device supports CoreMini scripts.
 *
 * @param[in] device The device to query.
 * @param[out] supported Set to true if the device supports CoreMini scripts, false otherwise.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_supports_coremini_script(const icsneoc2_device_t* device, bool* supported);

/**
 * Start a CoreMini script on the device.
 *
 * @param[in] device The device to start the script on.
 * @param[in] memory_type The memory location of the script (flash or SD).
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_script_start_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_script_start(const icsneoc2_device_t* device, icsneoc2_memory_type_t memory_type);

/**
 * Stop a running CoreMini script on the device.
 *
 * @param[in] device The device to stop the script on.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_script_stop_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_script_stop(const icsneoc2_device_t* device);

/**
 * Clear a CoreMini script from the device.
 *
 * @param[in] device The device to clear the script from.
 * @param[in] memory_type The memory location to clear the script from (flash or SD).
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_script_clear_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_script_clear(const icsneoc2_device_t* device, icsneoc2_memory_type_t memory_type);

/**
 * Prepare the device for a script load operation.
 *
 * @param[in] device The device to prepare.
 * @param[out] status Receives the prepare status code from the device.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_script_load_prepare_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_script_prepare_load(const icsneoc2_device_t* device, int8_t* status);

/**
 * Upload a CoreMini script to the device from a file path.
 *
 * @param[in] device The device to upload to.
 * @param[in] path Null-terminated file path to the CoreMini binary.
 * @param[in] memory_type The memory location to upload to (flash or SD).
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_script_upload_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_coremini_upload_file(const icsneoc2_device_t* device, const char* path, icsneoc2_memory_type_t memory_type);

/**
 * Upload a CoreMini script to the device from a memory buffer.
 *
 * @param[in] device The device to upload to.
 * @param[in] data Pointer to the CoreMini binary data.
 * @param[in] length Length of the data in bytes.
 * @param[in] memory_type The memory location to upload to (flash or SD).
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_script_upload_failed otherwise.
 */
icsneoc2_error_t icsneoc2_device_coremini_upload(const icsneoc2_device_t* device, const uint8_t* data, size_t length, icsneoc2_memory_type_t memory_type);

/**
 * Get the current script status from the device.
 *
 * Returns an opaque handle that must be freed with icsneoc2_script_status_free() when done.
 *
 * @param[in] device The device to query.
 * @param[out] script_status Receives a newly allocated script status handle.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 * @see icsneoc2_script_status_free
 */
icsneoc2_error_t icsneoc2_device_script_status_get(const icsneoc2_device_t* device, icsneoc2_script_status_t** script_status);

/**
 * Free a script status handle returned by icsneoc2_device_script_status_get().
 *
 * @param[in] script_status The handle to free. May be NULL.
 */
void icsneoc2_script_status_free(icsneoc2_script_status_t* script_status);

/**
 * Get whether the CoreMini script is currently running.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Set to true if a script is running, false otherwise.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_is_coremini_running(const icsneoc2_script_status_t* script_status, bool* value);

/**
 * Get whether the script is encrypted.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Set to true if the script is encrypted, false otherwise.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_is_encrypted(const icsneoc2_script_status_t* script_status, bool* value);

/**
 * Get the number of sector overflows.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the sector overflow count.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_sector_overflows_get(const icsneoc2_script_status_t* script_status, uint32_t* value);

/**
 * Get the number of remaining sector buffers.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the remaining sector buffer count.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_remaining_sector_buffers_get(const icsneoc2_script_status_t* script_status, uint32_t* value);

/**
 * Get the last sector written.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the last sector value.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_last_sector_get(const icsneoc2_script_status_t* script_status, uint32_t* value);

/**
 * Get the read binary size.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the read binary size in bytes.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_read_bin_size_get(const icsneoc2_script_status_t* script_status, uint32_t* value);

/**
 * Get the minimum sector.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the minimum sector value.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_min_sector_get(const icsneoc2_script_status_t* script_status, uint32_t* value);

/**
 * Get the maximum sector.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the maximum sector value.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_max_sector_get(const icsneoc2_script_status_t* script_status, uint32_t* value);

/**
 * Get the current sector.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the current sector value.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_current_sector_get(const icsneoc2_script_status_t* script_status, uint32_t* value);

/**
 * Get the CoreMini creation timestamp.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the creation timestamp as a Unix epoch value.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_coremini_create_time_get(const icsneoc2_script_status_t* script_status, uint64_t* value);

/**
 * Get the file checksum.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the file checksum.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_file_checksum_get(const icsneoc2_script_status_t* script_status, uint16_t* value);

/**
 * Get the CoreMini version.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the CoreMini version.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_coremini_version_get(const icsneoc2_script_status_t* script_status, uint16_t* value);

/**
 * Get the CoreMini header size.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the CoreMini header size in bytes.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_coremini_header_size_get(const icsneoc2_script_status_t* script_status, uint16_t* value);

/**
 * Get the diagnostic error code.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the diagnostic error code.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_diagnostic_error_code_get(const icsneoc2_script_status_t* script_status, uint8_t* value);

/**
 * Get the diagnostic error code count.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the diagnostic error code count.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_diagnostic_error_code_count_get(const icsneoc2_script_status_t* script_status, uint8_t* value);

/**
 * Get the maximum CoreMini size in kilobytes.
 *
 * @param[in] script_status The script status handle.
 * @param[out] value Receives the maximum CoreMini size in KB.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_script_status_max_coremini_size_kb_get(const icsneoc2_script_status_t* script_status, uint16_t* value);

/**
 * Get the list of chip versions on the device.
 * 
 * @param[in] device The device to query.
 * @param[out] chip_versions Receives a newly allocated chip versions handle. The caller owns this handle and must free it with icsneoc2_chip_versions_free() when done.
 * @param[in] refresh Whether to refresh the chip versions from the device.
 * @param[out] count Receives the number of chip versions. May be NULL if not needed.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_device_chip_versions_enumerate(const icsneoc2_device_t* device, icsneoc2_chip_versions_t** chip_versions, bool refresh, size_t* count);

/**
 * Free a chip versions handle returned by icsneoc2_device_chip_versions_enumerate().
 *
 * @param[in] chip_versions The chip versions handle to free. May be NULL.
 */
icsneoc2_error_t icsneoc2_chip_versions_free(icsneoc2_chip_versions_t* chip_versions);

/**
 * Advance to the next chip version in an enumeration list.
 *
 * @param[in] chip_versions The current chip versions handle.
 *
 * @return The next chip version handle, or NULL at the end of the list.
 */
icsneoc2_chip_versions_t* icsneoc2_chip_versions_next(const icsneoc2_chip_versions_t* chip_versions);

/**
 * Get the properties of a chip version.
 *
 * @param[in] chip_versions The chip versions handle to query.
 * @param[out] name Pointer to a buffer to copy the null-terminated chip name into. May be NULL if the name is not needed.
 * @param[in,out] name_length On input, the size of the name buffer. On output, the length of the chip name. May be NULL if the name is not needed.
 * @param[out] major Pointer to receive the major version number. May be NULL.
 * @param[out] minor Pointer to receive the minor version number. May be NULL.
 * @param[out] maintenance Pointer to receive the maintenance version number. May be NULL.
 * @param[out] build Pointer to receive the build version number. May be NULL.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_string_copy_failed otherwise.
 */
icsneoc2_error_t icsneoc2_chip_versions_props_get(const icsneoc2_chip_versions_t* chip_versions, char* name, size_t* name_length, uint8_t* major, uint8_t* minor, uint8_t* maintenance, uint8_t* build);


#ifdef __cplusplus
}
#endif

#endif // __ICSNEOC2_H_
