#include "icsneo/communication/packet/a2bpacket.h"
#include <cstring>


namespace icsneo {

const size_t HardwareA2BPacket::coreMiniMessageHeaderSize = 28;
const size_t HardwareA2BPacket::a2bMessageMaxLength = (size_t)HardwareA2BPacket::coreMiniMessageHeaderSize + 1024;
const size_t HardwareA2BPacket::a2bHeaderSize = 6;

std::shared_ptr<Message> HardwareA2BPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {

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

	std::shared_ptr<A2BMessage> msg = std::make_shared<A2BMessage>(bitDepth, bytesPerChannel, numChannels);
	msg->channelSize16 = data->header.channelSize16;
	msg->monitor = data->header.monitor;
	msg->txmsg = data->header.txmsg;
	msg->errIndicator = data->header.errIndicator;
	msg->syncFrame = data->header.syncFrame;
	msg->rfu2 = data->header.rfu2;

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

bool HardwareA2BPacket::EncodeFromMessage(const A2BMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& report) {

	if(message.getBytesPerSample() != 2 && message.getBytesPerSample() != 4) {
		report(APIEvent::Type::MessageFormattingError, APIEvent::Severity::Error);
		return false;
	}

	size_t sampleBytes = message.getNumSamples() * static_cast<size_t>(message.getBytesPerSample());
	size_t totalSize = coreMiniMessageHeaderSize + sampleBytes;

	if(totalSize > a2bMessageMaxLength) {
		report(APIEvent::Type::MessageMaxLengthExceeded, APIEvent::Severity::Error);
		return false;
	}

	bytestream.reserve(totalSize);

	bytestream.push_back(message.getNumChannels());
	bytestream.push_back(message.channelSize16 ? 1 : 0);
	
	uint8_t a2b2Bits = 0;
	
	if(message.monitor) {
		a2b2Bits = a2b2Bits | 1;
	}

	if(message.txmsg) {
		a2b2Bits = a2b2Bits | (1 << 1);
	}

	if(message.errIndicator) {
		a2b2Bits = a2b2Bits | (1 << 2);
	}

	if(message.syncFrame) {
		a2b2Bits = a2b2Bits | (1 << 3);
	}

	bytestream.push_back(a2b2Bits);
	bytestream.push_back(0);
	bytestream.push_back(static_cast<uint8_t>(message.rfu2));
	bytestream.push_back(static_cast<uint8_t>(message.rfu2 >> 8));

	for(size_t i = 0; i < (coreMiniMessageHeaderSize - a2bHeaderSize); i++)
		bytestream.push_back(0);

	uint8_t numChannels = message.getNumChannels();

	uint8_t channel = 0;
	uint32_t frame = 0;

	auto writeSample = [&](A2BPCMSample&& sample) {
		for(uint32_t i = 0; i < static_cast<uint32_t>(message.getBytesPerSample()); i++) {
			bytestream.push_back(static_cast<uint8_t>((sample >> (i*8))));
		}
	};

	while(true) {
		auto dsSample = message.getSample(A2BMessage::A2BDirection::DownStream, channel, frame);
		auto usSample = message.getSample(A2BMessage::A2BDirection::UpStream, channel, frame);


		// Check if getSample failed for both downstream and upstream
		if(!dsSample && !usSample) {

			if(channel != 0) {
				//Incomplete frame, the frame we are currently on does not contain all channel samples
				report(APIEvent::Type::A2BMessageIncompleteFrame, APIEvent::Severity::Error);
				return false;
			}
			// Since no samples have been written for the current frame yet and there are no more
			// samples in both upstream and downstream, we can break and end parsing.
			break;
		}
		// Since the first case failed, at least one of the streams still has samples.
		// This case checks to see if the other stream does not have a sample.
		else if(!dsSample || !usSample) {
			// Report an error since we must have a one to one correspondence between upstream
			// and downstream.
			report(APIEvent::Type::A2BMessageIncompleteFrame, APIEvent::Severity::Error);
			return false;
		}
		
		writeSample(std::move(dsSample.value()));
		writeSample(std::move(usSample.value()));

		channel = (channel + 1) % numChannels;
		if(channel == 0) 
			frame++;
	}

	return true;
}

}