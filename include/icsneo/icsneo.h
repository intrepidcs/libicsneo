#pragma once

#include <stdint.h>

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

typedef enum _icsneo_error_t {
    icsneo_error_success,
    icsneo_error_invalid_parameters,
    icsneo_error_open_failed,
    icsneo_error_open_gonline_failed,
    icsneo_error_open_message_polling_failed,
    icsneo_error_open_sync_rtc_failed,
} _icsneo_error_t;

typedef uint32_t icsneo_error_t;



/** @brief Find all hardware attached to the system. 
 * 
 * @param[out] icsneo_device_t array of devices to be filled with found devices. Last element will be NULL.
 * @param[in,out] uint32_t* devices_count Size of the devices array. Modified with the number of devices found.
 * @param[in] void* reserved Reserved for future use. Currently unused and must be set to NULL.
 * 
 * @return icsneo_error_t icsneo_error_success if successful, icsneo_error_failure otherwise.
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

ICSNEO_API icsneo_error_t icsneo_device_describe(icsneo_device_t* device, const char* value, uint32_t* value_length);
ICSNEO_API icsneo_error_t icsneo_device_type(icsneo_device_t* device, uint64_t* value);
ICSNEO_API icsneo_error_t icsneo_device_serial(icsneo_device_t* device, const char* value, uint32_t* value_length);

#ifdef __cplusplus
}
#endif