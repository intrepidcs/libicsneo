#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <icsneo/icsneotypes.h>

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

#ifdef ICSNEO_BUILD_STATIC
#define ICSNEO_API
#else
#ifdef ICSNEO_EXPORTS
#define ICSNEO_API EXPORT
#else
#define ICSNEO_API IMPORT
#endif // ICSNEO_EXPORTS
#endif // ICSNEO_BUILD_STATIC

/** @brief icsneo_device_t opaque struct definition
 * 
 * This object represents a single device found on the system.
 * 
 * @see icsneo_find_devices
 */
typedef struct icsneo_device_t icsneo_device_t;


/** @brief icsneo_message_t opaque struct definition
 * 
 * This object represents a single event from the device.
 * 
 * @see icsneo_device_get_events
 */
typedef struct icsneo_message_t icsneo_message_t;


/** @brief icsneo_event_t opaque struct definition
 * 
 * This object represents a single event from the device.
 * 
 * @see icsneo_device_get_events
 */
typedef struct icsneo_event_t icsneo_event_t;


/** @brief Error codes for icsneo functions.
 *
 * This enum is guaranteed to be ABI stable, any new values will be appended to the end. 
 */
typedef enum _icsneo_error_t {
    // Function was successful
    icsneo_error_success,
    // Invalid parameters, typically because of a NULL reference.
    icsneo_error_invalid_parameters,
    // Error opening the device.
    icsneo_error_open_failed,
    // Error going online.
    icsneo_error_go_online_failed,
    // Error enabling message polling.
    icsneo_error_enable_message_polling_failed,
    // Error syncing RTC.
    icsneo_error_sync_rtc_failed,
    // Error getting messages.
    icsneo_error_get_messages_failed,
    // Generic invalid type error
    icsneo_error_invalid_type,
    // Generic RTC error code
    icsneo_error_rtc_failure,
    // Error setting settings
    icsneo_error_set_settings_failure
} _icsneo_error_t;

/** @brief Integer representation of _icsneo_error_t enum. 
 * 
 * This is used for easier ABI compatibility, especially between other languages.
 */
typedef uint32_t icsneo_error_t;


/** @brief Get the error string for an error code.
 * 
 * @param[in] icsneo_device_t device The device to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_get_error_code(icsneo_error_t error_code, const char* value, uint32_t* value_length);

/** @brief Get the device type string for a icsneo_devicetype_t.
 * 
 * @param[in] icsneo_devicetype_t device_type The device type to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_type_from_type(icsneo_devicetype_t device_type, const char* value, uint32_t* value_length);

/** @brief Find all hardware attached to the system. 
 * 
 * @param[out] icsneo_device_t array of devices to be filled with found devices. 
 *  Undefined behaviour if index is out of range of devices_count.
 * @param[in,out] uint32_t* devices_count Size of the devices array. Modified with the number of devices found.
 * @param[in] void* reserved Reserved for future use. Currently unused and must be set to NULL.
 * 
 * @return icsneo_error_t icsneo_error_success if successful.
*/
ICSNEO_API icsneo_error_t icsneo_device_find_all(icsneo_device_t** devices, uint32_t* devices_count, void* reserved);

/** @brief Check to make sure a device is valid. 
 * 
 * @param[in] icsneo_device_t device The device to check.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_device_is_valid(icsneo_device_t* device);

/** @brief Get the open options for a device 
 * 
 * @param[in] icsneo_device_t device The device to set options for.
 * @param[in] icsneo_open_options_t options Options to set for the device.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_device_get_open_options(icsneo_device_t* device, icsneo_open_options_t* options);

/** @brief Set the open options for a device 
 * 
 * @param[in] icsneo_device_t device The device to set options for.
 * @param[in] icsneo_open_options_t options Options to set for the device.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_device_set_open_options(icsneo_device_t* device, icsneo_open_options_t options);

/** @brief Open a connection to a device.
 * 
 * After a successful call to this function, icsneo_device_close() must be called to close the device.
 * 
 * @param[in] icsneo_device_t device The device to open.
 * @param[out] icsneo_handle_t* handle Pointer to a handle to the opened device. Will be NULL on failure.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_failure otherwise.
 * 
 * @see icsneo_device_close
 */
ICSNEO_API icsneo_error_t icsneo_device_open(icsneo_device_t* device);

