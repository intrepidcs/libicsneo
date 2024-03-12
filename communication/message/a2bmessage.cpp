#include "icsneo/communication/message/a2bmessage.h"
#include "icsneo/communication/message/callback/streamoutput/streamoutput.h"

using namespace icsneo;

// Read a 16 bit sample from the audio buffer, which is stored as little endian
#define SAMPLE_FROM_BYTES_16(audioData) (((audioData)[0]) | ((audioData)[1] << 8))

// Read a 32 bit sample from the audio buffer
#define SAMPLE_FROM_BYTES_32(audioData) (((audioData)[0]) | ((audioData)[1] << 8) | ((audioData)[2] << 16) | ((audioData)[3] << 24))

// Read the most significant bytes of a sample stored in a 32 bit unsigned integer into audioData
#define SAMPLE_TO_BYTES_16(audioData, offset, sample) {\
	(audioData)[(offset)++] = static_cast<uint8_t>(((sample) & 0x00FF0000u) >> 16);\
	(audioData)[(offset)++] = static_cast<uint8_t>(((sample) & 0xFF000000u) >> 24);\
}

// Read little endian a 32 bit unsigned integer into audioData
#define SAMPLE_TO_BYTES_32(audioData, offset, sample) {\
	(audioData)[(offset)++] = static_cast<uint8_t>(((sample) & 0x000000FFu));\
	(audioData)[(offset)++] = static_cast<uint8_t>(((sample) & 0x0000FF00u) >> 8);\
	(audioData)[(offset)++] = static_cast<uint8_t>(((sample) & 0x00FF0000u) >> 16);\
	(audioData)[(offset)++] = static_cast<uint8_t>(((sample) & 0xFF000000u) >> 24);\
}

uint8_t A2BMessage::tdmToChannelNum(TDMMode tdm) {
	switch(tdm) {
		case TDMMode::TDM2:
			return 4;
		case TDMMode::TDM4:
			return 8;
		case TDMMode::TDM8:
			return 16;
		case TDMMode::TDM12:
			return 24;
		case TDMMode::TDM16:
			return 32;
		case TDMMode::TDM20:
			return 40;
		case TDMMode::TDM24:
			return 48;
		case TDMMode::TDM32:
			return 64;
	}

	return 0;
}


uint8_t A2BMessage::getBytesPerChannel() const {
	return channelSize16 ? 2u : 4u;
}
size_t A2BMessage::getFrameSize() const {
	return static_cast<size_t>(2 * numChannels * getBytesPerChannel());
}

size_t A2BMessage::getSampleOffset(Direction dir, uint8_t channel, size_t frame) const {
	size_t frameSize = getFrameSize();
	size_t sampleOffset = static_cast<size_t>(frameSize * frame + 2 * channel *  getBytesPerChannel());

	if(dir == Direction::Upstream) {
		sampleOffset++;
	}

	return sampleOffset;
}

size_t A2BMessage::getNumFrames() const {
	size_t frameSize = getFrameSize();
	if(frameSize == 0) {
		return 0;
	}

	return data.size() / frameSize;
}

A2BMessage::A2BMessage(size_t numFrames, TDMMode tdm, bool chSize16) : channelSize16(chSize16) {
	numChannels = static_cast<uint8_t>(tdmToChannelNum(tdm) / 2);
	
	size_t frameSize = static_cast<size_t>(2 * numChannels * (chSize16 ? 2u : 4u));
	size_t audioBufferSize = frameSize * numFrames;
	if(audioBufferSize > maxAudioBufferSize) {
		size_t maxNumFrames = maxAudioBufferSize / frameSize;
		audioBufferSize = maxNumFrames * frameSize;
	}

	data.resize(std::min<size_t>(maxAudioBufferSize, audioBufferSize), 0);
}

A2BMessage::A2BMessage(TDMMode tdm, bool chSize16) : channelSize16(chSize16) {
	numChannels = static_cast<uint8_t>(tdmToChannelNum(tdm) / 2);
	size_t frameSize = static_cast<size_t>(2 * numChannels * (chSize16 ? 2u : 4u));
	
	size_t maxNumFrames = maxAudioBufferSize / frameSize;
	size_t audioBufferSize = maxNumFrames * frameSize;

	data.resize(audioBufferSize, 0);
}

