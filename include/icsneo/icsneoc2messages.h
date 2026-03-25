#ifndef __ICSNEOC2MESSAGES_H_
#define __ICSNEOC2MESSAGES_H_

#include <icsneo/icsneoc2.h>
#include <icsneo/icsneoc2types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create CAN message
 *
 * @param[out] message Pointer to icsneoc2_message_t to copy the message into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_can_create(icsneoc2_message_t** message);

/**
 * Free message
 *
 * @param[in] message The message to free.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 *
 * @see icsneoc2_message_can_create icsneoc2_device_message_get
 */
icsneoc2_error_t icsneoc2_message_free(icsneoc2_message_t* message);

/**
 * Get the transmission status of a message.
 *
 * When a message is transmitted from the device, it will be returned in the receive buffer.
 * @see icsneoc2_device_messages_transmit
 *
 * @param[in] message The message to check.
 * @param[out] value Pointer to a bool to copy the transmission status into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_is_transmit(icsneoc2_message_t* message, bool* value);

/**
 * Get the Network ID (netid) of a bus message
 *
 * @param[in] message The message to check.
 * @param[out] netid Pointer to a icsneoc2_netid_t to copy the netid into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_invalid_type otherwise.
 *
 * @see icsneoc2_netid_t, icsneoc2_netid_name_get
 */
icsneoc2_error_t icsneoc2_message_netid_get(icsneoc2_message_t* message, icsneoc2_netid_t* netid);

/**
 * Get the netid string for a icsneoc2_netid_t.
 *
 * @param[in] netid The network id to get the description of.
 * @param[out] value Pointer to a buffer to copy the description into. Null terminated.
 * @param[in,out] value_length Size of the value buffer. Modified with the length of the description.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_netid_name_get(icsneoc2_netid_t netid, char* value, size_t* value_length);

/**
 * Set the Network ID (netid) of a bus message
 *
 * @param[in] device The device to check against.
 * @param[in] message The message to modify.
 * @param[in] netid The netid to set.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_netid_set(icsneoc2_message_t* message, icsneoc2_netid_t netid);

/**
 * Set the data bytes of a message
 *
 * @param[in] message The message to copy the data into.
 * @param[in] data Pointer to a uint8_t array to copy from.
 * @param[in] data_length length of the data.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_data_set(icsneoc2_message_t* message, uint8_t* data, size_t data_length);

/**
 * Get the data bytes of a message
 *
 * @param[in] message The message to check.
 * @param[out] data Pointer to a uint8_t to copy the data bytes into.
 * @param[in,out] data_length Pointer to a size_t to copy the length of the data into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_data_get(icsneoc2_message_t* message, uint8_t* data, size_t* data_length);

#define ICSNEOC2_MESSAGE_CAN_FLAGS_RTR 0x01 // Remote Transmission Request
#define ICSNEOC2_MESSAGE_CAN_FLAGS_IDE 0x02 // Identifier Extension
#define ICSNEOC2_MESSAGE_CAN_FLAGS_FDF 0x04 // FD Format Indicator
#define ICSNEOC2_MESSAGE_CAN_FLAGS_BRS 0x08 // Bit Rate Switch (FD only)
#define ICSNEOC2_MESSAGE_CAN_FLAGS_ESI 0x10 // Error State Indicator (FD only)

typedef uint64_t icsneoc2_message_can_flags_t;

/**
 * Set the CAN specific properties of a message
 *
 * @param[in] message The message to modify.
 * @param[in] arb_id Pointer to a uint64_t containing the arbitration ID to set. If NULL, it's ignored.
 * @param[in] flags Pointer to a icsneoc2_message_can_flags_t containing the flags to set. If NULL, it's ignored. See icsneoc2_message_can_flags_t for details.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_invalid_type otherwise.
 */
icsneoc2_error_t icsneoc2_message_can_props_set(icsneoc2_message_t* message, const uint64_t* arb_id, const icsneoc2_message_can_flags_t* flags);

/**
 * Get the CAN specific properties of a message
 *
 * @param[in] message The message to check.
 * @param[out] arb_id Pointer to a uint64_t to copy the arbitration ID into. If NULL, it's ignored.
 * @param[out] flags Pointer to a series of flags. If NULL, it's ignored. See icsneoc2_message_can_flags_t for details.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_invalid_type otherwise.
 */
icsneoc2_error_t icsneoc2_message_can_props_get(icsneoc2_message_t* message, uint64_t* arb_id, icsneoc2_message_can_flags_t* flags);

/**
 * Check if a message is valid
 *
 * @param[in] device The device to check against.
 * @param[in] message The message to check.
 * @param[out] is_valid Pointer to a bool to copy the validity of the message into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_is_valid(icsneoc2_message_t* message, bool* is_valid);

/**
 * Check if a message is a raw message (message with data)
 *
 * @param[in] device The device to check against.
 * @param[in] message The message to check.
 * @param[out] is_raw Pointer to a bool to copy the raw status of the message into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_is_raw(icsneoc2_message_t* message, bool* is_raw);

/**
 * Check if a message is a frame message (message with data)
 *
 * @param[in] device The device to check against.
 * @param[in] message The message to check.
 * @param[out] is_frame Pointer to a bool to copy the frame status of the message into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_is_frame(icsneoc2_message_t* message, bool* is_frame);

/**
 * Check if a message is a CAN message
 *
 * @param[in] device The device to check against.
 * @param[in] message The message to check.
 * @param[out] is_can Pointer to a bool to copy the CAN status of the message into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_is_can(icsneoc2_message_t* message, bool* is_can);

/**
 * Get the network type of a message
 *
 * @param[in] message The message to check.
 * @param[out] network_type Pointer to a icsneoc2_network_type_t to copy the network type into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_invalid_type otherwise.
 *
 * @see icsneoc2_network_type_t, icsneoc2_network_type_name_get
 */
icsneoc2_error_t icsneoc2_message_network_type_get(icsneoc2_message_t* message, icsneoc2_network_type_t* network_type);

#ifdef __cplusplus
}
#endif

#endif // __ICSNEOC2MESSAGES_H_
