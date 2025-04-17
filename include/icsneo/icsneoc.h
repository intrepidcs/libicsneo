#ifndef __ICSNEOC_H_
#define __ICSNEOC_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "icsneo/device/neodevice.h" // For neodevice_t
#include "icsneo/communication/message/neomessage.h" // For neomessage_t and friends
#include "icsneo/platform/dynamiclib.h" // Dynamic library loading and exporting
#include "icsneo/communication/network.h" // Network type and netID defines
#include "icsneo/communication/io.h" // IO enum defines
#include "icsneo/api/version.h" // For version info
#include "icsneo/api/event.h" // For event and error info

#ifndef ICSNEOC_DYNAMICLOAD

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Find Intrepid hardware connected via USB and Ethernet.
 * \param[out] devices Pointer to memory where devices should be written. If NULL, the current number of detected devices is written to count.
 * \param[inout] count Pointer to a size_t, which should initially contain the number of devices the buffer can hold,
 * and will afterwards contain the number of devices found.
 *
 * For each found device, a neodevice_t structure will be written into the memory you provide.
 *
 * The neodevice_t can later be passed by reference into the API to perform actions relating to the device.
 * The neodevice_t contains a handle to the internal memory for the icsneo::Device object.
 * The memory for the internal icsneo::Device object is managed by the API.
 *
 * Any neodevice_t objects which have not been opened will become invalid when icsneo_findAllDevices() is called again.
 * To invoke this behavior without finding devices again, call icsneo_freeUnconnectedDevices().
 *
 * If the size provided is not large enough, the output will be truncated.
 * An icsneo::APIEvent::OutputTruncatedError will be available in icsneo_getLastError() in this case.
 */
extern void DLLExport icsneo_findAllDevices(neodevice_t* devices, size_t* count);

/**
 * \brief Invalidate neodevice_t objects which have not been opened.
 *
 * See icsneo_findAllDevices() for information regarding the neodevice_t validity contract.
 */
extern void DLLExport icsneo_freeUnconnectedDevices(void);

/**
 * \brief Convert a serial number in numerical format to its string representation.
 * \param[in] num A numerical serial number.
 * \param[out] str A pointer to a buffer where the string representation will be written. NULL can be passed, which will write a character count to `count`.
 * \param[inout] count A pointer to a size_t which, prior to the call,
 * holds the maximum number of characters to be written (so str must be of size count + 1 to account for the NULL terminator),
 * and after the call holds the number of characters written.
 * \returns True if str contains the string representation of the given serial number.
 *
 * On older devices, the serial number is one like 138635, the numerical representation is the same as the string representation.
 *
 * On newer devices, the serial number is one like RS2259, and this function can convert the numerical value back into the string seen on the back of the device.
 *
 * A query for length (`str == NULL`) will return false.
 * icsneo_getLastError() should be checked to verify that the neodevice_t provided was valid.
 *
 * The client application should provide a buffer of size 7, as serial numbers are always 6 characters or fewer.
 *
 * If the size provided is not large enough, the output will be **NOT** be truncated.
 * Nothing will be written to the output.
 * Instead, an icsneo::APIEvent::BufferInsufficient will be available in icsneo_getLastError().
 * False will be returned, and `count` will now contain the number of *bytes* necessary to store the full string.
 */
extern bool DLLExport icsneo_serialNumToString(uint32_t num, char* str, size_t* count);

/**
 * \brief Convert a serial number in string format to its numerical representation.
 * \param[in] str A NULL terminated string containing the string representation of an Intrepid serial number.
 * \returns The numerical representation of the serial number, or 0 if the conversion was unsuccessful.
 *
 * On older devices, the serial number is one like 138635, and this string will simply be returned as a number.
 *
 * On newer devices, the serial number is one like RS2259, and this function can convert that string to a number.
 */
extern uint32_t DLLExport icsneo_serialStringToNum(const char* str);

/**
 * \brief Verify that a neodevice_t is valid.
 * \param[in] device A pointer to the neodevice_t structure to operate on.
 * \returns True if the neodevice_t is valid.
 *
 * This check is automatically performed at the beginning of any API function that operates on a device.
 * If there is a failure, an icsneo::APIEvent::InvalidNeoDevice will be available in icsneo_getLastError().
 *
 * See icsneo_findAllDevices() for information regarding the neodevice_t validity contract.
 */
extern bool DLLExport icsneo_isValidNeoDevice(const neodevice_t* device);

/**
 * \brief Connect to the specified hardware
 * \param[in] device A pointer to the neodevice_t structure specifying the device to open.
 * \returns True if the connection could be opened.
 *
 * The device **MUST** be opened before any other functions which operate on the device will be valid.
 *
 * See icsneo_goOnline() for information about enabling network communication once the device is open.
 *
 * If the open did not succeed, icsneo_getLastError() should provide more information about why.
 *
 * If the device was already open, an icsneo::APIEvent::DeviceCurrentlyOpen will be available in icsneo_getLastError().
 */
extern bool DLLExport icsneo_openDevice(const neodevice_t* device);

/**
 * \brief Close an open connection to the specified hardware
 * \param[in] device A pointer to the neodevice_t structure specifying the device to close.
 * \returns True if the connection could be closed.
 *
 * After this function succeeds, the neodevice_t will be invalid.
 * To connect again, you must call icsneo_findAllDevices() or similar to re-find the device.
 */
extern bool DLLExport icsneo_closeDevice(const neodevice_t* device);

/**
 * \brief Verify network connection for the specified hardware
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if the device is connected.
 *
 * This function does not modify the working state of the device at all.
 *
 * See icsneo_openDevice() for an explanation about the concept of being "open".
 */
extern bool DLLExport icsneo_isOpen(const neodevice_t* device);

/**
 * \brief Enable network communication for the specified hardware
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if communication could be enabled.
 *
 * The device is not "online" when it is first opened. It is not possible to receive or transmit while the device is "offline".
 * Network controllers are disabled. (i.e. In the case of CAN, the hardware will not send ACKs on the client application's behalf)
 *
 * This allows filtering or handlers to be set up before allowing traffic to flow.
 *
 * This also allows device settings to be set (i.e. baudrates) before enabling the controllers,
 * which prevents momentarily causing loss of communication if the baud rates are not correct.
 */
extern bool DLLExport icsneo_goOnline(const neodevice_t* device);

/**
 * \brief Disable network communication for the specified hardware
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if communication could be disabled.
 *
 * See icsneo_goOnline() for an explanation about the concept of being "online".
 */
extern bool DLLExport icsneo_goOffline(const neodevice_t* device);

