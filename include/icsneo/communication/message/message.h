#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <stdint.h>
typedef uint16_t neomessagetype_t;

#ifdef __cplusplus

#include "icsneo/communication/network.h"
#include "icsneo/icsneoc2types.h"
#include <vector>
#include <sstream>
#include <string>

namespace icsneo {

/** 
 * @brief Type of message class
 * 
 *  @see AbstractMessage::getMsgType()
 */
typedef enum class MessageType : icsneoc2_msg_type_t {
	Device = icsneoc2_msg_type_device,
	Internal = icsneoc2_msg_type_internal,
	Bus = icsneoc2_msg_type_bus,

	MaxSize = icsneoc2_msg_type_maxsize,
} MessageType;

/** 
 * @brief Pure virtual abstract class for all messages. Inherit from Message over this class.
 * 
 * @see Message
 * 
 */
class AbstractMessage {
public:
	virtual const MessageType getMsgType() const = 0;
};

/**
 * @brief Base Message class representing Device messages. 
 */
class Message : public AbstractMessage {
public:
	virtual const MessageType getMsgType() const { return MessageType::Device; }

	/**
	 * @brief Get the string representation of the message type
	 * 
	 * @return String representation of the message type
	 * 
	 * @see AbstractMessage::getMsgType()
	 */
	static std::string getMsgTypeName(MessageType msgType) {
		switch (msgType) {
			case MessageType::Device:
				return "Device";
			case MessageType::Internal:
				return "Internal";
			case MessageType::Bus:
				return "Bus";
			// Don't default here so we can rely on the compiler to warn us about missing cases
		};
		std::stringstream ss;
		ss << "Unknown (" << static_cast<int>(msgType) << ")";
		return ss.str();
	}

	enum class Type : neomessagetype_t {
		BusMessage = 0,
		// Deprecated: Will be removed in the future.
		Frame = BusMessage,

		CANErrorCount = 0x100,
		CANError = 0x100,

		LINHeaderOnly = 0x200,
		LINBreak = 0x201,

		// Past 0x8000 are all for internal use only
		Invalid = 0x8000,
		InternalMessage = 0x8001,
		// Deprecated: Will be removed in the future.
		RawMessage = InternalMessage,
		ReadSettings = 0x8002,
		ResetStatus = 0x8003,
		DeviceVersion = 0x8004,
		Main51 = 0x8005,
		FlexRayControl = 0x8006,
		EthernetPhyRegister = 0x8007,
		LogicalDiskInfo = 0x8008,
		ExtendedResponse = 0x8009,
		WiVICommandResponse = 0x800a,
		ScriptStatus = 0x800b,
		ComponentVersions = 0x800c,
		SupportedFeatures = 0x800d,
		GenericBinaryStatus = 0x800e,
		LiveData = 0x800f,
		HardwareInfo = 0x8010,
		TC10Status = 0x8011,
		AppError = 0x8012,
		GPTPStatus = 0x8013,
		EthernetStatus = 0x8014,
	};

	Message(Type t) : type(t) {}
	virtual ~Message() = default;
	const Type type;
	uint64_t timestamp = 0;
};

/**
 * @brief Internal Message class representing Device messages that shouldn't be exposed to public APIs.
 */
class InternalMessage : public Message {
public:
	InternalMessage(Message::Type type = Message::Type::InternalMessage) : Message(type) {}
	InternalMessage(Message::Type type, Network net) : Message(type), network(net) {}
	InternalMessage(Network net) : Message(Message::Type::InternalMessage), network(net) {}
	InternalMessage(Network net, std::vector<uint8_t> d) : Message(Message::Type::InternalMessage), network(net), data(d) {}

	virtual const MessageType getMsgType() const { return MessageType::Internal; }

	Network network;
	std::vector<uint8_t> data;
};

/**
 * @brief Bus Message class representing Device messages representing Bus networks like CAN, LIN, Ethernet, etc.
 */
class BusMessage : public InternalMessage {
public:
	BusMessage() : InternalMessage(Message::Type::BusMessage) {}

	/** @brief
	 * Bus message types, useful for filtering out or identifying Bus Messages.
	*/
	typedef enum class Type : icsneoc2_msg_bus_type_t {
		Invalid = icsneoc2_msg_bus_type_invalid,
		Internal = icsneoc2_msg_bus_type_internal,
		CAN = icsneoc2_msg_bus_type_can,
		LIN = icsneoc2_msg_bus_type_lin,
		FlexRay = icsneoc2_msg_bus_type_flexray,
		MOST = icsneoc2_msg_bus_type_most,
		Ethernet = icsneoc2_msg_bus_type_ethernet,
		LSFTCAN = icsneoc2_msg_bus_type_lsftcan,
		SWCAN = icsneoc2_msg_bus_type_swcan,
		ISO9141 = icsneoc2_msg_bus_type_iso9141,
		I2C = icsneoc2_msg_bus_type_i2c,
		A2B = icsneoc2_msg_bus_type_a2b,
		SPI = icsneoc2_msg_bus_type_spi,
		MDIO = icsneoc2_msg_bus_type_mdio,
		ANY = icsneoc2_msg_bus_type_any,
		OTHER = icsneoc2_msg_bus_type_other
	} Type;

	const MessageType getMsgType() const final { return MessageType::Bus; }

	virtual const BusMessage::Type getBusType() const = 0;

	// Description ID of the message. This is used for filtering / tracking in firmware and driver.
	// This is equivalent to icsSpyMessage::DescriptionID
	uint16_t description = 0;
	// weather the message was originally transmitted on the bus. This is equivalent to 
	// SPY_STATUS_TX_MSG bit field in icsSpyMessage::StatusBitField
	bool transmitted = false;
	bool error = false;
};

/** @brief Backwards compatibility, RawMessage was renamed to better reflect what it actually does. */
typedef InternalMessage RawMessage;
/** @brief Backwards compatibility, Frame was renamed to better reflect what it actually does. */
typedef BusMessage Frame;

}

#endif // __cplusplus

#ifdef __ICSNEOC_H_

#define ICSNEO_MESSAGE_TYPE_FRAME (0x0)
#define ICSNEO_MESSAGE_TYPE_CAN_ERROR_COUNT (0x100)
#define ICSNEO_MESSAGE_TYPE_INVALID (0x8000)
#define ICSNEO_MESSAGE_TYPE_RAW_MESSAGE (0x8001)
#define ICSNEO_MESSAGE_TYPE_READ_SETTINGS (0x8002)
#define ICSNEO_MESSAGE_TYPE_RESET_STATUS (0x8003)
#define ICSNEO_MESSAGE_TYPE_DEVICE_VERSION (0x8004)
#define ICSNEO_MESSAGE_TYPE_MAIN51 (0x8005)
#define ICSNEO_MESSAGE_TYPE_FLEXRAY_CONTROL (0x8006)
#define ICSNEO_MESSAGE_TYPE_ETHERNET_PHY_REGISTER (0x8007)
#define ICSNEO_MESSAGE_TYPE_LOGICAL_DISK_INFO (0x8008)
#define ICSNEO_MESSAGE_TYPE_EXTENDED_RESPONSE (0x8009)

#endif // __ICSNEOC_H_

#endif