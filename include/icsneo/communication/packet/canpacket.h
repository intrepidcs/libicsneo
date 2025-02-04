#ifndef __CANPACKET_H__
#define __CANPACKET_H__

#ifdef __cplusplus

#include "icsneo/communication/message/canmessage.h"
#include "icsneo/api/eventmanager.h"
#include <cstdint>
#include <memory>
#include <optional>

namespace icsneo {

typedef uint16_t icscm_bitfield;

#pragma pack(push,2)
struct HardwareCANPacket {
	static std::shared_ptr<Message> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static bool EncodeFromMessage(const CANMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);

	struct {
		icscm_bitfield IDE : 1;
		icscm_bitfield SRR : 1;
		icscm_bitfield SID : 11;
		icscm_bitfield EDL : 1;
		icscm_bitfield BRS : 1;
		icscm_bitfield ESI : 1;
	} header;
	struct {
		icscm_bitfield EID : 12;
		icscm_bitfield TXMSG : 1;
		icscm_bitfield TXAborted : 1;
		icscm_bitfield TXLostArb : 1;
		icscm_bitfield TXError : 1;
	} eid;
	struct {
		icscm_bitfield DLC : 4;
		icscm_bitfield RB0 : 1;
		icscm_bitfield IVRIF : 1;
		icscm_bitfield HVEnable : 1;// must be cleared before passing into CAN driver
		icscm_bitfield ExtendedNetworkIndexBit : 1;//DO NOT CLOBBER THIS
		icscm_bitfield RB1 : 1;
		icscm_bitfield RTR : 1;
		icscm_bitfield EID2 : 6;
	} dlc;
	unsigned char data[8];
	uint16_t stats;
	struct {
		uint64_t TS : 60;
		uint64_t : 3; // Reserved for future status bits
		uint64_t IsExtended : 1;
	} timestamp;
};
struct HardwareCANErrorPacket {
	uint8_t error_code;
	uint8_t brs_data_error_code;

	uint16_t reserved;

	uint16_t DLC : 4;
	uint16_t : 4;
	uint16_t ERROR_INDICATOR : 1;
	uint16_t : 7;

	uint8_t flags;
	uint8_t REC;
	uint8_t TEC;

	static bool GetErrorWarn(uint8_t flags) { return flags & 0b0000'0001; }
	static bool GetErrorPassive(uint8_t flags) { return flags & 0b0000'1000; }
	static bool GetBusOff(uint8_t flags) { return flags & 0b0010'0000; }
};


#pragma pack(pop)

}

#endif // __cplusplus

#endif