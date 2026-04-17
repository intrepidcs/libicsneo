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
 * Get the frame error status of a message.
 *
 * @param[in] message The message to check.
 * @param[out] value Pointer to a bool to copy the frame error status into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or
 * icsneoc2_error_invalid_type otherwise.
 */
icsneoc2_error_t icsneoc2_message_is_error(icsneoc2_message_t* message, bool* value);

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

#define ICSNEOC2_MESSAGE_CAN_FLAGS_RTR 0x01         // Remote Transmission Request
#define ICSNEOC2_MESSAGE_CAN_FLAGS_IDE 0x02         // Identifier Extension
#define ICSNEOC2_MESSAGE_CAN_FLAGS_FDF 0x04         // FD Format Indicator
#define ICSNEOC2_MESSAGE_CAN_FLAGS_BRS 0x08         // Bit Rate Switch (FD only)
#define ICSNEOC2_MESSAGE_CAN_FLAGS_ESI 0x10         // Error State Indicator (FD only)
#define ICSNEOC2_MESSAGE_CAN_FLAGS_TX_ABORTED  0x20 // CAN transmit was aborted
#define ICSNEOC2_MESSAGE_CAN_FLAGS_TX_LOST_ARB 0x40 // CAN transmit lost arbitration
#define ICSNEOC2_MESSAGE_CAN_FLAGS_TX_ERROR    0x80 // CAN transmit reported an error

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
 *   TX status flags are read-only and are only reported for CAN frames received back from the device.
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
 * Check if a message is a LIN message
 *
 * @param[in] message The message to check.
 * @param[out] is_lin Pointer to a bool to copy the LIN status of the message into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_is_lin(icsneoc2_message_t* message, bool* is_lin);

// ---- LIN Message Types ----

typedef enum _icsneoc2_lin_msg_type_t {
	icsneoc2_lin_msg_type_not_set = 0,
	icsneoc2_lin_msg_type_commander_msg = 1,
	icsneoc2_lin_msg_type_header_only = 2,
	icsneoc2_lin_msg_type_break_only = 3,
	icsneoc2_lin_msg_type_sync_only = 4,
	icsneoc2_lin_msg_type_update_responder = 5,
	icsneoc2_lin_msg_type_error = 6,
} _icsneoc2_lin_msg_type_t;

typedef uint8_t icsneoc2_lin_msg_type_t;

// LIN error flags bitmask
#define ICSNEOC2_LIN_ERR_RX_BREAK_ONLY          0x0001
#define ICSNEOC2_LIN_ERR_RX_BREAK_SYNC_ONLY     0x0002
#define ICSNEOC2_LIN_ERR_TX_RX_MISMATCH         0x0004
#define ICSNEOC2_LIN_ERR_RX_BREAK_NOT_ZERO      0x0008
#define ICSNEOC2_LIN_ERR_RX_BREAK_TOO_SHORT     0x0010
#define ICSNEOC2_LIN_ERR_RX_SYNC_NOT_55         0x0020
#define ICSNEOC2_LIN_ERR_RX_DATA_LEN_OVER_8     0x0040
#define ICSNEOC2_LIN_ERR_FRAME_SYNC             0x0080
#define ICSNEOC2_LIN_ERR_FRAME_MESSAGE_ID       0x0100
#define ICSNEOC2_LIN_ERR_FRAME_RESPONDER_DATA   0x0200
#define ICSNEOC2_LIN_ERR_CHECKSUM_MATCH         0x0400

typedef uint32_t icsneoc2_lin_err_flags_t;

// LIN status flags bitmask
#define ICSNEOC2_LIN_STATUS_TX_CHECKSUM_ENHANCED      0x01
#define ICSNEOC2_LIN_STATUS_TX_COMMANDER              0x02
#define ICSNEOC2_LIN_STATUS_TX_RESPONDER              0x04
#define ICSNEOC2_LIN_STATUS_TX_ABORTED                0x08
#define ICSNEOC2_LIN_STATUS_UPDATE_RESPONDER_ONCE     0x10
#define ICSNEOC2_LIN_STATUS_HAS_UPDATED_RESPONDER_ONCE 0x20
#define ICSNEOC2_LIN_STATUS_BUS_RECOVERED             0x40
#define ICSNEOC2_LIN_STATUS_BREAK_ONLY                0x80

typedef uint32_t icsneoc2_lin_status_flags_t;

/**
 * Create a LIN message.
 *
 * @param[out] message Pointer to receive the new LIN message handle.
 * @param[in] id The LIN frame ID (0-63). Bits above 0x3F are masked off.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful.
 *
 * @see icsneoc2_message_free
 */
icsneoc2_error_t icsneoc2_message_lin_create(icsneoc2_message_t** message, uint8_t id);