/**
 * \brief Verify network communication for the specified hardware
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if communication is enabled.
 *
 * This function does not modify the working state of the device at all.
 *
 * See icsneo_goOnline() for an explanation about the concept of being "online".
 */
extern bool DLLExport icsneo_isOnline(const neodevice_t* device);

/**
 * \brief Enable buffering of messages for the specified hardware
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if polling could be enabled.
 *
 * By default, traffic the device receives will not reach the client application.
 * The client application must register traffic handlers, enable message polling, or both.
 * This function addresses message polling.
 *
 * With polling enabled, all traffic that the device receives will be stored in a buffer managed by the API.
 * The client application should then call icsneo_getMessages() periodically to take ownership of the messages in that buffer.
 *
 * The API managed buffer will only grow to a specified size, 20k messages by default.
 * See icsneo_getPollingMessageLimit() and icsneo_setPollingMessageLimit() for more information.
 *
 * In high traffic situations, the default 20k message limit can be reached very quickly.
 * The client application will have to call icsneo_getMessages() very often to avoid losing messages, or change the limit.
 *
 * If the message limit is exceeded before a call to icsneo_getMessages() takes ownership of the messages,
 * the oldest message will be dropped (**LOST**) and an icsneo::APIEvent::PollingMessageOverflow will be flagged for the device.
 *
 * This function will succeed even if the device is not open.
 */
extern bool DLLExport icsneo_enableMessagePolling(const neodevice_t* device);

/**
 * \brief Disable buffering of messages for the specified hardware
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if polling could be disabled.
 *
 * See icsneo_enableMessagePolling() for more information about the message polling system.
 *
 * Any messages left in the API managed buffer will be lost upon disabling polling.
 */
extern bool DLLExport icsneo_disableMessagePolling(const neodevice_t* device);

/**
 * \brief Verify message polling status for the specified hardware
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if polling is enabled.
 *
 *  This function does not modify the working state of the device at all.
 *
 * See icsneo_enableMessagePolling() for an explanation about how polling works.
 */
extern bool DLLExport icsneo_isMessagePollingEnabled(const neodevice_t* device);

/**
 * \brief Read out messages which have been recieved
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[out] messages A pointer to a buffer which neomessage_t structures will be written to. NULL can be passed, which will write the current message count to size.
 * \param[inout] items A pointer to a size_t which, prior to the call,
 * holds the maximum number of messages to be written, and after the call holds the number of messages written.
 * \param[in] timeout The number of milliseconds to wait for a message to arrive. A value of 0 indicates a non-blocking call.
 * Querying for the current message count is always asynchronous and ignores this value.
 * \returns True if the messages were read out successfully (even if there were no messages to read) or if the count was read successfully.
 *
 * Messages are available using this function if icsneo_goOnline() and icsneo_enableMessagePolling() have been called.
 * See those functions for more information.
 *
 * Messages are read out of the API managed buffer in order of oldest to newest.
 * As they are read out, they are removed from the API managed buffer.
 *
 * If size is too small to contain all messages, as many messages as will fit will be read out.
 * Subsequent calls to icsneo_getMessages() can retrieve any messages which were not read out.
 *
 * The memory for the data pointer within the neomessage_t is managed by the API. Do *not* attempt to free the data pointer.
 * The memory will become invalid the next time icsneo_getMessages() is called for this device.
 *
 * \warning Do not call icsneo_close() while another thread is waiting on icsneo_getMessages().
 * Always allow the other thread to timeout first!
 *
 * ``` C
 * size_t messageCount;
 * bool result = icsneo_getMessages(device, NULL, &messageCount, 0); // Reading the message count
 * // Handle errors here
 * neomessage_t* messages = malloc(messageCount * sizeof(neomessage_t)); // It is also possible and encouraged to use a static buffer
 * result = icsneo_getMessages(device, messages, &messageCount, 0); // Non-blocking
 * // Handle errors here
 * for(size_t i = 0; i < messageCount; i++) {
 * 	switch(messages[i].type) {
 * 		case ICSNEO_NETWORK_TYPE_CAN: {
 * 			// All messages of type CAN can be accessed using neomessage_can_t
 * 			neomessage_can_t* canMessage = (neomessage_can_t*)&messages[i];
 * 			printf("ArbID: 0x%x\n", canMessage->arbid);
 * 			printf("DLC: %u\n", canMessage->length);
 * 			printf("Data: ");
 * 			for(size_t i = 0; i < canMessage->length; i++) {
 * 				printf("%02x ", canMessage->data[i]);
 * 			}
 * 			printf("\nTimestamp: %lu\n", canMessage->timestamp);
 * 		}
 * 	}
 * }
 * free(messages);
 * ```
 */
extern bool DLLExport icsneo_getMessages(const neodevice_t* device, neomessage_t* messages, size_t* items, uint64_t timeout);

/**
 * \brief Get the maximum number of messages which will be held in the API managed buffer for the specified hardware.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns Number of messages, or -1 if device is invalid.
 *
 * See icsneo_enableMessagePolling() for more information about the message polling system.
 */
extern int DLLExport icsneo_getPollingMessageLimit(const neodevice_t* device);

/**
 * \brief Set the maximum number of messages which will be held in the API managed buffer for the specified hardware.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] newLimit The new limit to be enforced.
 * \returns True if the limit was set successfully.
 *
 * See icsneo_enableMessagePolling() for more information about the message polling system.
 *
 * Setting the maximum lower than the current number of stored messages will cause the oldest messages
 * to be dropped (**LOST**) and an icsneo::APIEvent::PollingMessageOverflow to be flagged for the device.
 */
extern bool DLLExport icsneo_setPollingMessageLimit(const neodevice_t* device, size_t newLimit);

/**
 * \brief Adds a message callback to the specified device to be called when a new message is received.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] callback A function pointer with void return type and a single neomessage_t parameter.
 * \param[in] filter Unused for now. Exists as a placeholder here for future backwards-compatibility.
 * \returns The id of the callback added, or -1 if the operation failed.
 */
extern int DLLExport icsneo_addMessageCallback(const neodevice_t* device, void (*callback)(neomessage_t), void*);

/**
 * \brief Removes a message callback from the specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] id The id of the callback to remove.
 * \returns True if the callback was successfully removed.
 */
extern bool DLLExport icsneo_removeMessageCallback(const neodevice_t* device, int id);

/**
 * \brief Get the network ID for the nth network of a specified type on this device
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] type An ICSNEO_NETWORK_TYPE_* constant denoting the network type
 * \param[in] number The number of this network starting from 1
 * \returns The netid if the call succeeds, ICSNEO_NETID_INVALID otherwise
 *
 * This function is designed so that networks can be enumerated without knowledge of the specific
 * device. For instance, on a ValueCAN 4-2, the second CAN network is ICSNEO_NETID_DWCAN_02, while
 * on a neoVI FIRE the second CAN network is ICSNEO_NETID_DWCAN_08.
 */
