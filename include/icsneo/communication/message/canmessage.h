#ifndef __CANMESSAGE_H_
#define __CANMESSAGE_H_

#include "icsneo/communication/message/message.h"

namespace icsneo {

class CANMessage : public Message {
public:
	uint32_t arbid;
	uint8_t dlcOnWire;
	bool isRemote = false;
	bool isExtended = false;
	bool isCANFD = false;
	bool baudrateSwitch = false; // CAN FD only
};

}

#endif