#ifndef __READMEMORYMESSAGE_H_
#define __READMEMORYMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

namespace icsneo {

class NeoReadMemorySDMessage : public InternalMessage {
public:
	NeoReadMemorySDMessage() : InternalMessage(Message::Type::InternalMessage, Network::_icsneo_netid_t::icsneo_netid_neo_memory_sdread) {}
	uint32_t startAddress = 0;
};

}

#endif // __cplusplus

#endif