#ifndef __LIVEDATAPACKET_H__
#define __LIVEDATAPACKET_H__

#ifdef __cplusplus

#include <cstdint>
#include <memory>
#include <vector>
#include "icsneo/api/eventmanager.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/communication/message/livedatamessage.h"
#include "icsneo/communication/livedata.h"

namespace icsneo {

class LiveDataMessage;

struct HardwareLiveDataPacket {
	static std::shared_ptr<Message> DecodeToMessage(const std::vector<uint8_t>& bytes, const device_eventhandler_t& report);
	static bool EncodeFromMessage(LiveDataMessage& message, std::vector<uint8_t>& bytes, const device_eventhandler_t& report);
};

}

#endif // __cplusplus

#endif // __LIVEDATAPACKET_H__
