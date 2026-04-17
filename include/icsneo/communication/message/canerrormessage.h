#ifndef __CANERRORMESSAGE_H_
#define __CANERRORMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/icsneoc2types.h"

namespace icsneo {

enum class CANErrorCode : icsneoc2_can_error_code_t
{
	NoError = icsneoc2_can_error_code_no_error,
	StuffError = icsneoc2_can_error_code_stuff_error,
	FormError = icsneoc2_can_error_code_form_error,
	AckError = icsneoc2_can_error_code_ack_error,
	Bit1Error = icsneoc2_can_error_code_bit1_error,
	Bit0Error = icsneoc2_can_error_code_bit0_error,
	CRCError = icsneoc2_can_error_code_crc_error,
	NoChange = icsneoc2_can_error_code_no_change
};
class CANErrorMessage : public RawMessage {
public:
	CANErrorMessage() : RawMessage(Type::CANError) {}
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