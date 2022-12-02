#ifndef __A2BPACKET_H__
#define __A2BPACKET_H__

#ifdef __cplusplus

#include "icsneo/api/eventmanager.h"
#include <cstdint>
#include <memory>
#include <optional>
#include "icsneo/communication/message/a2bmessage.h"

namespace icsneo {

typedef uint16_t icscm_bitfield;

struct HardwareA2BPacket {

	static std::shared_ptr<Message> DecodeToMessage(const std::vector<uint8_t>& bytestream);
	static bool EncodeFromMessage(const A2BMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report);

	struct {
		// CxA2B
		icscm_bitfield channelNum : 8;
		icscm_bitfield channelSize16 : 1;
		icscm_bitfield : 7;

		// CxA2B2
		icscm_bitfield monitor : 1;
		icscm_bitfield txmsg : 1;
		icscm_bitfield errIndicator : 1;
		icscm_bitfield syncFrame : 1;
		icscm_bitfield upstream : 1;
		icscm_bitfield : 11;
		icscm_bitfield rfu2;
	} header;

	static const size_t coreMiniMessageHeaderSize;
	static const size_t a2bMessageMaxLength;
	static const size_t a2bHeaderSize;
};

}

#endif // __cplusplus

#endif