extern neonetid_t DLLExport icsneo_getNetworkByNumber(const neodevice_t* device, neonettype_t type, unsigned int number);

/**
 * \brief Get the friendly product name for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[out] str A pointer to a buffer where the string will be written. NULL can be passed, which will write a character count to maxLength.
 * \param[inout] maxLength A pointer to a size_t which, prior to the call,
 * holds the maximum number of characters to be written (so str must be of size maxLength + 1 to account for the NULL terminator),
 * and after the call holds the number of characters written.
 * \returns True if str was written to
 *
 * In the case of a neoVI FIRE 2, this function will write a string "neoVI FIRE 2" with a NULL terminator into str.
 *
 * The constant ICSNEO_DEVICETYPE_LONGEST_NAME is defined for the client application to create static buffers of the correct length.
 *
 * See also icsneo_describeDevice().
 *
 * A query for length (`str == NULL`) will return false.
 * icsneo_getLastError() should be checked to verify that the neodevice_t provided was valid.
 *
 * If the size provided is not large enough, the output will be truncated.
 * An icsneo::APIEvent::OutputTruncatedError will be available in icsneo_getLastError() in this case.
 * True will still be returned.
 */
extern bool DLLExport icsneo_getProductName(const neodevice_t* device, char* str, size_t* maxLength);

/**
 * \brief Get the friendly product name for a specified devicetype.
 * \param[in] type A neodevice_t structure specifying the device to operate on.
 * \param[out] str A pointer to a buffer where the string will be written. NULL can be passed, which will write a character count to maxLength.
 * \param[inout] maxLength A pointer to a size_t which, prior to the call,
 * holds the maximum number of characters to be written (so str must be of size maxLength + 1 to account for the NULL terminator),
 * and after the call holds the number of characters written.
 * \returns True if str was written to
 *
 * In the case of a neoVI FIRE 2, this function will write a string "neoVI FIRE 2" with a NULL terminator into str.
 *
 * Note that icsneo_getProductName should *always* be preferred where available, as the product name may change based on device-specific
 * factors, such as the serial number.
 *
 * The constant ICSNEO_DEVICETYPE_LONGEST_NAME is defined for the client application to create static buffers of the correct length.
 *
 * See also icsneo_describeDevice().
 *
 * A query for length (`str == NULL`) will return false.
 * icsneo_getLastError() should be checked to verify that the neodevice_t provided was valid.
 *
 * If the size provided is not large enough, the output will be truncated.
 * An icsneo::APIEvent::OutputTruncatedError will be available in icsneo_getLastError() in this case.
 * True will still be returned.
 */
extern bool DLLExport icsneo_getProductNameForType(devicetype_t type, char* str, size_t* maxLength);

/**
 * \brief Trigger a refresh of the settings structure for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if the refresh succeeded.
 */
extern bool DLLExport icsneo_settingsRefresh(const neodevice_t* device);

/**
 * \brief Commit the settings structure for a specified device to non-volatile storage.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if the settings were applied.
 *
 * When modifications are made to the device settings, this function (or icsneo_settingsApplyTemporary()) must be called to send the changes to the device and make them active.
 *
 * This function sets the settings such that they will survive device power cycles.
 *
 * If the function fails, the settings will be refreshed so that the structure in the API matches the one held by the device.
 */
extern bool DLLExport icsneo_settingsApply(const neodevice_t* device);

/**
 * \brief Apply the settings structure for a specified device temporarily.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if the settings were applied.
 *
 * See icsneo_settingsApply() for further information about applying settings.
 *
 * This function sets the settings such that they will revert to the values saved in non-volatile storage when the device loses power.
 */
extern bool DLLExport icsneo_settingsApplyTemporary(const neodevice_t* device);

/**
 * \brief Apply the default settings structure for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if the default settings were applied.
 *
 * See icsneo_settingsApply() for further information about applying settings.
 *
 * This function sets the default settings such that they will survive device power cycles.
 */
extern bool DLLExport icsneo_settingsApplyDefaults(const neodevice_t* device);

/**
 * \brief Apply the default settings structure for a specified device temporarily.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \returns True if the default settings were applied.
 *
 * See icsneo_settingsApply() for further information about applying settings. See icsneo_settingsApplyDefaults() for further information about applying default settings.
 *
 * This function sets the default settings such that they will revert to the values saved in non-volatile storage when the device loses power.
 */
extern bool DLLExport icsneo_settingsApplyDefaultsTemporary(const neodevice_t* device);

/**
 * \brief Apply the default settings structure for a specified device temporarily.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[out] structure A pointer to a device settings structure for the current device.
 * \param[in] structureSize The size of the current device settings structure in bytes.
 * \returns Number of bytes written to structure, or -1 if the operation failed.
 *
 * See icsneo_settingsApply() for further information about applying settings. See icsneo_settingsApplyDefaults() for further information about applying default settings.
 *
 * This function sets the default settings such that they will revert to the values saved in non-volatile storage when the device loses power.
 *
 * If possible, use functions specific to the operation you want to acomplish (such as icsneo_setBaudrate()) instead of modifying the structure directly.
 * This allows the client application to work with other hardware.
 */
extern int DLLExport icsneo_settingsReadStructure(const neodevice_t* device, void* structure, size_t structureSize);

/**
 * \brief Apply a provided settings structure for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] structure A pointer to a device settings structure for the current device.
 * \param[in] structureSize The size of the current device settings structure in bytes.
 * \returns True if the settings were applied.
 *
 * This function immediately applies the provided settings. See icsneo_settingsApplyTemporary() for further information about applying settings.
 *
 * If possible, use functions specific to the operation you want to acomplish (such as icsneo_setBaudrate()) instead of modifying the structure directly.
 * This allows the client application to work with other hardware.
 */
extern bool DLLExport icsneo_settingsApplyStructure(const neodevice_t* device, const void* structure, size_t structureSize);

/**
 * \brief Apply a provided settings structure for a specified device without saving to non-volatile EEPROM.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] structure A pointer to a device settings structure for the current device.
 * \param[in] structureSize The size of the current device settings structure in bytes.
 * \returns True if the settings were applied.
 *
 * This function immediately applies the provided settings. See icsneo_settingsApply() for further information about applying settings.
 *
 * This function sets the default settings such that they will revert to the values saved in non-volatile storage when the device loses power.
 *
 * If possible, use functions specific to the operation you want to acomplish (such as icsneo_setBaudrate()) instead of modifying the structure directly.
 * This allows the client application to work with other hardware.
 */
