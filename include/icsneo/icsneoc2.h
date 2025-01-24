#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <icsneo/icsneoc2types.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
    //  Microsoft 
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#ifdef ICSNEOC2_BUILD_STATIC
#define ICSNEOC2_API
#else
#ifdef ICSNEOC2_BUILD_DYNAMIC
#define ICSNEOC2_API EXPORT
#else
#define ICSNEOC2_API IMPORT
#endif // ICSNEOC2_BUILD_DYNAMIC
#endif // ICSNEOC2_BUILD_STATIC

/** @brief icsneoc2_device_t opaque struct definition
 * 
 * This object represents a single device found on the system.
 * 
 * @see icsneo_find_devices
 */
typedef struct icsneoc2_device_t icsneoc2_device_t;


/** @brief icsneoc2_message_t opaque struct definition
 * 
 * This object represents a single event from the device.
 * 
 * @see icsneoc2_device_events_get
 */
typedef struct icsneoc2_message_t icsneoc2_message_t;


/** @brief icsneoc2_event_t opaque struct definition
 * 
 * This object represents a single event from the device.
 * 
 * @see icsneoc2_device_events_get
 */
typedef struct icsneoc2_event_t icsneoc2_event_t;


/** @brief Error codes for icsneo functions.
 *
 * This enum is guaranteed to be ABI stable, any new values will be appended to the end. 
 */
typedef enum _icsneoc2_error_t {
    // Function was successful
    icsneoc2_error_success,
    // Invalid parameters, typically because of a NULL reference.
    icsneoc2_error_invalid_parameters,
    // Error opening the device.
    icsneoc2_error_open_failed,
    // Error going online.
    icsneoc2_error_go_online_failed,
    // Error enabling message polling.
    icsneoc2_error_enable_message_polling_failed,
    // Error syncing RTC.
    icsneoc2_error_sync_rtc_failed,
    // Error getting messages.
    icsneoc2_error_get_messages_failed,
    // Generic invalid type error
    icsneoc2_error_invalid_type,
    // Generic RTC error code
    icsneoc2_error_rtc_failure,
    // Error setting settings
    icsneoc2_error_set_settings_failure,
    // Failed to transmit messages
    icsneoc2_error_transmit_messages_failed,
    // Failed to copy string to buffer
    icsneoc2_error_string_copy_failed,
    // Invalid device parameter
    icsneoc2_error_invalid_device,
    // Invalid message parameter
    icsneoc2_error_invalid_message,
    // NOTE: Any new values added here should be updated in icsneoc2_error_code_get
} _icsneoc2_error_t;

