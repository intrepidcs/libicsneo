#include "icsneo/communication/packet/a2bpacket.h"
#include "icsneo/communication/message/a2bmessage.h"
#include <cstring>

using namespace icsneo;

std::shared_ptr<Message> HardwareA2BPacket::DecodeToMessage(const std::vector<uint8_t> &bytestream) {
	
	constexpr uint8_t coreMiniMessageHeaderSize = 28;
	
	if(bytestream.size() < coreMiniMessageHeaderSize)
	{
		return nullptr;
	}
	
	auto getSampleFromBytes = [](uint8_t bytesPerSample, const uint8_t *bytes) {
		A2BPCMSample result = 0;

		for(auto i = 0; i < bytesPerSample; i++) {
			result |= static_cast<uint32_t>(bytes[i]) << (i * 8);
		}

		return result;
	};


	const HardwareA2BPacket *data = (const HardwareA2BPacket*)bytestream.data();

	uint32_t totalPackedLength = static_cast<uint32_t>(bytestream.size()) - static_cast<uint32_t>(coreMiniMessageHeaderSize); // First 28 bytes are message header.

	uint8_t bytesPerChannel = data->header.channelSize16 ? 2 : 4;
	uint8_t numChannels = data->header.channelNum;
	uint8_t bitDepth = data->header.channelSize16 ? A2BPCM_L16 : A2BPCM_L24;
	bool monitor = data->header.monitor;

	std::shared_ptr<A2BMessage> msg = std::make_shared<A2BMessage>(bitDepth, bytesPerChannel, numChannels, monitor);

	const uint8_t *bytes = bytestream.data();
	bytes+=coreMiniMessageHeaderSize;

	uint8_t channel = 0;

	for(uint32_t i = 0; i < totalPackedLength; i += 2 * static_cast<uint32_t>(bytesPerChannel), bytes += 2 * bytesPerChannel, channel = (channel + 1) % numChannels) {

		msg->addSample(
			getSampleFromBytes(bytesPerChannel, bytes),
			A2BMessage::A2BDirection::DownStream,
			channel
		);

		msg->addSample(
			getSampleFromBytes(bytesPerChannel, bytes + bytesPerChannel),
			A2BMessage::A2BDirection::UpStream,
			channel
		);

	}

	return msg;
}
