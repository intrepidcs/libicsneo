#ifndef __CANMESSAGE_H_
#define __CANMESSAGE_H_

#include "communication/message/include/message.h"

namespace icsneo {

class CANMessage : public Message {
public:
	uint32_t arbid;
};

}

#endif