extern bool DLLExport icsneo_settingsApplyStructureTemporary(const neodevice_t* device, const void* structure, size_t structureSize);

/**
 * \brief Get the network baudrate for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] netid The network for which the baudrate should be retrieved.
 * \returns The value in baud with no multipliers. (i.e. 500k becomes 500000) A negative value is returned if an error occurs.
 *
 * In the case of CAN, this function gets the standard CAN baudrate.
 * See icsneo_getFDBaudrate() to get the baudrate for (the baudrate-switched portion of) CAN FD.
 */
extern int64_t DLLExport icsneo_getBaudrate(const neodevice_t* device, neonetid_t netid);

/**
 * \brief Set the network baudrate for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] netid The network to which the new baudrate should apply.
 * \param[in] newBaudrate The requested baudrate, with no multipliers. (i.e. 500K CAN should be represented as 500000)
 * \returns True if the baudrate could be set.
 *
 * In the case of CAN, this function sets the standard CAN baudrate.
 * See icsneo_setFDBaudrate() to set the baudrate for (the baudrate-switched portion of) CAN FD.
 *
 * Call icsneo_settingsApply() or similar to make the changes active on the device.
 */
extern bool DLLExport icsneo_setBaudrate(const neodevice_t* device, neonetid_t netid, int64_t newBaudrate);

/**
 * \brief Get the CAN FD baudrate for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] netid The network for which the baudrate should be retrieved.
 * \returns The value in baud with no multipliers. (i.e. 500k becomes 500000) A negative value is returned if an error occurs.
 *
 * See icsneo_getBaudrate() to get the baudrate for the non baudrate-switched portion of CAN FD, classical CAN 2.0, and other network types.
 */
extern int64_t DLLExport icsneo_getFDBaudrate(const neodevice_t* device, neonetid_t netid);

/**
 * \brief Set the CAN FD baudrate for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] netid The network to which the new baudrate should apply.
 * \param[in] newBaudrate The requested baudrate, with no multipliers. (i.e. 2Mbaud CAN FD should be represented as 2000000)
 * \returns True if the baudrate could be set.
 *
 * See icsneo_setBaudrate() to set the baudrate for the non baudrate-switched portion of CAN FD, classical CAN 2.0, and other network types.
 *
 * Call icsneo_settingsApply() or similar to make the changes active on the device.
 */
extern bool DLLExport icsneo_setFDBaudrate(const neodevice_t* device, neonetid_t netid, int64_t newBaudrate);

/**
 * \brief Transmit a single message.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to transmit on.
 * \param[in] message A pointer to the neomessage_t structure defining the message.
 * \returns True if the message was verified transmittable and enqueued for transmit.
 *
 * To transmit a message, you must set the `data`, `length`, and `netid` attributes of the neomessage_t.
 *
 * The `data` attribute must be set to a pointer to a buffer of at least `length` which holds the payload bytes.
 * This buffer only needs to be valid for the duration of this call, and can safely be deallocated or reused after the return.
 *
 * You may also have to set network dependent variables.
 * For CAN, you must set the `arbid` attribute defined in neomessage_can_t.
 *
 * Other attributes of the neomessage_t such as `timestamp`, `type` and `reserved` which are not used should be set to 0. Unused status bits should also be set to 0.
 *
 * Any types defined `neomessage_*_t` are designed to be binary compatible with neomessage_t.
 *
 * For instance, for CAN, it is recommended to use neomessage_can_t as it exposes the `arbid` field.
 *
 * ``` C
 * neomessage_can_t mySendMessage = {}; // Zero all before use
 * uint8_t myData[3] = { 0xAA, 0xBB, 0xCC }; // Either heap or stack allocated is okay
 * mySendMessage.netid = ICSNEO_NETID_DWCAN_01;
 * mySendMessage.arbid = 0x1c5001c5;
 * mySendMessage.length = 3;
 * mySendMessage.data = myData;
 * mySendMessage.status.canfdFDF = true; // CAN FD
 * mySendMessage.status.extendedFrame = true; // Extended (29-bit) arbitration IDs
 * mySendMessage.status.canfdBRS = true; // CAN FD Baudrate Switch
 * bool result = icsneo_transmit(device, (neomessage_t*)&mySendMessage);
 *
 * myData[1] = 0x55; // The message and buffer can be safely reused for the next message
 * result = icsneo_transmit(device, (neomessage_t*)&mySendMessage);
 * ```
 */
extern bool DLLExport icsneo_transmit(const neodevice_t* device, const neomessage_t* message);

/**
 * \brief Transmit a multiple messages.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to transmit on.
 * \param[in] messages A pointer to the neomessage_t structures defining the messages.
 * \param[in] count The number of messages to transmit.
 * \returns True if the messages were verified transmittable and enqueued for transmit.
 *
 * See icsneo_transmit() for information regarding transmitting messages.
 *
 * On a per-network basis, messages will be transmitted in the order that they were enqueued.
 *
 * In this case, messages will be enqueued in order of increasing index.
 */
extern bool DLLExport icsneo_transmitMessages(const neodevice_t* device, const neomessage_t* messages, size_t count);

/**
 * \brief Set the behavior of whether writing is a blocking action or not.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to transmit on.
 * \param[in] blocks Whether or not writing is a blocking action.
 *
 * By default, writing is a blocking action.
 */
extern void DLLExport icsneo_setWriteBlocks(const neodevice_t* device, bool blocks);

/**
 * \brief Get the friendly description for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[out] str A pointer to a buffer where the string will be written. NULL can be passed, which will write a character count to maxLength.
 * \param[inout] maxLength A pointer to a size_t which, prior to the call,
 * holds the maximum number of characters to be written (so str must be of size maxLength + 1 to account for the NULL terminator),
 * and after the call holds the number of characters written.
 * \returns True if str was written to
 *
 * In the case of a neoVI FIRE 2 with serial number CY2285, this function will write a string "neoVI FIRE 2 CY2285" with a NULL terminator into str.
 *
 * The constant ICSNEO_DEVICETYPE_LONGEST_DESCRIPTION is defined for the client application to create static buffers of the correct length.
 *
 * See also icsneo_getProductName().
 *
 * A query for length (`str == NULL`) will return false.
 * icsneo_getLastError() should be checked to verify that the neodevice_t provided was valid.
 *
 * If the size provided is not large enough, the output will be truncated.
 * An icsneo::APIEvent::OutputTruncatedError will be available in icsneo_getLastError() in this case.
 * True will still be returned.
 */
extern bool DLLExport icsneo_describeDevice(const neodevice_t* device, char* str, size_t* maxLength);

