#ifndef __ENCODER_H_
#define __ENCODER_H_

#include "communication/message/include/message.h"
#include "communication/message/include/canmessage.h"
#include "communication/include/packet.h"
#include "communication/include/network.h"
#include <queue>
#include <vector>
#include <memory>

#pragma pack(push, 1)

namespace icsneo {

class Encoder {
public:
	std::vector<uint8_t> encode(const std::shared_ptr<Message>& message);
	std::vector<uint8_t> encode(Command cmd, bool boolean) { return encode(cmd, std::vector<uint8_t>({ (uint8_t)boolean })); }
	std::vector<uint8_t> encode(Command cmd, std::vector<uint8_t> arguments = {});
	
private:
	typedef uint16_t icscm_bitfield;
	struct HardwareCANPacket {
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
			icscm_bitfield EID : 6;
		} dlc;
		unsigned char data[8];
		struct {
			uint64_t TS : 60;
			uint64_t : 3;
			uint64_t IsExtended : 1;
		} timestamp;
	};
};

}

#pragma pack(pop)

#endif