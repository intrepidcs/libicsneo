#ifndef __CANERRORCOUNTMESSAGE_H_
#define __CANERRORCOUNTMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

namespace icsneo {

class CANErrorCountMessage : public Message {
public:
	CANErrorCountMessage(uint8_t tec, uint8_t rec, bool busOffFlag)
		: Message(Message::Type::CANErrorCount), transmitErrorCount(tec), receiveErrorCount(rec), busOff(busOffFlag){}

	Network network;
	uint8_t transmitErrorCount;
	uint8_t receiveErrorCount;
	bool busOff;
};

}

#endif // __cplusplus

#endif