/**
 * \brief Get the version of libicsneo in use.
 * \returns A neoversion_t structure containing the version.
 */
extern neoversion_t DLLExport icsneo_getVersion(void);

/**
 * \brief Adds an event callback to be called when a new event is added.
 * \param[in] callback A function pointer with void return type and a single neoevent_t parameter.
 * \param[in] filter Unused for now. Exists as a placeholder here for future backwards-compatibility.
 * \returns The id of the callback added. Does not error.
 *
 * Do not attempt to add or remove callbacks inside of a callback, as the stored callbacks are locked during calls.
 */
extern int DLLExport icsneo_addEventCallback(void (*callback)(neoevent_t), void*);

/**
 * \brief Removes an event callback.
 * \param[in] id The id of the callback to remove.
 * \returns True if the callback was successfully removed.
 */
extern bool DLLExport icsneo_removeEventCallback(int id);

/**
 * \brief Read out events which have occurred in API operation
 * \param[out] events A pointer to a buffer which neoevent_t structures will be written to. NULL can be passed, which will write the current event count to size.
 * \param[inout] size A pointer to a size_t which, prior to the call,
 * holds the maximum number of events to be written, and after the call holds the number of events written.
 * \returns True if the events were read out successfully (even if there were no events to report).
 *
 * Events contain INFO and WARNINGS, and may potentially contain one TooManyEvents WARNING at the end. No ERRORS are found in Events, see icsneo_getLastError() instead.
 *
 * Events can be caused by API usage, such as providing too small of a buffer or disconnecting from a device.
 *
 * Events can also occur asynchronously to the client application threads, in the case of a device communication event or similar.
 *
 * Events are read out of the API managed buffer in order of oldest to newest.
 * As they are read out, they are removed from the API managed buffer.
 *
 * If size is too small to contain all events, as many events as will fit will be read out.
 * Subsequent calls to icsneo_getEvents() can retrieve any events which were not read out.
 */
extern bool DLLExport icsneo_getEvents(neoevent_t* events, size_t* size);

/**
 * \brief Read out events which have occurred in API operation for a specific device
 * \param[in] device A pointer to the neodevice_t structure specifying the device to read out events for. NULL can be passed, which indicates that **ONLY** events *not* associated with a device are desired (API events).
 * \param[out] events A pointer to a buffer which neoevent_t structures will be written to. NULL can be passed, which will write the current event count to size.
 * \param[inout] size A pointer to a size_t which, prior to the call,
 * holds the maximum number of events to be written, and after the call holds the number of events written.
 * \returns True if the events were read out successfully (even if there were no events to report).
 *
 * See icsneo_getEvents() for more information about the event system.
 */
extern bool DLLExport icsneo_getDeviceEvents(const neodevice_t* device, neoevent_t* events, size_t* size);

/**
 * \brief Read out the last error which occurred in API operation on this thread.
 * \param[out] error A pointer to a buffer which a neoevent_t structure will be written to.
 * \returns True if an error was read out.
 *
 * All errors are stored on a per-thread basis, meaning that calling icsneo_getLastError() will return the last error that occured on the calling thread.
 * Any errors can only be retrieved through this function, and NOT icsneo_getEvents() or similar! Only INFO and WARNING level events are accessible through those.
 * Only the last error is stored, so the intention is for this function to be called immediately following another failed API call.
 *
 * The API error system is thread-safe. Only an API error which occurred on the current thread will be returned.
 *
 * See icsneo_getEvents() for more information about the event system.
 *
 * This operation removes the returned error from the buffer, so subsequent calls to error functions will not include the error.
 */
extern bool DLLExport icsneo_getLastError(neoevent_t* error);

/**
 * \brief Discard all events which have occurred in API operation. Does NOT discard any errors.
 */
extern void DLLExport icsneo_discardAllEvents(void);

/**
 * \brief Discard all events which have occurred in API operation.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to discard events for. NULL can be passed, which indicates that **ONLY** events *not* associated with a device are desired (API events).
 * Does NOT discard any errors (device or otherwise).
 */
extern void DLLExport icsneo_discardDeviceEvents(const neodevice_t* device);

/**
 * \brief Set the number of events which will be held in the API managed buffer before icsneo::APIEvent::TooManyEvents
 * \param[in] newLimit The new limit. Must be >10. 1 event slot is always reserved for a potential icsneo::APIEvent::TooManyEvents, so (newLimit - 1) other events can be stored.
 *
 * If the event limit is reached, an icsneo::APIEvent::TooManyEvents will be flagged.
 *
 * If the `newLimit` is smaller than the current event count,
 * events will be removed in order of decreasing age.
 * This will also flag an icsneo::APIEvent::TooManyEvents.
 */
extern void DLLExport icsneo_setEventLimit(size_t newLimit);

/**
 * \brief Get the number of events which can be held in the API managed buffer
 * \returns The current limit.
 */
extern size_t DLLExport icsneo_getEventLimit(void);

/**
 * \brief Get the devices supported by the current version of the API
 * \param[out] devices A pointer to a buffer of devicetype_t structures which will be written to.
 * NULL can be passed, which will write the current supported device count to count.
 * \param[inout] count A pointer to a size_t which, prior to the call,
 * holds the maximum number of devicetype_t structures to be written,
 * and after the call holds the number of devicetype_t structures written.
 * \returns True if devices was written to
 *
 * See icsneo_getProductNameForType() to get textual descriptions of each device.
 *
 * A query for length (`devices == NULL`) will return false.
 *
 * If the count provided is not large enough, the output will be truncated.
 * An icsneo::APIEvent::OutputTruncatedError will be available in icsneo_getLastError() in this case.
 * True will still be returned.
 */
extern bool DLLExport icsneo_getSupportedDevices(devicetype_t* devices, size_t* count);

/**
 * \brief Get the timestamp resolution for the given device
 * \param[out] device A pointer to the neodevice_t structure specifying the device to read out the timestamp for.
 * \param[out] resolution A pointer to a uint16_t where the resolution will be stored. This value is in nanoseconds.

 * \returns True if the resolution was written
 */
extern bool DLLExport icsneo_getTimestampResolution(const neodevice_t* device, uint16_t* resolution);

/**
 * \brief Get the value of a digital IO for the given device
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] type The IO type
 * \param[in] number The index within the IO type, starting from 1
 * \param[out] value A pointer to the uint8_t which will store the value of the IO port, if successful
 * \returns True if the value is read successfully
 *
 * These values are often not populated if the device is not "online".
 */
extern bool DLLExport icsneo_getDigitalIO(const neodevice_t* device, neoio_t type, uint32_t number, bool* value);

