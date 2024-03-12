#include "icsneo/communication/message/callback/streamoutput/a2bwavoutput.h"
#include "icsneo/device/tree/rada2b/rada2b.h"
#include "icsneo/icsneocpp.h"

namespace icsneo {

A2BWAVOutput::A2BWAVOutput(
	const char* filename,
	const ChannelMap& channelMap,
	PCMType bitDepth,
	size_t numWAVChannels,
	uint32_t sampleRate
) 
	: StreamOutput(filename), chMap(channelMap), wavSampleRate(sampleRate), numChannelsWAV(numWAVChannels) {
	switch(bitDepth) {
		case PCMType::L16:
			bytesPerSampleWAV = 2;
			break;
		case PCMType::L24:
			bytesPerSampleWAV = 3;
			break;
		case PCMType::L32:
			bytesPerSampleWAV = 4;
			break;
	}

	if(initialize()) {
		initialized = true; 
	}
}

A2BWAVOutput::A2BWAVOutput(
	std::ostream& os,
	const ChannelMap& channelMap,
	PCMType bitDepth,
	size_t numWAVChannels,
	uint32_t sampleRate
) 
	: StreamOutput(os), chMap(channelMap), wavSampleRate(sampleRate), numChannelsWAV(numWAVChannels) {
	switch(bitDepth) {
		case PCMType::L16:
			bytesPerSampleWAV = 2;
			break;
		case PCMType::L24:
			bytesPerSampleWAV = 3;
			break;
		case PCMType::L32:
			bytesPerSampleWAV = 4;
			break;
	}

	if(initialize()) {
		initialized = true; 
	}
}

A2BWAVOutput::~A2BWAVOutput() {
	if(!closed) {
		close();
	}
}

bool A2BWAVOutput::initialize() {
	static constexpr size_t maxWAVChannels = 256;

	if(numChannelsWAV > maxWAVChannels) {
		return false;
	}

	maxMessageChannel = 0;
	
	// Check if the inputted channel map has invalid mappings and compute maxMessageChannel
	for(auto [wavChannel, messageChannel] : chMap) {
		maxMessageChannel = std::max<size_t>(maxMessageChannel, messageChannel);
		if(wavChannel >= numChannelsWAV) {
			return false;
		}
	}
	
	WAVHeader header = WAVHeader(
		static_cast<uint16_t>(chMap.size()),
		wavSampleRate,
		static_cast<uint16_t>(bytesPerSampleWAV * 8)
	);

	if(!stream->write(reinterpret_cast<const char*>(&header), sizeof(WAVHeader))) {
		return false;
	}

	streamStartPos = static_cast<uint32_t>(stream->tellp());
	wavBuffer = std::vector<uint8_t>(wavBufferSize, 0);
	wavBufferOffset = 0;
	return true;
}

bool A2BWAVOutput::callIfMatch(const std::shared_ptr<Message>& message) const {
	if(!initialized) {
		return false;
	}

	if(closed) {
		return false;
	}

	if(message->type != Message::Type::Frame) {
		return false;
	}

	const auto& frameMsg = std::dynamic_pointer_cast<Frame>(message);

	if(!frameMsg) {
		return false;
	}

	if(frameMsg->network.getType() != Network::Type::A2B)
		return false;

	const auto& a2bMsg = std::dynamic_pointer_cast<A2BMessage>(frameMsg);

	if(!a2bMsg) {
		return false;
	}

	size_t frameSize = a2bMsg->getFrameSize();
	size_t wavFrameSize = numChannelsWAV * bytesPerSampleWAV;
	size_t bytesPerChannel = static_cast<size_t>(a2bMsg->getBytesPerChannel());
	size_t numMessageChannels = 2 * a2bMsg->numChannels;
	size_t numFrames = a2bMsg->getNumFrames();

	const uint8_t* audioBuffer = a2bMsg->data.data();

	if(maxMessageChannel >= numMessageChannels) {
		// The max message channel in our channel map is larger than the number of channels in this message
		// this is likely due to the user inputting incorrect settings
		return false;
	}

	for(size_t frame = 0; frame < numFrames; frame++) {
		// Check to see if we can read another frame in wavBuffer, otherwise write and clear the buffer
		if(wavBufferOffset + wavFrameSize >= wavBufferSize) {
			if(!writeCurrentBuffer()) {
				return false;
			}
		}

		for(size_t wavChannel = 0; wavChannel < numChannelsWAV; wavChannel++) {
			if(auto iter = chMap.find(static_cast<uint8_t>(wavChannel)); iter != chMap.end()) {
				auto messageChannel = iter->second;
				size_t messageChannelOffset = messageChannel * bytesPerChannel + frameSize* frame;
				
				// Samples in the WAV are little endian signed integers
				// Samples in the message channels are little endian signed integers that are
				// most significant bit aligned
				if(a2bMsg->channelSize16) {
					// In this case, the channel size will be less than or equal to the sample we are writing
					// so we zero out any of the least significant bytes which won't be occupied by a sample byte

					for(size_t zeroByte = 0; zeroByte < bytesPerSampleWAV - bytesPerChannel; zeroByte++) {
						wavBuffer[wavBufferOffset++] = 0;
					}
					
					// Write the channel data in the most signifant bytes of the wav sample, this effectively
					// writes a sample which is scaled up.
					for(size_t channelByte = 0; channelByte < bytesPerChannel; channelByte++) {
						wavBuffer[wavBufferOffset++] = audioBuffer[messageChannelOffset + channelByte];
					}
				} else {
					// In this case, the channel size will be greater than or equal to the sample we are reading

					// Align the wav sample with the most significant bytes of the channel
					size_t channelByte = messageChannelOffset + (bytesPerChannel - bytesPerSampleWAV); 

					// Read the most significant bytes of the channel into the wavBuffer
					for(size_t sampleByte = 0; sampleByte < bytesPerSampleWAV; sampleByte++, channelByte++) {
						wavBuffer[wavBufferOffset++] = audioBuffer[channelByte];
					}
				}
			} else {
				// If this channel wasn't specified in the channel map, set a zero sample
				for(
					size_t sampleByte = 0;
					sampleByte < bytesPerSampleWAV;
					sampleByte++
				) {
					wavBuffer[wavBufferOffset++] = 0;
				}
			}
		}
	}

	return true;
}

void A2BWAVOutput::close() const {
	if(closed) {
		return;
	}

	if(!initialized) {
		return;
	}

	// Write any left over data in the buffer
	if(wavBufferOffset > 0) {
		writeCurrentBuffer();
	}

	// Seek back in the output stream and write the WAV chunk sizes
	uint32_t streamEndPos = static_cast<uint32_t>(stream->tellp());

	uint32_t subChunk2Size = streamEndPos - streamStartPos;
	uint32_t chunkSize = streamEndPos - 8;

	stream->seekp(streamStartPos - 4);
	stream->write(reinterpret_cast<const char*>(&subChunk2Size), 4);
	stream->seekp(4, std::ios::beg);
	stream->write(reinterpret_cast<const char*>(&chunkSize), 4);

	closed = true;
}

bool A2BWAVOutput::writeCurrentBuffer() const {

	if(!stream->write(reinterpret_cast<const char*>(wavBuffer.data()), wavBufferOffset)) {
		return false;
	}

	wavBufferOffset = 0;

	return true;
}


}