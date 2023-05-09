#ifndef __MDIOPACKET_H__
#define __MDIOPACKET_H__

#ifdef __cplusplus

#include "icsneo/communication/message/mdiomessage.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/network.h"
#include <cstdint>
#include <memory>

namespace icsneo {

#pragma pack(push, 2)

struct HardwareMDIOPacket {

	static std::shared_ptr<Message> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static bool EncodeFromMessage(const MDIOMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);

	struct {
		// C1xMDIO
		uint16_t PHY_ADDR : 5; // mdio phy address, 5-bit
		uint16_t ST : 2; // Start of Frame (Clause 22/45)
		uint16_t OP : 2; // OP Code (Read/Write)
		uint16_t ADVANCED_MODE : 1; // future?
		uint16_t TRANSMIT : 1; // This is a TX message
		uint16_t C45_DEVTYPE : 5;

		// C2xMDIO
		uint16_t ERR_JOB_CANCELLED : 1; // error flag
		uint16_t ERR_TIMEOUT : 1; // error flag
		uint16_t ERR_INVALID_BUS : 1; // error flag
		uint16_t ERR_INVALID_PHYADDR : 1; // error flag
		uint16_t ERR_INVALID_REGADDR : 1; // error flag
		uint16_t ERR_UNSUPPORTED_CLAUSE : 1; // error flag
		uint16_t ERR_UNSUPPORTED_OPCODE : 1; // error flag
		uint16_t ERR_OVERFLOW : 1; // error flag
		uint16_t : 8;

		// C3xMDIO
		uint16_t REG_ADDR;
	} header;

	uint8_t data[8];
	uint16_t stats;
	uint64_t timestamp;
	uint16_t networkID;
	uint16_t length;
	
	static const size_t mdioDataSize;
};

#pragma pack(pop)

}

#endif // __cplusplus

#endif