#ifndef __SPIPACKET_H__
#define __SPIPACKET_H__

#ifdef __cplusplus

#include "icsneo/api/eventmanager.h"
#include <cstdint>
#include <memory>
#include <optional>
#include "icsneo/communication/message/spimessage.h"

namespace icsneo {

typedef uint16_t icscm_bitfield;

#pragma pack(push, 2)
struct HardwareSPIPacket {

	static std::shared_ptr<Message> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static bool EncodeFromMessage(const SPIMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);

	struct {
		uint16_t frameLength;
	} header;
	uint8_t offset[12];
	uint16_t stats;
	struct {
		uint64_t TS : 60;
		uint64_t : 3; // Reserved for future status bits
		uint64_t IsExtended : 1;
	} timestamp;
	uint16_t networkID;
	uint16_t length;
};

#pragma pack(pop)

}

#endif // __cplusplus

#endif