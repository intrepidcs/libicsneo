#ifndef __ISO9141PACKET_H__
#define __ISO9141PACKET_H__

#ifdef __cplusplus

#include "icsneo/communication/message/iso9141message.h"
#include "icsneo/api/eventmanager.h"
#include <cstdint>
#include <memory>
#include <vector>

namespace icsneo {

typedef uint16_t icscm_bitfield;

struct HardwareISO9141Packet {
	class Decoder {
	public:
		std::shared_ptr<ISO9141Message> decodeToMessage(const std::vector<uint8_t>& bytestream);
	private:
		std::shared_ptr<ISO9141Message> mMsg;
		size_t mGotPackets = 0;
	};

	struct {
		icscm_bitfield d8 : 8;
		icscm_bitfield options : 4;
		icscm_bitfield tx : 1;
		icscm_bitfield networkIndex : 3;
	} c1;
	struct {
		icscm_bitfield d9 : 8;
		icscm_bitfield d10 : 8;
	} c2;
	struct {
		icscm_bitfield len : 4;
		icscm_bitfield extendedNetworkIndexBit2 : 1;
		icscm_bitfield frm : 1;
		icscm_bitfield init : 1;
		icscm_bitfield extendedNetworkIndexBit : 1;
		icscm_bitfield d11 : 8;
	} c3;
	unsigned char data[8];
	uint16_t stats;
	struct {
		uint64_t TS : 60;
		uint64_t : 3; // Reserved for future status bits
		uint64_t IsExtended : 1;
	} timestamp;
};

}

#endif // __cplusplus

#endif