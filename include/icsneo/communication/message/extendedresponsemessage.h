#ifndef __EXTENDEDRESPONSEMESSAGE_H_
#define __EXTENDEDRESPONSEMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/communication/command.h"

namespace icsneo {

class ExtendedResponseMessage : public Message {
public:
	ExtendedResponseMessage(ExtendedCommand cmd, ExtendedResponse resp = ExtendedResponse::OK)
		: Message(Message::Type::ExtendedResponse), command(cmd), response(resp) {}

	const ExtendedCommand command;
	const ExtendedResponse response;
	std::vector<uint8_t> data;

#pragma pack(push, 2)
	struct ResponseHeader {
		ExtendedCommand command;
		uint16_t length;
	};

	struct PackedGenericResponse {
		ResponseHeader header;
		ExtendedCommand command; // `header.command` is ExtendedCommand::GenericReturn, this is the real command
		ExtendedResponse returnCode;
	};
#pragma pack(pop)
};

}

#endif // __cplusplus

#endif