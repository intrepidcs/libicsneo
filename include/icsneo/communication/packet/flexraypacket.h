#ifndef __FLEXRAYPACKET_H__
#define __FLEXRAYPACKET_H__

#include "icsneo/communication/message/flexray/flexraymessage.h"
#include "icsneo/api/eventmanager.h"
#include <cstdint>
#include <memory>

namespace icsneo {

struct HardwareFlexRayPacket {
	static std::shared_ptr<FlexRayMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static bool EncodeFromMessage(const FlexRayMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);

	//Word 0
	uint16_t slotid : 11;
	uint16_t startup : 1;
	uint16_t sync : 1;
	uint16_t null_frame : 1;
	uint16_t payload_preamble : 1;
	uint16_t reserved_0 : 1;
	//Word 1
	uint16_t hdr_crc_10 : 1;
	uint16_t payload_len : 7;
	uint16_t reserved_1 : 4;
	uint16_t txmsg : 1;
	uint16_t reserved_2 : 3;
	//Word 2
	uint16_t cycle : 6;
	uint16_t hdr_crc_9_0 : 10;
	//Word 3 (D0-D1)
	union {
		uint16_t word;
		struct
		{
			uint16_t bytesRxed : 9;
			uint16_t dynamic : 1;
			uint16_t chb : 1;
			uint16_t hcrc_error : 1;
			uint16_t fcrc_error : 1;
			uint16_t reserved_3 : 3;
		} bits;
	} statusBits;
	//Word 4 (D2-D3)
	uint16_t tss_length_12_5ns;
	//Word 5 (D4-D5)
	uint16_t frame_length_12_5ns;
	//Word 6 (D6-D7)
	uint16_t extra;
	//Word 7
	uint16_t stat;
	//Word 8-14 (Timestamp)
	struct {
		uint64_t TS : 60;
		uint64_t : 3; // Reserved for future status bits
		uint64_t IsExtended : 1;
	} timestamp;
	//Word 15 (From Extended Header)
	uint16_t NetworkID;
	//Word 16 (From Extended Header)
	uint16_t Length;
};

}

#endif