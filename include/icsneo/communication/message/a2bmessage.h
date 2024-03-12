#ifndef __A2BMESSAGE_H_
#define __A2BMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/api/eventmanager.h"
#include <iostream>
#include <unordered_map>
#include "icsneo/communication/message/callback/streamoutput/streamoutput.h"

namespace icsneo {

using PCMSample = int32_t;
enum class PCMType : uint8_t {
	L16,
	L24,
	L32
};

using ChannelMap = std::unordered_map<uint8_t, uint8_t>;

class A2BMessage : public Frame {
public:
	static constexpr size_t maxAudioBufferSize = 2048;

	enum class TDMMode : uint8_t {
		TDM2 = 0,
		TDM4 = 1,
		TDM8 = 2,
		TDM12 = 3,
		TDM16 = 4,
		TDM20 = 5,
		TDM24 = 6,
		TDM32 = 7,
	};

	static uint8_t tdmToChannelNum(TDMMode tdm);

	enum class Direction : uint8_t {
		Downstream = 0,
		Upstream = 1
	};

	uint16_t rfu2 = 0;
	uint8_t numChannels = 0;
	bool channelSize16 = false;
	bool monitor = false;
	bool txmsg = false;
	bool errIndicator = false;
	bool syncFrame = false;

	A2BMessage() = default;
	/**
	 * Creates a new A2BMessage
	 * 
	 * @param numFrames The number of audio frames to hold in the message audio buffer
	 * @param tdm The TDM mode to transmit this message to, note this variable determines the number of channels
	 * @param chSize16 True if the message channel sizes are 16 bit, false for 32 bit.
	*/
	A2BMessage(size_t numFrames, TDMMode tdm, bool chSize16);

	/**
	 * Creates a new A2BMessage with the maximum number of possible frames
	 * 
	 * @param tdm The TDM mode to transmit this message to, note this variable determines the number of channels
	 * @param chSize16 True if the message channel sizes are 16 bit, false for 32 bit.
	*/
	A2BMessage(TDMMode tdm, bool chSize16);

	/**
	 * Loads A2BMessage audio buffer from a IWAVStream object representing a WAV data-stream
	 * 
	 * @param wavStream The WAV data-stream the audio buffer with
	 * @param channelMap A map which maps a message channel to a wav channel. See docs for A2B message channel format
	 * @returns true on successful load, false otherwise
	*/
	bool loadAudioBuffer(IWAVStream& wavStream, const ChannelMap& channelMap);

	/**
	 * Get a PCM sample from the audio buffer. If the desired pcmType is larger than the channel size,
	 * the output will be a PCM sample which is scaled up.
	 * 
	 * @param dir The direction of the A2B stream
	 * @param channel The desired channel to read a PCM sample from
	 * @param frame The desired frame to read a PCM sample from
	 * @param pcmType The interpretted bit depth of the audio buffer sample
	*/
	PCMSample getChannelSample(Direction dir, uint8_t channel, size_t frame, PCMType pcmType) const;

	/**
	 * Write a PCM sample to the audio buffer
	 * 
	 * @param dir The direction of the A2B stream
	 * @param channel The desired channel to write a PCM sample to
	 * @param frame The desired frame to write a PCM sample to
	 * @param sampleToSet The PCM sample which will be written to the buffer
	 * @param pcmType The interpretted bit depth of the sample to write
	*/
	void setChannelSample(Direction dir, uint8_t channel, size_t frame, PCMSample sampleToSet, PCMType pcmType);

	size_t getFrameSize() const;
	size_t getSampleOffset(Direction dir, uint8_t channel, size_t frame) const;
	uint8_t getBytesPerChannel() const;
	size_t getNumFrames() const;
};

}

#endif // __cplusplus

#endif