/** @brief Close a connection to a previously opened device.
 * 
 * After a successful call to icsneo_device_open(), this function must be called to close the device.
 * An already closed device will still succeed.
 * 
 * @param[in] icsneo_device_t device The device to close.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_failure otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_close(icsneo_device_t* device);

/** @brief Get the description of a device
 * 
 * @param[in] icsneo_device_t device The device to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_get_description(icsneo_device_t* device, const char* value, uint32_t* value_length);

/** @brief Get the type of a device
 * 
 * @param[in] icsneo_device_t device The device to get the type of.
 * @param[out] icsneo_devicetype_t* value Pointer to an icsneo_devicetype_t to copy the type into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_get_type(icsneo_device_t* device, icsneo_devicetype_t* value);

/** @brief Get the serial of a device
 * 
 * @param[in] icsneo_device_t device The device to get the serial of.
 * @param[out] const char* value Pointer to a buffer to copy the serial into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the serial.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_get_serial(icsneo_device_t* device, const char* value, uint32_t* value_length);

/** @brief Set the online state of a device.
 * 
 * @param[in] icsneo_device_t device The device to set the online state of.
 * @param[in] bool go_online true to go online, false to go offline.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_go_online(icsneo_device_t* device, bool go_online);

/** @brief Get the online state of a device.
 * 
 * @param[in] icsneo_device_t device The device to get the online state of.
 * @param[out] bool true if online, false if offline.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_is_online(icsneo_device_t* device, bool* is_online);

/** @brief Get the online supported state of a device.
 * 
 * @param[in] icsneo_device_t device The device to get the online supported state of.
 * @param[out] bool true if online, false if offline.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_is_online_supported(icsneo_device_t* device, bool* is_online_supported);


/** @brief Set the message polling state of a device.
 * 
 * @param[in] icsneo_device_t device The device to set the message polling state of.
 * @param[in] bool enable true to enable message polling, false to disable message polling..
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_set_message_polling(icsneo_device_t* device, bool enable);

/** @brief Get the message polling state of a device.
 * 
 * @param[in] icsneo_device_t device The device to set the message polling state of.
 * @param[out] bool is_enabled true if message polling is enabled, false if message polling is disabled.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_get_message_polling(icsneo_device_t* device, bool* is_enabled);

/** @brief Set the message polling limit of a device.
 * 
 * This will truncate the message queue to the specified limit.
 * 
 * @param[in] icsneo_device_t device The device to enforce the message polling limit.
 * @param[in] uint32_t limit The limit to enforce.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_device_set_message_polling_limit(icsneo_device_t* device, uint32_t limit);

/** @brief Get the message polling limit of a device.
 * 
 * @see icsneo_device_set_message_polling_limit
 * 
 * @param[in] icsneo_device_t device The device to enforce the message polling limit.
 * @param[out] uint32_t limit The limit to get.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_device_get_message_polling_limit(icsneo_device_t* device, uint32_t* limit);

/** @brief Get the message count of a device
 * 
 * @param[in] icsneo_device_t device The device to get the message count of.
 * @param[out] uint32_t* count Pointer to a uint32_t to copy the message count into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_get_message_count(icsneo_device_t* device, uint32_t* count);


/** @brief Get the timestamp resolution (nanoseconds) of a device
 * 
 * @param[in] icsneo_device_t device The device to get the timestamp resolution of.
 * @param[out] uint32_t* resolution Pointer to a uint32_t to copy the timestamp resolution in nanoseconds into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_get_timestamp_resolution(icsneo_device_t* device, uint32_t* resolution);

/** @brief Get the messages of a device
 * 
 * When calling this function, the previous messages retrieved by this function will be invalid.
 * 
 * @param[in] icsneo_device_t device The device to get the messages of.
 * @param[out] icsneo_message_t** messages Pointer to an array of icsneo_message_t to copy the messages into.
 *  Undefined behaviour if index is out of range of messages_count.
 * @param[in,out] uint32_t* messages_count Size of the messages array. Modified with the number of messages found.
 * @param[in] uint32_t timeout_ms The timeout in milliseconds to wait for messages. A value of 0 indicates a non-blocking call.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_get_messages(icsneo_device_t* device, icsneo_message_t** messages, uint32_t* messages_count, uint32_t timeout_ms);

/** @brief Check if a message is valid
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] bool* is_valid Pointer to a bool to copy the validity of the message into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_message_is_valid(icsneo_device_t* device, icsneo_message_t* message, bool* is_valid);

/** @brief Get the type of a message
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] icsneo_msg_type_t* msg_type Pointer to a icsneo_msg_type_t to copy the type of the value into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 * 
 * @see icsneo_msg_type_t
 */
ICSNEO_API icsneo_error_t icsneo_message_get_type(icsneo_device_t* device, icsneo_message_t* message, icsneo_msg_type_t* msg_type);

