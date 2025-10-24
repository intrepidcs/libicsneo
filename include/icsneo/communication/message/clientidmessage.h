#ifndef CLIENT_ID_MESSAGE_H_
#define CLIENT_ID_MESSAGE_H_

#include <cstdint>
#include "icsneo/communication/network.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/api/eventmanager.h"

namespace icsneo {

class ClientIdMessage : public Message {
public:
	ClientIdMessage() : Message(Message::Type::ClientId) {}
	static std::shared_ptr<ClientIdMessage> DecodeToMessage(const std::vector<uint8_t>& bytestream);

	std::optional<uint32_t> clientId;
};

} // namespace icsneo

#endif