/**
 * \brief Get the value of a digital IO for the given device
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] type The IO type
 * \param[in] number The index within the IO type, starting from 1
 * \param[in] value The value which will be written to the IO
 * \returns True if the parameters and connection state are correct to submit the request to the device
 *
 * Note that this function is not synchronous with the device confirming the change.
 */
extern bool DLLExport icsneo_setDigitalIO(const neodevice_t* device, neoio_t type, uint32_t number, bool value);

/**
 * \brief Check whether software switchable termination is supported for a given network on this device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] netid The network ID to check
 * \returns True if software switchable termination is supported
 *
 * This does not check whether another network in the termination
 * group has termination enabled, check canTerminationBeEnabledFor
 * for that.
 */
extern bool DLLExport icsneo_isTerminationSupportedFor(const neodevice_t* device, neonetid_t netid);

/**
 * \brief Check whether software switchable termination can currently be enabled for a given network.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] netid The network ID to check
 * \returns True if software switchable termination can currently be enabled
 * 
 * If another network in the group is already enabled, or if
 * termination is not supported on this network, false is
 * returned and an error will have been reported in
 * icsneo_getLastError().
 */
extern bool DLLExport icsneo_canTerminationBeEnabledFor(const neodevice_t* device, neonetid_t netid);

/**
 * \brief Check whether software switchable termination is currently
 * enabled for a given network in the currently active device settings.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] netid The network ID to check
 * \returns True if software switchable termination is currently enabled
 *
 * Note that if the termination status is set, but not yet
 * applied to the device, the current device status will be
 * reflected here rather than the pending status.
 * 
 * False will be returned and an error will be set in
 * icsneo_getLastError if the setting is unreadable.
 */
extern bool DLLExport icsneo_isTerminationEnabledFor(const neodevice_t* device, neonetid_t netid);

/**
 * \brief Enable or disable software switchable termination for a given network.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] netid The network ID to affect
 * \param[in] enabled Whether to enable or disable switchable termination
 * \returns True if if the call was successful, otherwise an error will have been reported in icsneo_getLastError().
 *
 * All other networks in the termination group must be disabled
 * prior to the call, but the change does not need to be applied
 * to the device before enqueing the enable.
 */
extern bool DLLExport icsneo_setTerminationFor(const neodevice_t* device, neonetid_t netid, bool enabled);

/**
 * \brief Return the device status structures for a specified device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.
 * \param[in] status A pointer to a device status structure for the current device.
 * \param[in] size The size of the current device status structure in bytes.
 * \returns True if the device status was successfully read.
 *
 * This function populates the device status structures and sub members.
 */
extern int DLLExport icsneo_getDeviceStatus(const neodevice_t* device, void* status, size_t* size);

/**
 * \brief Get the real-time clock for the given device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to read the RTC from.
 * \param[out] output A pointer to a uint64_t where the RTC will be stored. This value is in seconds.

 * \returns True if the RTC was successfully retrieved.
 */
extern bool DLLExport icsneo_getRTC(const neodevice_t* device, uint64_t* output);

/**
 * \brief Set the real-time clock for the given device.
 * \param[in] device A pointer to the neodevice_t structure specifying the device to write the RTC to.
 * \param[in] input A uint64_t object holding the RTC value. This value is in seconds.

 * \returns True if the RTC was successfully set.
 */
extern bool DLLExport icsneo_setRTC(const neodevice_t* device, uint64_t input);

/**
 * \brief Check if the device supports the ability to go online
 * \param[in] device A pointer to the neodevice_t structure specifying the device to operate on.

 * \returns True if the device supports the ability to go online
*/
extern bool DLLExport icsneo_isOnlineSupported(const neodevice_t* device);

#ifdef __cplusplus
} // extern "C"
#endif

#else // ICSNEOC_DYNAMICLOAD

typedef void(*fn_icsneo_findAllDevices)(neodevice_t* devices, size_t* count);
fn_icsneo_findAllDevices icsneo_findAllDevices;

typedef void(*fn_icsneo_freeUnconnectedDevices)();
fn_icsneo_freeUnconnectedDevices icsneo_freeUnconnectedDevices;

typedef bool(*fn_icsneo_serialNumToString)(uint32_t num, char* str, size_t* count);
fn_icsneo_serialNumToString icsneo_serialNumToString;

typedef uint32_t(*fn_icsneo_serialStringToNum)(const char* str);
fn_icsneo_serialStringToNum icsneo_serialStringToNum;

typedef bool(*fn_icsneo_isValidNeoDevice)(const neodevice_t* device);
fn_icsneo_isValidNeoDevice icsneo_isValidNeoDevice;

typedef bool(*fn_icsneo_openDevice)(const neodevice_t* device);
fn_icsneo_openDevice icsneo_openDevice;

typedef bool(*fn_icsneo_closeDevice)(const neodevice_t* device);
fn_icsneo_closeDevice icsneo_closeDevice;

typedef bool(*fn_icsneo_isOpen)(const neodevice_t* device);
fn_icsneo_isOpen icsneo_isOpen;

typedef bool(*fn_icsneo_goOnline)(const neodevice_t* device);
fn_icsneo_goOnline icsneo_goOnline;

typedef bool(*fn_icsneo_goOffline)(const neodevice_t* device);
fn_icsneo_goOffline icsneo_goOffline;

typedef bool(*fn_icsneo_isOnline)(const neodevice_t* device);
fn_icsneo_isOnline icsneo_isOnline;

typedef bool(*fn_icsneo_enableMessagePolling)(const neodevice_t* device);
fn_icsneo_enableMessagePolling icsneo_enableMessagePolling;

typedef bool(*fn_icsneo_disableMessagePolling)(const neodevice_t* device);
fn_icsneo_disableMessagePolling icsneo_disableMessagePolling;

typedef bool(*fn_icsneo_isMessagePollingEnabled)(const neodevice_t* device);
fn_icsneo_isMessagePollingEnabled icsneo_isMessagePollingEnabled;

typedef bool(*fn_icsneo_getMessages)(const neodevice_t* device, neomessage_t* messages, size_t* items, uint64_t timeout);
fn_icsneo_getMessages icsneo_getMessages;

typedef size_t(*fn_icsneo_getPollingMessageLimit)(const neodevice_t* device);
fn_icsneo_getPollingMessageLimit icsneo_getPollingMessageLimit;

typedef bool(*fn_icsneo_setPollingMessageLimit)(const neodevice_t* device, size_t newLimit);
fn_icsneo_setPollingMessageLimit icsneo_setPollingMessageLimit;

typedef int(*fn_icsneo_addMessageCallback)(const neodevice_t* device, void (*callback)(neomessage_t), void*);
fn_icsneo_addMessageCallback icsneo_addMessageCallback;

