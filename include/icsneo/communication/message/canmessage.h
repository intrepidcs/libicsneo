#ifndef __CANMESSAGE_H_
#define __CANMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

namespace icsneo {

class CANMessage : public Message {
public:
	uint32_t arbid;
	uint8_t dlcOnWire;
	bool isRemote = false; // Not allowed if CAN FD
	bool isExtended = false;
	bool isCANFD = false;
	bool baudrateSwitch = false; // CAN FD only
	bool errorStateIndicator = false; // CAN FD only
};

}

#endif // __cplusplus

#endif