#include "icsneo/communication/packet/a2bpacket.h"
#include <cstring>
#include <vector>

namespace icsneo {

const size_t HardwareA2BPacket::coreMiniMessageHeaderSize = 28;
const size_t HardwareA2BPacket::a2bMessageMaxLength = (size_t)HardwareA2BPacket::coreMiniMessageHeaderSize + 1024;
const size_t HardwareA2BPacket::a2bHeaderSize = 6;

std::shared_ptr<Message> HardwareA2BPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {

	if(bytestream.size() < coreMiniMessageHeaderSize)
	{
		return nullptr;
	}

	const HardwareA2BPacket *data = (const HardwareA2BPacket*)bytestream.data();

	size_t totalPackedLength = static_cast<size_t>(bytestream.size()) - static_cast<size_t>(coreMiniMessageHeaderSize); // First 28 bytes are message header.

	std::shared_ptr<A2BMessage> msg = std::make_shared<A2BMessage>(
		(uint8_t)data->header.channelNum,
		data->header.channelSize16,
		totalPackedLength
	);

	msg->setMonitorBit(data->header.monitor);
	msg->setTxMsgBit(data->header.txmsg);
	msg->setErrIndicatorBit(data->header.errIndicator);
	msg->setSyncFrameBit(data->header.syncFrame);
	msg->setRFU2(data->header.rfu2);
	msg->setAudioBuffer(bytestream.begin() + coreMiniMessageHeaderSize, bytestream.end());

	return msg;
}

bool HardwareA2BPacket::EncodeFromMessage(const A2BMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report) {
	constexpr size_t a2btxMessageHeaderSize = 6;

	if(message.getBytesPerSample() != 2 && message.getBytesPerSample() != 4) {
		report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
		return false;
	}

	size_t sampleBytes = message.getAudioBufferSize();
	size_t totalSize = a2btxMessageHeaderSize + sampleBytes;

	bytestream.resize(totalSize, 0);
	uint32_t offset = 0;
	
	bytestream[offset++] = 0;
	bytestream[offset++] = 0;
	bytestream[offset++] = (uint8_t)(sampleBytes & 0xFF);
	bytestream[offset++] = (uint8_t)((sampleBytes >> 8) & 0xFF);
	bytestream[offset++] = (uint8_t)((message.description >> 8) & 0xFF);
	bytestream[offset++] = (uint8_t)(message.description & 0xFF);

	std::copy(message.data.begin(), message.data.end(), bytestream.begin() + offset);

	return true;
}

}