typedef bool(*fn_icsneo_removeMessageCallback)(const neodevice_t* device, int id);
fn_icsneo_removeMessageCallback icsneo_removeMessageCallback;

typedef neonetid_t (*fn_icsneo_getNetworkByNumber)(const neodevice_t* device, neonettype_t type, unsigned int number);
fn_icsneo_getNetworkByNumber icsneo_getNetworkByNumber;

typedef bool(*fn_icsneo_getProductName)(const neodevice_t* device, char* str, size_t* maxLength);
fn_icsneo_getProductName icsneo_getProductName;

typedef bool(*fn_icsneo_settingsRefresh)(const neodevice_t* device);
fn_icsneo_settingsRefresh icsneo_settingsRefresh;

typedef bool(*fn_icsneo_settingsApply)(const neodevice_t* device);
fn_icsneo_settingsApply icsneo_settingsApply;

typedef bool(*fn_icsneo_settingsApplyTemporary)(const neodevice_t* device);
fn_icsneo_settingsApplyTemporary icsneo_settingsApplyTemporary;

typedef bool(*fn_icsneo_settingsApplyDefaults)(const neodevice_t* device);
fn_icsneo_settingsApplyDefaults icsneo_settingsApplyDefaults;

typedef bool(*fn_icsneo_settingsApplyDefaultsTemporary)(const neodevice_t* device);
fn_icsneo_settingsApplyDefaultsTemporary icsneo_settingsApplyDefaultsTemporary;

typedef int(*fn_icsneo_settingsReadStructure)(const neodevice_t* device, void* structure, size_t structureSize);
fn_icsneo_settingsReadStructure icsneo_settingsReadStructure;

typedef bool(*fn_icsneo_settingsApplyStructure)(const neodevice_t* device, const void* structure, size_t structureSize);
fn_icsneo_settingsApplyStructure icsneo_settingsApplyStructure;

typedef bool(*fn_icsneo_settingsApplyStructureTemporary)(const neodevice_t* device, const void* structure, size_t structureSize);
fn_icsneo_settingsApplyStructureTemporary icsneo_settingsApplyStructureTemporary;

typedef int64_t(*fn_icsneo_getBaudrate)(const neodevice_t* device, neonetid_t netid);
fn_icsneo_getBaudrate icsneo_getBaudrate;

typedef bool(*fn_icsneo_setBaudrate)(const neodevice_t* device, neonetid_t netid, int64_t newBaudrate);
fn_icsneo_setBaudrate icsneo_setBaudrate;

typedef int64_t(*fn_icsneo_getFDBaudrate)(const neodevice_t* device, neonetid_t netid);
fn_icsneo_getFDBaudrate icsneo_getFDBaudrate;

typedef bool(*fn_icsneo_setFDBaudrate)(const neodevice_t* device, neonetid_t netid, int64_t newBaudrate);
fn_icsneo_setFDBaudrate icsneo_setFDBaudrate;

typedef bool(*fn_icsneo_transmit)(const neodevice_t* device, const neomessage_t* message);
fn_icsneo_transmit icsneo_transmit;

typedef bool(*fn_icsneo_transmitMessages)(const neodevice_t* device, const neomessage_t* messages, size_t count);
fn_icsneo_transmitMessages icsneo_transmitMessages;

typedef bool(*fn_icsneo_setWriteBlocks)(const neodevice_t* device, bool blocks);
fn_icsneo_setWriteBlocks icsneo_setWriteBlocks;

typedef bool(*fn_icsneo_describeDevice)(const neodevice_t* device, char* str, size_t* maxLength);
fn_icsneo_describeDevice icsneo_describeDevice;

typedef neoversion_t(*fn_icsneo_getVersion)(void);
fn_icsneo_getVersion icsneo_getVersion;

typedef int(*fn_icsneo_addEventCallback)(void (*callback)(neoevent_t), void*);
fn_icsneo_addEventCallback icsneo_addEventCallback;

typedef bool(*fn_icsneo_removeEventCallback)(int id);
fn_icsneo_removeEventCallback icsneo_removeEventCallback;

typedef bool(*fn_icsneo_getEvents)(neoevent_t* events, size_t* size);
fn_icsneo_getEvents icsneo_getEvents;

typedef bool(*fn_icsneo_getDeviceEvents)(const neodevice_t* device, neoevent_t* events, size_t* size);
fn_icsneo_getDeviceEvents icsneo_getDeviceEvents;

typedef bool(*fn_icsneo_getLastError)(neoevent_t* error);
fn_icsneo_getLastError icsneo_getLastError;

typedef void(*fn_icsneo_discardAllEvents)(void);
fn_icsneo_discardAllEvents icsneo_discardAllEvents;

typedef void(*fn_icsneo_discardDeviceEvents)(const neodevice_t* device);
fn_icsneo_discardDeviceEvents icsneo_discardDeviceEvents;

typedef void(*fn_icsneo_setEventLimit)(size_t newLimit);
fn_icsneo_setEventLimit icsneo_setEventLimit;

typedef size_t(*fn_icsneo_getEventLimit)(void);
fn_icsneo_getEventLimit icsneo_getEventLimit;

typedef bool(*fn_icsneo_getSupportedDevices)(devicetype_t* devices, size_t* count);
fn_icsneo_getSupportedDevices icsneo_getSupportedDevices;

typedef bool(*fn_icsneo_getTimestampResolution)(const neodevice_t* device, uint16_t* resolution);
fn_icsneo_getTimestampResolution icsneo_getTimestampResolution;

typedef bool(*fn_icsneo_getDigitalIO)(const neodevice_t* device, neoio_t type, uint32_t number, bool* value);
fn_icsneo_getDigitalIO icsneo_getDigitalIO;

typedef bool(*fn_icsneo_setDigitalIO)(const neodevice_t* device, neoio_t type, uint32_t number, bool value);
fn_icsneo_setDigitalIO icsneo_setDigitalIO;

typedef bool(*fn_icsneo_isTerminationSupportedFor)(const neodevice_t* device, neonetid_t netid);
fn_icsneo_isTerminationSupportedFor icsneo_isTerminationSupportedFor;

typedef bool(*fn_icsneo_canTerminationBeEnabledFor)(const neodevice_t* device, neonetid_t netid);
fn_icsneo_canTerminationBeEnabledFor icsneo_canTerminationBeEnabledFor;

typedef bool(*fn_icsneo_isTerminationEnabledFor)(const neodevice_t* device, neonetid_t netid);
fn_icsneo_isTerminationEnabledFor icsneo_isTerminationEnabledFor;

