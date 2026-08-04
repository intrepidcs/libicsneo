#include "icsneo/communication/message/spiportkeymessage.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/network.h"

using namespace icsneo;

std::shared_ptr<SPIPortKeyMessage> SPIPortKeyMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	if(bytestream.size() < sizeof(ExtendedResponseMessage::ResponseHeader))
		return nullptr;

	const auto* hdr = reinterpret_cast<const ExtendedResponseMessage::ResponseHeader*>(bytestream.data());

	if(hdr->command != ExtendedCommand::ExecuteSPIPortKeyOperation)
		return nullptr;

	const size_t required = sizeof(ExtendedResponseMessage::ResponseHeader) + sizeof(SPIPortKeyPacket);
	if(bytestream.size() < required)
		return nullptr;

	auto msg = std::make_shared<SPIPortKeyMessage>();

	const auto* packet = reinterpret_cast<const SPIPortKeyPacket*>(bytestream.data() + sizeof(ExtendedResponseMessage::ResponseHeader));
	msg->isKeyLoaded = packet->isKeyLoaded;

	return msg;
}
