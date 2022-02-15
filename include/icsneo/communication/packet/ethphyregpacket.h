#ifndef __ETHPHYREGPACKET_H__
#define __ETHPHYREGPACKET_H__

#ifdef __cplusplus

#include "icsneo/api/eventmanager.h"
#include <cstdint>
#include <memory>

namespace icsneo
{

class Packetizer;
class EthPhyMessage;

#pragma pack(push, 1)
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201) // nameless struct/union
#endif

struct PhyRegisterHeader_t {
	uint16_t numEntries;
	uint8_t version;
	uint8_t entryBytes;
};

struct Clause22Message {
	uint8_t phyAddr;  //5 bits
	uint8_t page;	  //8 bits
	uint16_t regAddr; //5 bits
	uint16_t regVal;
}; //6 bytes

struct Clause45Message {
	uint8_t port;	//5 bits
	uint8_t device; //5 bits
	uint16_t regAddr;
	uint16_t regVal;
}; //6 bytes

struct PhyRegisterPacket_t {
	union {
		struct {
			uint16_t Enabled : 1;
			uint16_t WriteEnable : 1;
			uint16_t Clause45Enable : 1;
			uint16_t reserved : 9;
			uint16_t version : 4;
		};
		uint16_t flags;
	};

	union {
		Clause22Message clause22;
		Clause45Message clause45;
	};
};

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#pragma pack(pop)

static constexpr size_t MaxPhyEntries = 128u;
static constexpr size_t MaxBytesPhyEntries = MaxPhyEntries * sizeof(PhyRegisterHeader_t);
static constexpr uint8_t PhyPacketVersion = 1u;
static constexpr uint8_t FiveBits = 0x1Fu;

class EthPhyMessage;

struct HardwareEthernetPhyRegisterPacket {
	static std::shared_ptr<EthPhyMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);
	static bool EncodeFromMessage(const EthPhyMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);
};

}

#endif //__cplusplus

#endif