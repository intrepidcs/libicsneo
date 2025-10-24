#ifndef TRANSMIT_MESSAGE_H_
#define TRANSMIT_MESSAGE_H_

#include <cstdint>
#include "icsneo/communication/message/message.h"  
#include "icsneo/api/eventmanager.h"

namespace icsneo {

struct TransmitMessage {
	static std::vector<uint8_t> EncodeFromMessage(std::shared_ptr<Frame> message, uint32_t client_id, const device_eventhandler_t& report);

	constexpr static size_t messageOptionsOffset = 0;
	constexpr static size_t messageOptionsSize = 20; // todo determine max
	constexpr static size_t messageCommonHeaderOffset = messageOptionsOffset + messageOptionsSize; 
	constexpr static size_t messageCommonHeaderSize = 28; // CoreminiMsgExtendedHdr
#pragma pack(push,1)
	struct
	{
		uint32_t clientId;
		uint32_t networkId;
		uint32_t reserved[3]; // set to 0
	} options;
	uint8_t commonHeader[messageCommonHeaderSize];
#pragma pack(pop)
};

} // namespace icsneo

#endif // TRANSMIT_MESSAGE_H_