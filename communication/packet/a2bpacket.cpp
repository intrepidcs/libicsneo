#include "icsneo/communication/packet/a2bpacket.h"
#include <cstring>
#include <vector>

namespace icsneo {

const size_t HardwareA2BPacket::a2bMessageMaxLength = sizeof(HardwareA2BPacket) + 1024;

std::shared_ptr<Message> HardwareA2BPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {

	if(bytestream.size() < sizeof(HardwareA2BPacket))
	{
		return nullptr;
	}

	const HardwareA2BPacket* data = (const HardwareA2BPacket*)bytestream.data();

	size_t totalPackedLength = static_cast<size_t>(bytestream.size()) - sizeof(HardwareA2BPacket); // First 28 bytes are message header.

	if(totalPackedLength == 0) {
		return nullptr;
	}

	std::shared_ptr<A2BMessage> msg = std::make_shared<A2BMessage>();

	msg->numChannels = data->header.channelNum;
	msg->channelSize16 = data->header.channelSize16;
	msg->monitor = data->header.monitor;
	msg->txmsg = data->header.txmsg;
	msg->errIndicator = data->header.errIndicator;
	msg->syncFrame = data->header.syncFrame;
	msg->rfu2 = data->header.rfu2;
	msg->timestamp = data->timestamp.TS;

	msg->data = std::vector(bytestream.begin() + sizeof(HardwareA2BPacket), bytestream.end());
	return msg;
}

bool HardwareA2BPacket::EncodeFromMessage(const A2BMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& /*report*/) {
	constexpr size_t a2btxMessageHeaderSize = 6;

	size_t audioBufferSize = message.data.size();
	size_t totalSize = a2btxMessageHeaderSize + audioBufferSize;

	bytestream.resize(totalSize, 0);
	uint32_t offset = 0;
	
	bytestream[offset++] = 0;
	bytestream[offset++] = 0;
	bytestream[offset++] = (uint8_t)(audioBufferSize & 0xFF);
	bytestream[offset++] = (uint8_t)((audioBufferSize >> 8) & 0xFF);
	bytestream[offset++] = (uint8_t)((message.description >> 8) & 0xFF);
	bytestream[offset++] = (uint8_t)(message.description & 0xFF);

	std::copy(message.data.begin(), message.data.end(), bytestream.begin() + offset);

	return true;
}

}