/** @brief Get the type of a bus message
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] icsneo_msg_type_t* msg_type Pointer to a icsneo_msg_type_t to copy the type of the value into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters or icsneo_error_invalid_type otherwise.
 * 
 * @see icsneo_msg_bus_type_t
 */
ICSNEO_API icsneo_error_t icsneo_message_get_bus_type(icsneo_device_t* device, icsneo_message_t* message, icsneo_msg_bus_type_t* bus_type);

/** @brief Get the data bytes of a message 
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] uint8_t* data Pointer to a uint8_t to copy the data bytes into.
 * @param[out] uint32_t* data_length Pointer to a uint32_t to copy the length of the data into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_message_get_data(icsneo_device_t* device, icsneo_message_t* message, uint8_t* data, uint32_t* data_length);

/** @brief Get the Arbitration ID of a CAN message 
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] uint32_t* value Pointer to a uint32_t to copy the Arbitration ID into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_can_message_get_arbid(icsneo_device_t* device, icsneo_message_t* message, uint32_t* value);

/** @brief Get the DLC on wire of a CAN message 
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] uint32_t* value Pointer to a uint32_t to copy the DLC on wire into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_can_message_get_dlc_on_wire(icsneo_device_t* device, icsneo_message_t* message, uint32_t* value);

/** @brief Get the remote status of a CAN message 
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] bool* value Pointer to a uint32_t to copy the remote status into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_can_message_is_remote(icsneo_device_t* device, icsneo_message_t* message, bool* value);

/** @brief Get the extended status of a CAN message 
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] bool* value Pointer to a uint32_t to copy the extended status into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_can_message_is_extended(icsneo_device_t* device, icsneo_message_t* message, bool* value);

/** @brief Get the CANFD status of a CAN message 
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] bool* value Pointer to a uint32_t to copy the CANFD status into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_can_message_is_canfd(icsneo_device_t* device, icsneo_message_t* message, bool* value);

/** @brief Get the baudrate switch status of a CAN message 
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] bool* value Pointer to a uint32_t to copy the baudrate switch status into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_can_message_get_baudrate_switch(icsneo_device_t* device, icsneo_message_t* message, bool* value);

/** @brief Get the error state indicator status of a CAN message 
 * 
 * @param[in] icsneo_device_t* device The device to check against.
 * @param[in] icsneo_message_t* message The message to check.
 * @param[out] bool* value Pointer to a uint32_t to copy the error state indicator status into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_can_message_get_error_state_indicator(icsneo_device_t* device, icsneo_message_t* message, bool* value);

/** @brief Get the global events not specifically related to a device.
 * 
 * @param[out] icsneo_event_t** events Pointer to an array of icsneo_event_t to copy the events into.
 *  Undefined behaviour if index is out of range of events_count.
 * @param[in,out] uint32_t* events_count Size of the events array. Modified with the number of events found.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_get_events(icsneo_event_t** events, uint32_t* events_count);

/** @brief Get the events of a device.
 * 
 * @param[in] icsneo_device_t device The device to get the events of.
 * @param[out] icsneo_event_t** events Pointer to an array of icsneo_event_t to copy the events into.
 *  Undefined behaviour if index is out of range of events_count.
 * @param[in,out] uint32_t* events_count Size of the events array. Modified with the number of events found.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_get_events(icsneo_device_t* device, icsneo_event_t** events, uint32_t* events_count);

/** @brief Get the error string for an error code.
 * 
 * @param[in] icsneo_event_t* event The event to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_event_get_description(icsneo_event_t* event, const char* value, uint32_t* value_length);

/** @brief Get the RTC (Real time clock) of a device.
 * 
 * @param[in] icsneo_device_t device The device to get the RTC of.
 * @param[out] int64_t* unix_epoch Pointer to an int64_t to copy the RTC into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_get_rtc(icsneo_device_t* device, int64_t* unix_epoch);

/** @brief Set the RTC (Real time clock) of a device.
 * 
 * @param[in] icsneo_device_t device The device to get the RTC of.
 * @param[in] int64_t* unix_epoch int64_t to copy the RTC into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_set_rtc(icsneo_device_t* device, int64_t* unix_epoch);

/** @brief Load the default settings for a device
 * 
 * @param[in] icsneo_device_t device The device to load the settings for.
 * @param[in] bool save True to make the settings permanent, false settings will be reverted on next boot.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_load_default_settings(icsneo_device_t* device, bool save);
#ifdef __cplusplus
}
#endif
