#ifndef __A2BMESSAGE_H_
#define __A2BMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"


#define A2BMESSAGE_UPSTREAM 1
#define A2BMESSAGE_DOWNSTREAM 0

#define A2BPCM_SAMPLE_SIZE 4

#define A2BPCM_L16 16
#define A2BPCM_L24 24

#define A2BPCM_SAMPLERATE_44100 44100
#define A2BPCM_SAMPLERATE_48000 48000

namespace icsneo {

typedef uint32_t A2BPCMSample;
typedef std::vector<A2BPCMSample> ChannelBuffer;

class A2BMessage : public Frame {
public:
	enum class A2BDirection : uint8_t {
		DownStream = 0,
		UpStream = 1
	};

	A2BMessage() = delete;

	A2BMessage(uint8_t bitDepth, uint8_t bytesPerSample, uint8_t numChannels) : 
		bDepth(bitDepth), 
		bps(bytesPerSample)
	{
		downstream.resize(numChannels);
		upstream.resize(numChannels);
	}

	void addSample(A2BPCMSample&& sample, A2BDirection dir, uint8_t channel) {
		if(dir == A2BDirection::DownStream) {
			downstream[channel].push_back(std::move(sample));
		}
		else {
			upstream[channel].push_back(std::move(sample));
		}
		totalSamples++;
	}

	const A2BPCMSample* getSamples(A2BDirection dir, uint8_t channel) const {
		if(channel >= getNumChannels()) {
			return nullptr;
		}

		if(dir == A2BDirection::DownStream) {
			return downstream[channel].data();
		}
		return upstream[channel].data();
	}

	std::optional<A2BPCMSample> getSample(A2BDirection dir, uint8_t channel, uint32_t frame) const {
		const A2BPCMSample* samples = getSamples(dir, channel);
		auto numSamplesInChannel = getNumSamplesInChannel(dir, channel);

		if(
			samples == nullptr ||
			frame >= numSamplesInChannel.value_or(0)
		) {
			return std::nullopt;
		}

		return samples[frame];
	}

	std::optional<size_t> getNumSamplesInChannel(A2BDirection dir, uint8_t channel) const {
		if(channel >= getNumChannels()) {
			return std::nullopt;
		}

		if(dir == A2BDirection::DownStream) {
			return downstream[channel].size();
		}

		return upstream[channel].size();
	}

	const std::vector<ChannelBuffer>& getDownstream() const {
		return downstream;
	}

	const std::vector<ChannelBuffer>& getUpstream() const {
		return upstream;
	}

	size_t getNumSamples() const {
		return totalSamples;
	}

	uint8_t getNumChannels() const {
		return static_cast<uint8_t>(downstream.size());
	}

	uint8_t getBitDepth() const {
		return bDepth;
	}

	uint8_t getBytesPerSample() const {
		return bps;
	}

	// Equals operation for testing.
	bool operator==(const A2BMessage& rhs) const {
		return channelSize16 == rhs.channelSize16 &&
			monitor == rhs.monitor &&
			txmsg == rhs.txmsg &&
			errIndicator == rhs.errIndicator &&
			syncFrame == rhs.syncFrame &&
			rfu2 == rhs.rfu2 &&
			downstream == rhs.downstream &&
			upstream == rhs.upstream &&
			totalSamples == rhs.totalSamples &&
			bDepth == rhs.bDepth &&
			bps == rhs.bps;
	}

	bool channelSize16;
	bool monitor;
	bool txmsg;
	bool errIndicator;
	bool syncFrame;
	uint16_t rfu2;
private:
	std::vector<ChannelBuffer> downstream;
	std::vector<ChannelBuffer> upstream;
	size_t totalSamples = 0;
	
	uint8_t bDepth;
	uint8_t bps;
};

}

#endif // __cplusplus

#endif