#ifndef __READMEMORYMESSAGE_H_
#define __READMEMORYMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

namespace icsneo {

class NeoReadMemorySDMessage : public RawMessage {
public:
	NeoReadMemorySDMessage() : RawMessage(Message::Type::RawMessage, Network::NetID::NeoMemorySDRead) {}
	uint32_t startAddress = 0;
};

}

#endif // __cplusplus

#endif