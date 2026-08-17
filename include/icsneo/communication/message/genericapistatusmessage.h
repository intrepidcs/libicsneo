#ifndef __GENERICAPISTATUSMESSAGE_H_
#define __GENERICAPISTATUSMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"

#include <memory>

namespace icsneo {

class GenericAPIStatusMessage : public Message {
public:
#pragma pack(push, 2)
	struct GenericAPIStatusResponsePacket
	{
		uint8_t apiIndex;
		uint8_t instance;
		uint8_t functionId;
		uint8_t functionError;
		uint8_t callbackError;
		uint8_t finishedProcessing;
	};
#pragma pack(pop)

	static std::shared_ptr<GenericAPIStatusMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);

	GenericAPIStatusMessage() : Message(Type::GenericAPIStatus) {}

	uint8_t functionId;
	bool finishedProcessing;
	uint8_t functionError;
	uint8_t callbackError;
};

} // namespace icsneo

#endif // __cplusplus

#endif // __GENERICAPISTATUSMESSAGE_H_
