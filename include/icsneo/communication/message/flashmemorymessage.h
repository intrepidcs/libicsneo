#ifndef __FLASHMEMORYMESSAGE_H_
#define __FLASHMEMORYMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

namespace icsneo {

class FlashMemoryMessage : public InternalMessage {
public:
	FlashMemoryMessage() : InternalMessage(Message::Type::InternalMessage, Network::NetID::RED_INT_MEMORYREAD) {}
	uint16_t startAddress = 0;
};

}

#endif // __cplusplus

#endif