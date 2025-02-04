#ifndef __CANERRORMESSAGE_H_
#define __CANERRORMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

namespace icsneo {

enum class CANErrorCode : uint8_t
{
	NoError = 0,
	StuffError = 1,
	FormError = 2,
	AckError = 3,
	Bit1Error = 4,
	Bit0Error = 5,
	CRCError = 6,
	NoChange = 7
};
class CANErrorMessage : public Message {
public:
	CANErrorMessage() : Message(Type::CANError) {}
	Network network;
	uint8_t transmitErrorCount;
	uint8_t receiveErrorCount;
	bool busOff;
	bool errorPassive;
	bool errorWarn;
	CANErrorCode dataErrorCode;
	CANErrorCode errorCode;
};

using CANErrorCountMessage = CANErrorMessage;

}

#endif // __cplusplus

#endif