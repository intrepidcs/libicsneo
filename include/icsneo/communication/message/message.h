#ifndef __MESSAGE_H_
#define __MESSAGE_H_

#include <stdint.h>
typedef uint16_t neomessagetype_t;

#ifdef __cplusplus

#include "icsneo/communication/network.h"
#include "icsneo/icsneotypes.h"
#include <vector>

namespace icsneo {

class AbstractMessage {
public:
	virtual const icsneo_msg_type_t getMsgType() const = 0;
};

class Message : public AbstractMessage {
public:
	virtual const icsneo_msg_type_t getMsgType() const { return icsneo_msg_type_device; }

	enum class Type : neomessagetype_t {
		Frame = 0,

		CANErrorCount = 0x100,

		LINHeaderOnly = 0x200,
		LINBreak = 0x201,

		// Past 0x8000 are all for internal use only
		Invalid = 0x8000,
		RawMessage = 0x8001,
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
	};

	Message(Type t) : type(t) {}
	virtual ~Message() = default;
	const Type type;
	uint64_t timestamp = 0;
};

class InternalMessage : public Message {
public:
	InternalMessage(Message::Type type = Message::Type::RawMessage) : Message(type) {}
	InternalMessage(Message::Type type, Network net) : Message(type), network(net) {}
	InternalMessage(Network net) : Message(Message::Type::RawMessage), network(net) {}
	InternalMessage(Network net, std::vector<uint8_t> d) : Message(Message::Type::RawMessage), network(net), data(d) {}

	virtual const icsneo_msg_type_t getMsgType() const { return icsneo_msg_type_internal; }

	Network network;
	std::vector<uint8_t> data;
};

class BusMessage : public InternalMessage {
public:
	BusMessage() : InternalMessage(Message::Type::Frame) {}

	const icsneo_msg_type_t getMsgType() const final { return icsneo_msg_type_bus; }

	virtual const icsneo_msg_bus_type_t getBusType() const = 0;

	uint16_t description = 0;
	bool transmitted = false;
	bool error = false;
};

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