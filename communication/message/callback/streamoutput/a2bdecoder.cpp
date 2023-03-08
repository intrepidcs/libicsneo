#include "icsneo/communication/message/callback/streamoutput/a2bdecoder.h"
#include <chrono>
#include "icsneo/icsneocpp.h"


namespace icsneo {

static constexpr uint8_t maxChannel = 255;


size_t A2BAudioChannelMap::getChannelIndex(Channel channel, A2BMessage::A2BDirection dir) const {
	size_t output = (size_t)channel;

	if(dir == A2BMessage::A2BDirection::Upstream) {
		output++;
	}
	return output;
}

A2BAudioChannelMap::A2BAudioChannelMap(uint8_t tdm) {
	rawMap.resize(2*tdm, maxChannel);
}

void A2BAudioChannelMap::set(Channel outChannel, A2BMessage::A2BDirection dir, Channel inChannel) {
	auto index = getChannelIndex(outChannel, dir);
	rawMap[index] = inChannel;
}

void A2BAudioChannelMap::setAll(Channel inChannel) {
	std::fill(rawMap.begin(), rawMap.end(), inChannel);
}

Channel A2BAudioChannelMap::get(Channel outChannel, A2BMessage::A2BDirection dir) const {
	auto index = getChannelIndex(outChannel, dir);
	
	return rawMap[index];
}

size_t A2BAudioChannelMap::A2BAudioChannelMap::size() const {
	return rawMap.size();
}

uint8_t A2BAudioChannelMap::getTDM() const {
	return (uint8_t)(rawMap.size() / 2);
}

Channel& A2BAudioChannelMap::operator[](size_t idx) {
	return rawMap[idx];
}

A2BAudioChannelMap::operator const std::vector<Channel>&() const {
	return rawMap;
}

A2BDecoder::A2BDecoder(
	std::unique_ptr<std::istream>&& streamOut,
	bool chSize16,
	const A2BAudioChannelMap& chMap
) : channelSize16(chSize16), channelMap(chMap) {
	stream = std::move(streamOut);
	tdm = chMap.getTDM();
	initializeFromHeader();
}

A2BDecoder::A2BDecoder(
	const char* filename,
	bool chSize16,
	const A2BAudioChannelMap& chMap
) : A2BDecoder(std::make_unique<std::ifstream>(filename, std::ios::binary), chSize16, chMap) { }

A2BDecoder::operator bool() const {
	return initialized && *stream;
}

void A2BDecoder::initializeFromHeader() {
	WaveFileHeader header;
	if(!stream->read((char*)&header, sizeof(header))) {
		initialized = false;
		return;
	}

	// Only allow 16 or 24 bit samples
	if(header.bitsPerSample != 16 && header.bitsPerSample != 24) {
		initialized = false;
		return;
	}

	audioBytesPerSample = header.bitsPerSample == 16 ? 2 : 3;
	channelsInWave = (uint8_t)header.numChannels;

	size_t bytesPerSample = channelSize16 ? 2 : 4;
	size_t frameSize = 2*tdm*bytesPerSample;
	size_t frameSizeWave = (size_t)(channelsInWave) * (size_t)(audioBytesPerSample);

	frame.resize(frameSize, 0);
	frameWave.resize(frameSizeWave, 0);

	initialized = true;
}

std::shared_ptr<A2BMessage> A2BDecoder::decode() {
	if(!*(this)) {
		return nullptr;
	}

	auto a2bMessagePtr = std::make_shared<icsneo::A2BMessage>(
		tdm,
		channelSize16,
		2048
	);

	A2BMessage& a2bMessage = *a2bMessagePtr.get();

	a2bMessage.setMonitorBit(false); // Probably not necessary
	a2bMessage.setTxMsgBit(true);

	a2bMessage.network = Network(Network::NetID::A2B2);

	for(uint32_t frameIndex = 0; frameIndex < a2bMessage.getNumFrames(); frameIndex++) {
		if(!stream->read((char*)frameWave.data(), frameWave.size())) {
			break;
		}
		
		for(size_t icsChannel = 0; icsChannel < channelMap.size(); icsChannel++) {

			if(channelMap[icsChannel] >= maxChannel) {
				continue;
			}

			size_t wBegin = audioBytesPerSample * channelMap[icsChannel];
			A2BPCMSample sample = 0;
			uint8_t* sampBytes = (uint8_t*)&sample;

			std::copy(frameWave.begin() + wBegin, frameWave.begin() + wBegin + audioBytesPerSample, sampBytes);
			a2bMessage[frameIndex][icsChannel] = sample;
		}
	}

	return a2bMessagePtr;
}

bool A2BDecoder::outputAll(std::shared_ptr<Device>& device) {
	const auto& networks = device->getSupportedTXNetworks();

	if(std::none_of(networks.begin(), networks.end(), [](const Network& net) { return net.getNetID() == Network::NetID::A2B2; })) {
		return false;
	}

	while(*this) {
		device->transmit(decode());
	}

	return true;
}

}