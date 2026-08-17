#include "icsneo/communication/message/genericapidatamessage.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include "icsneo/communication/command.h"

using namespace icsneo;

std::shared_ptr<GenericAPIDataMessage> GenericAPIDataMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	if(bytestream.size() < sizeof(ExtendedResponseMessage::ResponseHeader))
		return nullptr;

	const auto* hdr = reinterpret_cast<const ExtendedResponseMessage::ResponseHeader*>(bytestream.data());

	if(hdr->command != ExtendedCommand::ReadGenericAPIData)
		return nullptr;

	const size_t required = sizeof(ExtendedResponseMessage::ResponseHeader) + sizeof(GenericAPIDataHeader);
	if(bytestream.size() < required)
		return nullptr;

	auto msg = std::make_shared<GenericAPIDataMessage>();

	const auto* packet = reinterpret_cast<const GenericAPIDataPacket*>(bytestream.data() + sizeof(ExtendedResponseMessage::ResponseHeader));

	msg->functionId = packet->header.functionId;
	msg->buffer.resize(packet->header.bufferLength);
	std::copy(packet->buffer, packet->buffer + packet->header.bufferLength, msg->buffer.data());
	return msg;
}
