#ifndef _EXTENDED_GENERIC_BINARY_STATUS_MESSAGE_H_
#define _EXTENDED_GENERIC_BINARY_STATUS_MESSAGE_H_

#ifdef __cplusplus
#include "icsneo/communication/message/extendedresponsemessage.h"

namespace icsneo {

class GenericBinaryStatusMessage : public Message {
public:
	GenericBinaryStatusMessage() : Message(Message::Type::GenericBinaryStatus) {}

	size_t binarySize;
	uint16_t binaryIndex;
	uint16_t binaryStatus;
};

}

#endif // __cplusplus

#endif // _EXTENDED_GENERIC_BINARY_STATUS_RESPONSE_MESSAGE_H_