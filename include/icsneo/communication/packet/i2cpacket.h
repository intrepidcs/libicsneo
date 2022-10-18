#ifndef __I2CPACKET_H__
#define __I2CPACKET_H__

#ifdef __cplusplus

#include "icsneo/communication/message/i2cmessage.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/network.h"
#include <cstdint>
#include <memory>

namespace icsneo {

static constexpr size_t I2CMaxLength = 1024u;

#pragma pack(push, 2)

struct I2CHeader {
	struct {
		// C1xI2C
		uint16_t ID : 10;// i2c address, 7-bit or 10-bit
		uint16_t EID : 1;// using extended addressing, i.e. 10-bit
		uint16_t CT : 1;// Controller/Target
		uint16_t DIR : 1;// read/write
		uint16_t RESERVED_0 : 3;

		// C2xI2C
		uint16_t TXMsg: 1;
		uint16_t CBLen: 11;
		uint16_t RESERVED_1: 4;

		// C3xI2C
		uint16_t TXTimeout : 1;
		uint16_t TXNack : 1;
		uint16_t TXAborted : 1;
		uint16_t TXLostArb : 1;
		uint16_t TXError : 1;
		uint16_t RESERVED_2: 11;
	} CoreMiniBitsI2C;

	uint8_t coreMiniMessageData[8];
	uint16_t stats;

	uint64_t timestamp;

	uint16_t networkID;
	uint16_t length;
};

#pragma pack(pop)

struct HardwareI2CPacket {
	static std::shared_ptr<Message> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static bool EncodeFromMessage(const I2CMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);
};
}

#endif //_cplusplus

#endif