/**
 * Get the LIN-specific properties of a message.
 *
 * Any output pointer may be NULL to skip that field.
 *
 * @param[in] message The message to query (must be a LIN message).
 * @param[out] id Pointer to receive the LIN frame ID.
 * @param[out] protected_id Pointer to receive the protected ID (ID with parity bits).
 * @param[out] checksum Pointer to receive the checksum byte.
 * @param[out] msg_type Pointer to receive the LIN message type.
 * @param[out] is_enhanced_checksum Pointer to receive whether enhanced checksum is used.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_type if not a LIN message.
 */
icsneoc2_error_t icsneoc2_message_lin_props_get(const icsneoc2_message_t* message,
	uint8_t* id, uint8_t* protected_id, uint8_t* checksum,
	icsneoc2_lin_msg_type_t* msg_type, bool* is_enhanced_checksum);

/**
 * Set the LIN-specific properties of a message.
 *
 * Any input pointer may be NULL to skip that field. Setting the ID also recalculates the protected ID.
 *
 * @param[in] message The message to modify (must be a LIN message).
 * @param[in] id Pointer to the LIN frame ID to set (0-63). NULL to skip.
 * @param[in] checksum Pointer to the checksum byte to set. NULL to skip.
 * @param[in] msg_type Pointer to the LIN message type to set. NULL to skip.
 * @param[in] is_enhanced_checksum Pointer to set enhanced checksum mode. NULL to skip.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_type if not a LIN message.
 */
icsneoc2_error_t icsneoc2_message_lin_props_set(icsneoc2_message_t* message,
	const uint8_t* id, const uint8_t* checksum,
	const icsneoc2_lin_msg_type_t* msg_type, const bool* is_enhanced_checksum);

/**
 * Get the LIN error flags of a message.
 *
 * @param[in] message The message to query (must be a LIN message).
 * @param[out] err_flags Pointer to receive the error flags bitmask.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_type if not a LIN message.
 *
 * @see ICSNEOC2_LIN_ERR_*
 */
icsneoc2_error_t icsneoc2_message_lin_err_flags_get(const icsneoc2_message_t* message, icsneoc2_lin_err_flags_t* err_flags);

/**
 * Get the LIN status flags of a message.
 *
 * @param[in] message The message to query (must be a LIN message).
 * @param[out] status_flags Pointer to receive the status flags bitmask.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_type if not a LIN message.
 *
 * @see ICSNEOC2_LIN_STATUS_*
 */
icsneoc2_error_t icsneoc2_message_lin_status_flags_get(const icsneoc2_message_t* message, icsneoc2_lin_status_flags_t* status_flags);

/**
 * Calculate and set the checksum on a LIN message.
 *
 * Uses enhanced or classic checksum based on the isEnhancedChecksum property.
 *
 * @param[in] message The LIN message to calculate the checksum for.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_type if not a LIN message.
 */
icsneoc2_error_t icsneoc2_message_lin_calc_checksum(icsneoc2_message_t* message);

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

/**
 * Check if a message is a CAN error message
 *
 * @param[in] message The message to check.
 * @param[out] is_can_error Pointer to a bool to copy the CAN error status of the message into.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters otherwise.
 */
icsneoc2_error_t icsneoc2_message_is_can_error(icsneoc2_message_t* message, bool* is_can_error);

/**
 * Get the CAN error specific properties of a message
 *
 * @param[in] message The message to check.
 * @param[out] tx_err_count Pointer to a uint8_t to copy the transmit error count into. If NULL, it's ignored.
 * @param[out] rx_err_count Pointer to a uint8_t to copy the receive error count into. If NULL, it's ignored.
 * @param[out] error_code Pointer to a icsneoc2_can_error_code_t to copy the error code into. If NULL, it's ignored.
 * @param[out] data_error_code Pointer to a icsneoc2_can_error_code_t to copy the data phase error code into. If NULL, it's ignored.
 * @param[out] flags Pointer to a icsneoc2_message_can_error_flags_t to copy the error flags into. If NULL, it's ignored.
 *   See ICSNEOC2_MESSAGE_CAN_ERROR_FLAGS_* for controller-error bits.
 *
 * @return icsneoc2_error_t icsneoc2_error_success if successful, icsneoc2_error_invalid_parameters or icsneoc2_error_invalid_type otherwise.
 */
icsneoc2_error_t icsneoc2_message_can_error_props_get(
    icsneoc2_message_t *message, uint8_t *tx_err_count, uint8_t *rx_err_count,
    icsneoc2_can_error_code_t *error_code,
    icsneoc2_can_error_code_t *data_error_code,
    icsneoc2_message_can_error_flags_t *flags);

#ifdef __cplusplus
}
#endif

#endif // __ICSNEOC2MESSAGES_H_
