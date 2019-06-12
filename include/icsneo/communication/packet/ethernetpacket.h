#ifndef __ETHERNETPACKET_H__
#define __ETHERNETPACKET_H__

#include "icsneo/communication/message/ethernetmessage.h"
#include "icsneo/api/errormanager.h"
#include <cstdint>
#include <memory>

namespace icsneo {

typedef uint16_t icscm_bitfield;

struct HardwareEthernetPacket {
	static std::shared_ptr<EthernetMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static bool EncodeFromMessage(const EthernetMessage& message, std::vector<uint8_t>& bytestream, const device_errorhandler_t& err);

	struct {
		icscm_bitfield FCS_AVAIL : 1;
		icscm_bitfield RUNT_FRAME : 1;
		icscm_bitfield DISABLE_PADDING : 1;
		icscm_bitfield PREEMPTION_ENABLED : 1;
		icscm_bitfield MPACKET_TYPE : 4;
		icscm_bitfield MPACKET_FRAG_CNT : 2;
		icscm_bitfield : 6;
	} header;
	struct {
		icscm_bitfield txlen : 12;
		icscm_bitfield TXMSG : 1;
		icscm_bitfield : 3;
	} eid;
	icscm_bitfield reserved;
	unsigned char data[8];
	uint16_t stats;
	struct {
		uint64_t TS : 60;
		uint64_t : 3; // Reserved for future status bits
		uint64_t IsExtended : 1;
	} timestamp;
	uint16_t NetworkID;
	uint16_t Length;
};

}

#endif