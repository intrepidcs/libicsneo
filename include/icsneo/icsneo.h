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

#ifdef ICSNEO_EXPORTS
#define ICSNEO_API EXPORT
#else
#define ICSNEO_API IMPORT
#endif

/** @brief icsneo_device_t opaque struct definition
 * 
 * This object represents a single device found on the system.
 * 
 * @see icsneo_find_devices
 */
typedef struct icsneo_device_t icsneo_device_t;


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
ICSNEO_API icsneo_error_t icsneo_error_code(icsneo_error_t error_code, const char* value, uint32_t* value_length);

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
ICSNEO_API icsneo_error_t icsneo_find(icsneo_device_t** devices, uint32_t* devices_count, void* reserved);

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
ICSNEO_API icsneo_error_t icsneo_get_open_options(icsneo_device_t* device, icsneo_open_options_t* options);

/** @brief Set the open options for a device 
 * 
 * @param[in] icsneo_device_t device The device to set options for.
 * @param[in] icsneo_open_options_t options Options to set for the device.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
*/
ICSNEO_API icsneo_error_t icsneo_set_open_options(icsneo_device_t* device, icsneo_open_options_t options);

/** @brief Open a connection to a device.
 * 
 * After a successful call to this function, icsneo_close() must be called to close the device.
 * 
 * @param[in] icsneo_device_t device The device to open.
 * @param[out] icsneo_handle_t* handle Pointer to a handle to the opened device. Will be NULL on failure.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_failure otherwise.
 * 
 * @see icsneo_close
 */
ICSNEO_API icsneo_error_t icsneo_open(icsneo_device_t* device);

/** @brief Close a connection to a previously opened device.
 * 
 * After a successful call to icsneo_open(), this function must be called to close the device.
 * An already closed device will still succeed.
 * 
 * @param[in] icsneo_device_t device The device to close.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_failure otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_close(icsneo_device_t* device);

/** @brief Get the description of a device
 * 
 * @param[in] icsneo_device_t device The device to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_describe(icsneo_device_t* device, const char* value, uint32_t* value_length);

/** @brief Get the description type a device
 * 
 * @param[in] icsneo_device_t device The device to get the description of.
 * @param[out] icsneo_devicetype_t* value Pointer to an icsneo_devicetype_t to copy the type into.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_type(icsneo_device_t* device, icsneo_devicetype_t* value);

/** @brief Get the description of a device
 * 
 * @param[in] icsneo_device_t device The device to get the description of.
 * @param[out] const char* value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] uint32_t* value_length Size of the value buffer. Modified with the length of the description.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_device_serial(icsneo_device_t* device, const char* value, uint32_t* value_length);


/** @brief Set the online state of a device.
 * 
 * @param[in] icsneo_device_t device The device to set the online state of.
 * @param[in] bool go_online true to go online, false to go offline.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_go_online(icsneo_device_t* device, bool go_online);

/** @brief Get the online state of a device.
 * 
 * @param[in] icsneo_device_t device The device to set the online state of.
 * @param[out] bool true if online, false if offline.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_is_online(icsneo_device_t* device, bool* is_online);


/** @brief Set the message polling state of a device.
 * 
 * @param[in] icsneo_device_t device The device to set the message polling state of.
 * @param[in] bool enable true to enable message polling, false to disable message polling..
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_set_message_polling(icsneo_device_t* device, bool enable);

/** @brief Get the message polling state of a device.
 * 
 * @param[in] icsneo_device_t device The device to set the message polling state of.
 * @param[out] bool is_enabled true if message polling is enabled, false if message polling is disabled.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_invalid_parameters otherwise.
 */
ICSNEO_API icsneo_error_t icsneo_get_message_polling(icsneo_device_t* device, bool* is_enabled);



#ifdef __cplusplus
}
#endif