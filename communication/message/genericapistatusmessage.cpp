#include "icsneo/communication/message/genericapistatusmessage.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include "icsneo/communication/command.h"

using namespace icsneo;

std::shared_ptr<GenericAPIStatusMessage> GenericAPIStatusMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	if(bytestream.size() < sizeof(ExtendedResponseMessage::ResponseHeader))
		return nullptr;

	const auto* hdr = reinterpret_cast<const ExtendedResponseMessage::ResponseHeader*>(bytestream.data());

	if(hdr->command != ExtendedCommand::ReadGenericAPIStatus)
		return nullptr;

	const size_t required = sizeof(ExtendedResponseMessage::ResponseHeader) + sizeof(GenericAPIStatusResponsePacket);
	if(bytestream.size() < required)
		return nullptr;

	auto msg = std::make_shared<GenericAPIStatusMessage>();

	const auto* packet = reinterpret_cast<const GenericAPIStatusResponsePacket*>(bytestream.data() + sizeof(ExtendedResponseMessage::ResponseHeader));

	msg->functionId = packet->functionId;
	msg->finishedProcessing = static_cast<bool>(packet->finishedProcessing);
	msg->functionError = packet->functionError;
	msg->callbackError = packet->callbackError;
	return msg;
}