typedef bool(*fn_icsneo_setTerminationFor)(const neodevice_t* device, neonetid_t netid, bool enabled);
fn_icsneo_setTerminationFor icsneo_setTerminationFor;

typedef int (*fn_icsneo_getDeviceStatus)(const neodevice_t* device, void* status, size_t* size);
fn_icsneo_getDeviceStatus icsneo_getDeviceStatus;

typedef bool (*fn_icsneo_getRTC)(const neodevice_t* device, uint64_t* output);
fn_icsneo_getRTC icsneo_getRTC;

typedef bool (*fn_icsneo_setRTC)(const neodevice_t* device, uint64_t input);
fn_icsneo_setRTC icsneo_setRTC;

typedef bool(*fn_icsneo_isOnlineSupported)(const neodevice_t* device);
fn_icsneo_isOnlineSupported icsneo_isOnlineSupported;

#define ICSNEO_IMPORT(func) func = (fn_##func)icsneo_dynamicLibraryGetFunction(icsneo_libraryHandle, #func)
#define ICSNEO_IMPORTASSERT(func) if((ICSNEO_IMPORT(func)) == NULL) return 3
void* icsneo_libraryHandle = NULL;
bool icsneo_initialized = false;
bool icsneo_destroyed = false;
int icsneo_init() {
	icsneo_destroyed = false;
	if(icsneo_initialized)
		return 1;

	icsneo_libraryHandle = icsneo_dynamicLibraryLoad();
	if(icsneo_libraryHandle == NULL)
		return 2;

	ICSNEO_IMPORTASSERT(icsneo_findAllDevices);
	ICSNEO_IMPORTASSERT(icsneo_freeUnconnectedDevices);
	ICSNEO_IMPORTASSERT(icsneo_serialNumToString);
	ICSNEO_IMPORTASSERT(icsneo_serialStringToNum);
	ICSNEO_IMPORTASSERT(icsneo_isValidNeoDevice);
	ICSNEO_IMPORTASSERT(icsneo_openDevice);
	ICSNEO_IMPORTASSERT(icsneo_closeDevice);
	ICSNEO_IMPORTASSERT(icsneo_isOpen);
	ICSNEO_IMPORTASSERT(icsneo_goOnline);
	ICSNEO_IMPORTASSERT(icsneo_goOffline);
	ICSNEO_IMPORTASSERT(icsneo_isOnline);
	ICSNEO_IMPORTASSERT(icsneo_enableMessagePolling);
	ICSNEO_IMPORTASSERT(icsneo_disableMessagePolling);
	ICSNEO_IMPORTASSERT(icsneo_isMessagePollingEnabled);
	ICSNEO_IMPORTASSERT(icsneo_getMessages);
	ICSNEO_IMPORTASSERT(icsneo_getPollingMessageLimit);
	ICSNEO_IMPORTASSERT(icsneo_setPollingMessageLimit);
	ICSNEO_IMPORTASSERT(icsneo_addMessageCallback);
	ICSNEO_IMPORTASSERT(icsneo_removeMessageCallback);
	ICSNEO_IMPORTASSERT(icsneo_getNetworkByNumber);
	ICSNEO_IMPORTASSERT(icsneo_getProductName);
	ICSNEO_IMPORTASSERT(icsneo_settingsRefresh);
	ICSNEO_IMPORTASSERT(icsneo_settingsApply);
	ICSNEO_IMPORTASSERT(icsneo_settingsApplyTemporary);
	ICSNEO_IMPORTASSERT(icsneo_settingsApplyDefaults);
	ICSNEO_IMPORTASSERT(icsneo_settingsApplyDefaultsTemporary);
	ICSNEO_IMPORTASSERT(icsneo_settingsReadStructure);
	ICSNEO_IMPORTASSERT(icsneo_settingsApplyStructure);
	ICSNEO_IMPORTASSERT(icsneo_settingsApplyStructureTemporary);
	ICSNEO_IMPORTASSERT(icsneo_getBaudrate);
	ICSNEO_IMPORTASSERT(icsneo_setBaudrate);
	ICSNEO_IMPORTASSERT(icsneo_getFDBaudrate);
	ICSNEO_IMPORTASSERT(icsneo_setFDBaudrate);
	ICSNEO_IMPORTASSERT(icsneo_transmit);
	ICSNEO_IMPORTASSERT(icsneo_transmitMessages);
	ICSNEO_IMPORTASSERT(icsneo_setWriteBlocks);
	ICSNEO_IMPORTASSERT(icsneo_describeDevice);
	ICSNEO_IMPORTASSERT(icsneo_getVersion);
	ICSNEO_IMPORTASSERT(icsneo_addEventCallback);
	ICSNEO_IMPORTASSERT(icsneo_removeEventCallback);
	ICSNEO_IMPORTASSERT(icsneo_getEvents);
	ICSNEO_IMPORTASSERT(icsneo_getDeviceEvents);
	ICSNEO_IMPORTASSERT(icsneo_getLastError);
	ICSNEO_IMPORTASSERT(icsneo_discardAllEvents);
	ICSNEO_IMPORTASSERT(icsneo_discardDeviceEvents);
	ICSNEO_IMPORTASSERT(icsneo_setEventLimit);
	ICSNEO_IMPORTASSERT(icsneo_getEventLimit);
	ICSNEO_IMPORTASSERT(icsneo_getSupportedDevices);
	ICSNEO_IMPORTASSERT(icsneo_getTimestampResolution);
	ICSNEO_IMPORTASSERT(icsneo_getDigitalIO);
	ICSNEO_IMPORTASSERT(icsneo_setDigitalIO);
	ICSNEO_IMPORTASSERT(icsneo_isTerminationSupportedFor);
	ICSNEO_IMPORTASSERT(icsneo_canTerminationBeEnabledFor);
	ICSNEO_IMPORTASSERT(icsneo_isTerminationEnabledFor);
	ICSNEO_IMPORTASSERT(icsneo_setTerminationFor);
	ICSNEO_IMPORTASSERT(icsneo_getDeviceStatus);
	ICSNEO_IMPORTASSERT(icsneo_getRTC);
	ICSNEO_IMPORTASSERT(icsneo_setRTC);
	ICSNEO_IMPORTASSERT(icsneo_isOnlineSupported);

	icsneo_initialized = true;
	return 0;
}

bool icsneo_close() ICSNEO_DESTRUCTOR {
	icsneo_initialized = false;
	if(icsneo_destroyed)
		return true;

	return icsneo_destroyed = icsneo_dynamicLibraryClose(icsneo_libraryHandle);
}

#endif // ICSNEOC_DYNAMICLOAD

#endif // __ICSNEOC_H_