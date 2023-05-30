#include "icsneo/communication/packet/genericbinarystatuspacket.h"
#include "icsneo/communication/message/genericbinarystatusmessage.h"

using namespace icsneo;

#pragma pack(push, 2)
struct GenericBinaryStatusResponse {
	ExtendedResponseMessage::ResponseHeader header;
	size_t size;
	uint16_t index;
	uint16_t status;
};
#pragma pack(pop)

std::shared_ptr<GenericBinaryStatusMessage> GenericBinaryStatusPacket::DecodeToMessage(const std::vector<uint8_t>& bytes) {
	if(bytes.size() < sizeof(GenericBinaryStatusResponse)) {
		return nullptr;
	}

	auto msg = std::make_shared<GenericBinaryStatusMessage>();

	const auto& response = *reinterpret_cast<const GenericBinaryStatusResponse*>(bytes.data());

	msg->binarySize = response.size;
	msg->binaryIndex = response.index;
	msg->binaryStatus = response.status;

	return msg;
}

std::vector<uint8_t> GenericBinaryStatusPacket::EncodeArguments(uint16_t binaryIndex) {
	std::vector<uint8_t> bytestream(sizeof(GenericBinaryStatusResponse));

	auto& parameters = *reinterpret_cast<GenericBinaryStatusResponse*>(bytestream.data());
	parameters.index = binaryIndex;

	return bytestream;
}