/** @brief Integer representation of _icsneoc2_error_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint32_t icsneoc2_error_t;


/** @brief Get the error string for an error code.
 * 
 * @param[in] icsneoc2_error_t error_code The error code to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_error_code_get(icsneoc2_error_t error_code, const char* value, uint32_t* value_length);

/** @brief Get the device type string for a icsneoc2_devicetype_t.
 * 
 * @param[in] icsneoc2_devicetype_t device_type The device type to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_type_name_get(icsneoc2_devicetype_t device_type, const char* value, uint32_t* value_length);

/** @brief Find all hardware attached to the system. 
 * 
 * @param[out] icsneoc2_device_t array of devices to be filled with found devices. 
 *  Undefined behaviour if index is out of range of devices_count.
 * @param[in,out] uint32_t* devices_count Size of the devices array. Modified with the number of devices found.
 * @param[in] void* reserved Reserved for future use. Currently unused and must be set to NULL.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_find_all(icsneoc2_device_t** devices, uint32_t* devices_count, void* reserved);

/** @brief Check to make sure a device is valid. 
 * 
 * @param[in] icsneoc2_device_t device The device to check.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_valid(icsneoc2_device_t* device);

/** @brief Check to make sure a device is open. 
 * 
 * @param[in] icsneoc2_device_t device The device to check.
 * @param[out] bool is_open true if the device is open, false otherwise 
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_open(icsneoc2_device_t* device, bool* is_open);

/** @brief Check see if a device is disconnected. 
 * 
 * @param[in] icsneoc2_device_t device The device to check.
 * @param[out] bool is_disconnected true if the device is open, false otherwise 
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_disconnected(icsneoc2_device_t* device, bool* is_disconnected);

/** @brief Get the open options for a device 
 * 
 * @param[in] icsneoc2_device_t device The device to set options for.
 * @param[in] icsneoc2_open_options_t options Options to set for the device.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_open_options_get(icsneoc2_device_t* device, icsneoc2_open_options_t* options);

/** @brief Set the open options for a device 
 * 
 * @param[in] icsneoc2_device_t device The device to set options for.
 * @param[in] icsneoc2_open_options_t options Options to set for the device.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_open_options_set(icsneoc2_device_t* device, icsneoc2_open_options_t options);

/** @brief Open a connection to a device.
 * 
 * After a successful call to this function, icsneoc2_device_close() must be called to close the device.
 * 
 * @param[in] icsneoc2_device_t device The device to open.
 * @param[out] icsneo_handle_t* handle Pointer to a handle to the opened device. Will be NULL on failure.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_failure otherwise.
 * 
 * @see icsneoc2_device_close
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_open(icsneoc2_device_t* device);

/** @brief Close a connection to a previously opened device.
 * 
 * After a successful call to icsneoc2_device_open(), this function must be called to close the device.
 * An already closed device will still succeed. All messages and events related to the device will be freed.
 * 
 * @param[in] icsneoc2_device_t device The device to close.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_failure otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_close(icsneoc2_device_t* device);

/** @brief Get the description of a device
 * 
 * @param[in] icsneoc2_device_t device The device to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_description_get(icsneoc2_device_t* device, const char* value, uint32_t* value_length);

/** @brief Get the type of a device
 * 
 * @param[in] icsneoc2_device_t device The device to get the type of.
 * @param[out] icsneoc2_devicetype_t* value Pointer to an icsneoc2_devicetype_t to copy the type into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_type_get(icsneoc2_device_t* device, icsneoc2_devicetype_t* value);

/** @brief Get the serial of a device
 * 
 * @param[in] icsneoc2_device_t device The device to get the serial of.
 * @param[out] const char* value Pointer to a buffer to copy the serial into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the serial.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_serial_get(icsneoc2_device_t* device, const char* value, uint32_t* value_length);

/** @brief Set the online state of a device.
 * 
 * @param[in] icsneoc2_device_t device The device to set the online state of.
 * @param[in] bool go_online true to go online, false to go offline.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_go_online(icsneoc2_device_t* device, bool go_online);

/** @brief Get the online state of a device.
 * 
 * @param[in] icsneoc2_device_t device The device to get the online state of.
 * @param[out] bool true if online, false if offline.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_online(icsneoc2_device_t* device, bool* is_online);

/** @brief Get the online supported state of a device.
 * 
 * @param[in] icsneoc2_device_t device The device to get the online supported state of.
 * @param[out] bool true if online, false if offline.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_is_online_supported(icsneoc2_device_t* device, bool* is_online_supported);


/** @brief Set the message polling state of a device.
 * 
 * @param[in] icsneoc2_device_t device The device to set the message polling state of.
 * @param[in] bool enable true to enable message polling, false to disable message polling..
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_polling_set(icsneoc2_device_t* device, bool enable);

/** @brief Get the message polling state of a device.
 * 
 * @param[in] icsneoc2_device_t device The device to set the message polling state of.
 * @param[out] bool is_enabled true if message polling is enabled, false if message polling is disabled.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_polling_get(icsneoc2_device_t* device, bool* is_enabled);

/** @brief Set the message polling limit of a device.
 * 
 * This will truncate the message queue to the specified limit.
 * 
 * @param[in] icsneoc2_device_t device The device to enforce the message polling limit.
 * @param[in] uint32_t limit The limit to enforce.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_polling_set_limit(icsneoc2_device_t* device, uint32_t limit);

/** @brief Get the message polling limit of a device.
 * 
 * @see icsneoc2_device_message_polling_set_limit
 * 
 * @param[in] icsneoc2_device_t device The device to enforce the message polling limit.
 * @param[out] uint32_t limit The limit to get.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_polling_limit_get(icsneoc2_device_t* device, uint32_t* limit);

/** @brief Get the message count of a device
 * 
 * @param[in] icsneoc2_device_t device The device to get the message count of.
 * @param[out] uint32_t* count Pointer to a uint32_t to copy the message count into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_message_count_get(icsneoc2_device_t* device, uint32_t* count);


/** @brief Get the timestamp resolution (nanoseconds) of a device
 * 
 * @param[in] icsneoc2_device_t device The device to get the timestamp resolution of.
 * @param[out] uint32_t* resolution Pointer to a uint32_t to copy the timestamp resolution in nanoseconds into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_timestamp_resolution_get(icsneoc2_device_t* device, uint32_t* resolution);

/** @brief Get the messages of a device
 * 
 * When calling this function, the previous messages retrieved by this function will be invalid.
 * 
 * @param[in] icsneoc2_device_t device The device to get the messages of.
 * @param[out] icsneoc2_message_t** messages Pointer to an array of icsneoc2_message_t to copy the messages into.
 *  Undefined behaviour if index is out of range of messages_count.
 * @param[in,out] uint32_t* messages_count Size of the messages array. Modified with the number of messages found.
 * @param[in] uint32_t timeout_ms The timeout in milliseconds to wait for messages. A value of 0 indicates a non-blocking call.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_messages_get(icsneoc2_device_t* device, icsneoc2_message_t** messages, uint32_t* messages_count, uint32_t timeout_ms);

/** @brief Transmit messages from a device
 * 
 * @param[in] icsneoc2_device_t device The device to get the messages of.
 * @param[out] icsneoc2_message_t** messages Pointer to an array of icsneoc2_message_t to copy the messages into.
 *  Undefined behaviour if index is out of range of messages_count.
 * @param[in,out] uint32_t* messages_count Size of the messages array. Modified with the number of messages actually transmitted.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_transmission_failed otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_messages_transmit(icsneoc2_device_t* device, icsneoc2_message_t** messages, uint32_t* messages_count);

/** @brief Check if a message is valid
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] bool* is_valid Pointer to a bool to copy the validity of the message into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_is_valid(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* is_valid);

/** @brief Get the type of a message
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] icsneoc2_msg_type_t* msg_type Pointer to a icsneoc2_msg_type_t to copy the type of the value into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 * 
 * @see icsneoc2_msg_type_t
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_type_get(icsneoc2_device_t* device, icsneoc2_message_t* message, icsneoc2_msg_type_t* msg_type);

/** @brief Get the message type string for a icsneoc2_msg_bus_type_t.
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_type_name_get(icsneoc2_msg_type_t msg_type, const char* value, uint32_t* value_length);

/** @brief Get the type of a bus message
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] icsneoc2_msg_type_t* msg_type Pointer to a icsneoc2_msg_type_t to copy the type of the value into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_invalid_type otherwise.
 * 
 * @see icsneoc2_msg_bus_type_t, icsneoc2_bus_type_name_get
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_bus_type_get(icsneoc2_device_t* device, icsneoc2_message_t* message, icsneoc2_msg_bus_type_t* bus_type);

/** @brief Get the bus type string for a icsneoc2_msg_bus_type_t.
 * 
 * @param[in] icsneoc2_msg_bus_type_t bus_type The bus type to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_bus_type_name_get(icsneoc2_msg_bus_type_t bus_type, const char* value, uint32_t* value_length);

/** @brief Get the transmission status of a message.
 * 
 * When a message is transmitted from the device, It will be returned in the receive buffer. 
 * @see icsneoc2_device_messages_transmit
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to modify.
 * @param[out] bool value Pointer to a bool to copy the tranmission status into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_is_transmit(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value);

/** @brief Get the Network ID (netid) of a bus message
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] icsneoc2_netid_t* netid Pointer to a icsneoc2_netid_t to copy the type of the value into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_invalid_type otherwise.
 * 
 * @see icsneoc2_netid_t, icsneoc2_netid_name_get
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_netid_get(icsneoc2_device_t* device, icsneoc2_message_t* message, icsneoc2_netid_t* netid);

/** @brief Get the netid string for a icsneoc2_netid_t.
 * 
 * @param[in] icsneoc2_netid_t netid The network id to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_netid_name_get(icsneoc2_netid_t netid, const char* value, uint32_t* value_length);

/** @brief Set the Network ID (netid) of a bus message
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[in] icsneoc2_netid_t netid The netid to set.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_netid_set(icsneoc2_device_t* device, icsneoc2_message_t* message, icsneoc2_netid_t netid);

/** @brief Set the data bytes of a message 
 * 
 * @note This function will not set the DLC of the message. @see icsneoc2_message_set_dlc
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to copy the data into.
 * @param[out] uint8_t* data Pointer to a uint8_t array to copy from.
 * @param[in] uint32_t data_length length of the data.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_data_set(icsneoc2_device_t* device, icsneoc2_message_t* message, uint8_t* data, uint32_t data_length);

/** @brief Get the data bytes of a message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] uint8_t* data Pointer to a uint8_t to copy the data bytes into.
 * @param[in,out] uint32_t* data_length Pointer to a uint32_t to copy the length of the data into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_data_get(icsneoc2_device_t* device, icsneoc2_message_t* message, uint8_t* data, uint32_t* data_length);

/** @brief Get the Arbitration ID of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] uint32_t* value Pointer to a uint32_t to copy the Arbitration ID into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_arbid_get(icsneoc2_device_t* device, icsneoc2_message_t* message, uint32_t* value);

/** @brief Set the Arbitration ID of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] uint32_t value Arbitration ID to set.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_arbid_set(icsneoc2_device_t* device, icsneoc2_message_t* message, uint32_t value);

/** @brief Get the DLC on wire of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] uint32_t* value Pointer to a uint32_t to copy the DLC on wire into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_dlc_get(icsneoc2_device_t* device, icsneoc2_message_t* message, int32_t* value);

/** @brief Set the DLC on wire of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] int32_t value DLC to set. Set to a negative value to auto calculate. Auto setting assumes data and
 *  canfd parameters are correct. Set to 0 on failure. @see icsneoc2_message_can_set_data and icsneoc2_message_can_canfd_set
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_dlc_set(icsneoc2_device_t* device, icsneoc2_message_t* message, int32_t value);

/** @brief Get the Remote Transmission Request (RTR) status of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[in] int32_t value DLC to get. 
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_is_remote(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value);

/** @brief Set the Remote Transmission Request (RTR) of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to modify.
 * @param[out] bool value Remote status to set.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_set_remote(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value);

/** @brief Get the extended status of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] bool* value Pointer to a uint32_t to copy the extended status into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_is_extended(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value);

/** @brief Set the extended status of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to modify.
 * @param[out] bool value Extended status to set.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_extended_set(icsneoc2_device_t* device, icsneoc2_message_t* message, bool value);

/** @brief Get the CANFD status of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] bool* value Pointer to a uint32_t to copy the CANFD status into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_is_canfd(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value);

/** @brief Set the CANFD status of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to modify.
 * @param[out] bool value CANFD status to set.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_canfd_set(icsneoc2_device_t* device, icsneoc2_message_t* message, bool value);

/** @brief Get the baudrate switch status (BRS) of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] bool* value Pointer to a uint32_t to copy the baudrate switch (BRS) status into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_baudrate_switch_get(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value);

/** @brief Set the baudrate switch status (BRS) of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to modify.
 * @param[out] bool value baudrate switch status (BRS) to set.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_baudrate_switch_set(icsneoc2_device_t* device, icsneoc2_message_t* message, bool value);

/** @brief Get the error state indicator status of a CAN message 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[in] icsneoc2_message_t* message The message to check.
 * @param[out] bool* value Pointer to a uint32_t to copy the error state indicator status into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_error_state_indicator_get(icsneoc2_device_t* device, icsneoc2_message_t* message, bool* value);

/** @brief Create CAN messages for a device
 * 
 * @param[in] icsneoc2_device_t device The device to get the messages of.
 * @param[out] icsneoc2_message_t** messages Pointer to an array of icsneoc2_message_t to copy the messages into.
 *  Undefined behaviour if index is out of range of messages_count.
 * @param[in] uint32_t* messages_count Size of the messages array.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_create(icsneoc2_device_t* device, icsneoc2_message_t** messages, uint32_t messages_count);

/** @brief Free CAN messages for a device
 * 
 * @param[in] icsneoc2_device_t device The device to free the messages of.
 * @param[in] icsneoc2_message_t* message The message to free.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 * 
 * @warning This function should only be called on messages created by icsneoc2_message_can_create.
 * 
 * @see icsneoc2_message_can_create
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_message_can_free(icsneoc2_device_t* device, icsneoc2_message_t* message);

/** @brief Get the global events not specifically related to a device.
 * 
 * @param[out] icsneoc2_event_t** events Pointer to an array of icsneoc2_event_t to copy the events into.
 *  Undefined behaviour if index is out of range of events_count.
 * @param[in,out] uint32_t* events_count Size of the events array. Modified with the number of events found.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_events_get(icsneoc2_event_t** events, uint32_t* events_count);

/** @brief Get the events of a device.
 * 
 * @param[in] icsneoc2_device_t device The device to get the events of.
 * @param[out] icsneoc2_event_t** events Pointer to an array of icsneoc2_event_t to copy the events into.
 *  Undefined behaviour if index is out of range of events_count.
 * @param[in,out] uint32_t* events_count Size of the events array. Modified with the number of events found.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_events_get(icsneoc2_device_t* device, icsneoc2_event_t** events, uint32_t* events_count);

/** @brief Get the error string for an error code.
 * 
 * @param[in] icsneoc2_event_t* event The event to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_event_description_get(icsneoc2_event_t* event, const char* value, uint32_t* value_length);

/** @brief Get the RTC (Real time clock) of a device.
 * 
 * @param[in] icsneoc2_device_t device The device to get the RTC of.
 * @param[out] int64_t* unix_epoch Pointer to an int64_t to copy the RTC into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_rtc_get(icsneoc2_device_t* device, int64_t* unix_epoch);

/** @brief Set the RTC (Real time clock) of a device.
 * 
 * @param[in] icsneoc2_device_t device The device to get the RTC of.
 * @param[in] int64_t unix_epoch int64_t to copy the RTC into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_rtc_set(icsneoc2_device_t* device, int64_t unix_epoch);

/** @brief Load the default settings for a device
 * 
 * @param[in] icsneoc2_device_t device The device to load the settings for.
 * @param[in] bool save True to make the settings permanent, false will be reverted on power cycle.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_load_default_settings(icsneoc2_device_t* device, bool save);

/** @brief Get the baudrate for a network
 * 
 * @note @see icsneoc2_device_canfd_baudrate_get for CANFD.
 * 
 * @param[in] icsneoc2_device_t* device The device to get the baudrate value.
 * @param[in] icsneoc2_netid_t netid The network to get the baudrate value.
 * @param[in] uint64_t* baudrate The baudrate to get the network value.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_baudrate_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint64_t* baudrate);

/** @brief Set the baudrate for a network
 *
 * @note @see icsneoc2_device_canfd_baudrate_set for CANFD.
 * 
 * @param[in] icsneoc2_device_t* device The device to set the baudrate for.
 * @param[in] icsneoc2_netid_t netid The network to set the baudrate for.
 * @param[in] uint64_t baudrate The baudrate to set the network to.
 * @param[in] bool save True to make the settings permanent, false will be reverted on power cycle.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_baudrate_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint64_t baudrate, bool save);

/** @brief Get the baudrate for a CAN FD network 
 * 
 * @param[in] icsneoc2_device_t* device The device to get the baudrate value.
 * @param[in] icsneoc2_netid_t netid The network to get the baudrate value.
 * @param[in] uint64_t* baudrate The baudrate to get the network value.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_canfd_baudrate_get(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint64_t* baudrate);

/** @brief Set the baudrate for a CANFD network 
 * 
 * @param[in] icsneoc2_device_t* device The device to set the baudrate for.
 * @param[in] icsneoc2_netid_t netid The network to set the baudrate for.
 * @param[in] uint64_t baudrate The baudrate to set the network to.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_canfd_baudrate_set(icsneoc2_device_t* device, icsneoc2_netid_t netid, uint64_t baudrate, bool save);

/** @brief Check if the device supports TC10. 
 * 
 * @param[in] icsneoc2_device_t* device The device to check against.
 * @param[out] bool* supported Pointer to a uint32_t to copy the value into.
 * 
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
*/
ICSNEOC2_API icsneoc2_error_t icsneoc2_device_supports_tc10(icsneoc2_device_t* device, bool* supported);

#ifdef __cplusplus
}
#endif
