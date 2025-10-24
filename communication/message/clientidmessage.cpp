#include "icsneo/communication/message/clientidmessage.h"
#include "icsneo/communication/icspb.h"
#include "icsneo/communication/command.h"
#include "icsneo/communication/message/extendedresponsemessage.h"

using namespace icsneo;

std::shared_ptr<ClientIdMessage> ClientIdMessage::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	ClientIdMessage decoded;
	commands::generic::v1::ClientId msg;

	if(!protoapi::processResponse(bytestream.data(), bytestream.size(), msg)) {
		return nullptr;
	}

	if(msg.has_client_id()) {
		decoded.clientId.emplace(msg.client_id());
	}

	return std::make_shared<ClientIdMessage>(decoded);
}
