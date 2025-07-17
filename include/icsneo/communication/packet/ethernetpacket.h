#ifndef __ETHERNETPACKET_H__
#define __ETHERNETPACKET_H__

#ifdef __cplusplus

#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/api/eventmanager.h"
#include <cstdint>
#include <memory>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace icsneo {

#pragma pack(push, 2)

typedef uint16_t icscm_bitfield;

struct HardwareEthernetPacket {
	static std::shared_ptr<EthernetMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);
	static bool EncodeFromMessage(const EthernetMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);

	// Word 0 - Header flags (offset 0)
	struct {
		icscm_bitfield FCS_AVAIL : 1;
		icscm_bitfield RUNT_FRAME : 1;
		icscm_bitfield ENABLE_PADDING : 1;
		icscm_bitfield PREEMPTION_ENABLED : 1;
		icscm_bitfield MPACKET_TYPE : 4;
		icscm_bitfield MPACKET_FRAG_CNT : 2;
		icscm_bitfield UPDATE_CHECKSUMS : 1;
		icscm_bitfield FCS_OVERRIDE : 1;
		icscm_bitfield CRC_ERROR : 1;
		icscm_bitfield FCS_VERIFIED : 1;
		icscm_bitfield DISABLE_TX_RECEIPT : 1;
		icscm_bitfield T1S_ETHERNET : 1;
	} header;

	// Word 1 - Extended ID flags
	struct {
		icscm_bitfield txlen : 12;
		icscm_bitfield TXMSG : 1;
		icscm_bitfield TXAborted : 1;
		icscm_bitfield T1S_SYMBOL : 1;
		icscm_bitfield T1S_BURST : 1;
	} eid;

	// Word 2 - T1S Status flags
	struct {
		icscm_bitfield TXCollision : 1;
		icscm_bitfield T1SWake : 1;
		icscm_bitfield : 14;
	} t1s_status;

	// Word 3 - T1S Node Information
	struct {
		icscm_bitfield T1S_BURST_COUNT : 8;
		icscm_bitfield T1S_NODE_ID : 8;
		uint8_t RESERVED[6];
	} t1s_node;

	// Words 4-7 - Reserved/Padding
	uint16_t stats;

	// Words 8-11 - Timestamp (offset 14-22)
	struct {
		uint64_t TS : 60;
		uint64_t : 3;
		uint64_t IsExtended : 1;
	} timestamp;


	uint16_t NetworkID;
	uint16_t Length;

};

#pragma pack(pop)

}

#endif // __cplusplus

#endif // __ETHERNETPACKET_H__