PCMSample A2BMessage::getChannelSample(Direction dir, uint8_t channel, size_t frame, PCMType pcmType) const {
	size_t sampleOffset = getSampleOffset(dir, channel, frame);
	const uint8_t* audioData = &data[sampleOffset];
	PCMSample result = 0;

	// Samples coming from the device will either come from a 16 bit channel or 32 bit channel
	if(channelSize16) {
		int16_t sample16 = 0;
		uint16_t& uSample16 = *reinterpret_cast<uint16_t*>(&sample16);

		// Read little endian from the audio buffer
		uSample16 = SAMPLE_FROM_BYTES_16(audioData);
		
		// Scale the sample up according to the desired PCM size by
		// multiplying using logical shifting
		switch(pcmType) {
			case PCMType::L16:
				result = static_cast<PCMSample>(sample16);
				break;
			case PCMType::L24:
				result = static_cast<PCMSample>(sample16) << 8;
				break;
			case PCMType::L32:
				result = static_cast<PCMSample>(sample16) << 16;
				break;
		}
	} else {
		PCMSample sample32 = 0;
		uint32_t& uSample32 = *reinterpret_cast<uint32_t*>(&sample32);

		// Read little endian
		uSample32 = SAMPLE_FROM_BYTES_32(audioData);

		// Scale the sample down according to the desired PCM size by dividing using
		// logical shifting, if the A2B network was set up with the desired pcmType
		// there should be a clean division and no loss in PCM resolution.
		switch(pcmType) {
			case PCMType::L16:
				result = sample32 >> 16;
				break;
			case PCMType::L24:
				result = sample32 >> 8;
				break;
			case PCMType::L32:
				result = sample32;
				break;
		}
	}

	return result;
}

void A2BMessage::setChannelSample(Direction dir, uint8_t channel, size_t frame, PCMSample sampleToSet, PCMType pcmType) {
	
	size_t sampleOffset = getSampleOffset(dir, channel, frame);
	uint8_t* audioData = data.data();
	uint32_t& uSample = *reinterpret_cast<uint32_t*>(&sampleToSet);

	// Align the bytes towards the most significant bit by multiplying using
	// left shifts
	switch(pcmType) {
		case PCMType::L16:
			sampleToSet = sampleToSet << 16;
			break;
		case PCMType::L24:
			sampleToSet = sampleToSet << 8;
			break;
	}

	if(channelSize16) {
		// Read the 2 most significant bytes of the sample
		SAMPLE_TO_BYTES_16(audioData, sampleOffset, uSample)
	} else {
		// Read the entire sample
		SAMPLE_TO_BYTES_32(audioData, sampleOffset, uSample);
	}
}

bool A2BMessage::loadAudioBuffer(IWAVStream& wavStream, const ChannelMap& channelMap) {
	if(!wavStream) {
		return false;
	}

	size_t totalMessageChannels = numChannels * 2; // Multiply by two inorder to include both down and upstream channels
	size_t bytesPerChannel = static_cast<size_t>(getBytesPerChannel()); // Number of bytes per message channel
	size_t frameSize = getFrameSize();
	size_t numFrames = getNumFrames();

	size_t bytesPerSampleWAV = static_cast<size_t>(wavStream.header.bitsPerSample / 8); // Number of bytes per sample in the WAV data-stream
	size_t numWAVChannels = static_cast<size_t>(wavStream.header.numChannels);
	size_t wavFrameSize = numWAVChannels * bytesPerSampleWAV;

	if(bytesPerSampleWAV != 2 && bytesPerSampleWAV != 3 && bytesPerSampleWAV != 4) {
		return false;
	}

	if(numFrames == 0) {
		return false;
	}

	uint8_t* audioBuffer = data.data();

	std::vector<uint8_t> wavFrame(wavFrameSize, 0);
	for(size_t frame = 0; frame < numFrames; frame++) {

		// Read one frame of data from the input stream
		if(!wavStream.read(reinterpret_cast<char*>(wavFrame.data()), wavFrame.size())) {
			break;
		}

		// Iterate through each mapping and set a message channel to a channel in the WAV frame above
		for(const auto& [messageChannel, wavChannel] : channelMap) {
			if(messageChannel >= totalMessageChannels || wavChannel >= numWAVChannels) {
				return false;
			}

			size_t frameOffset = wavChannel * bytesPerSampleWAV; // Offset in the read WAV frame
			size_t audioBufferOffset = frame * frameSize + messageChannel * bytesPerChannel; // Offset in the message audio buffer

			if(bytesPerChannel < bytesPerSampleWAV) {
				// In this case, the message channels are smaller than the samples in the input WAV
				// samples in both the message channel and WAV are little endian, so we write only the
				// most significant bytes of the WAV

				// Align to most significant bytes of wav frame
				size_t align = bytesPerSampleWAV - bytesPerChannel;

				for(
					size_t frameByte = frameOffset + align;
					frameByte < frameOffset + bytesPerSampleWAV;
					frameByte++,
					audioBufferOffset++
				) {
					audioBuffer[audioBufferOffset] = wavFrame[frameByte];
				}
			} else {
				// The message channel is greater than or equal to the sample in the WAV
				// I2S specifies that the sample in this case is right aligned to the most significant
				// byte of the message channel

				// Align to most significant byte of audio buffer channel
				size_t align = bytesPerChannel - bytesPerSampleWAV;

				for(
					size_t audioByte = audioBufferOffset + align;
					audioByte < audioBufferOffset + bytesPerChannel;
					audioByte++,
					frameOffset++
				) {
					audioBuffer[audioByte] = wavFrame[frameOffset];
				}
			}
		}
	}

	return true;	
}


