#ifndef __CANPACKET_H__
#define __CANPACKET_H__

#include "icsneo/communication/message/canmessage.h"
#include "icsneo/api/errormanager.h"
#include <cstdint>
#include <memory>

namespace icsneo {

typedef uint16_t icscm_bitfield;

struct HardwareCANPacket {
	static std::shared_ptr<CANMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static bool EncodeFromMessage(const CANMessage& message, std::vector<uint8_t>& bytestream, const device_errorhandler_t& err);

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

}

#endif