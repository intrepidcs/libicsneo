#ifndef __A2BWAVOUTPUT_H_
#define __A2BWAVOUTPUT_H_

#ifdef __cplusplus

#include "icsneo/communication/message/callback/streamoutput/streamoutput.h"
#include "icsneo/communication/message/a2bmessage.h"
#include "icsneo/device/device.h"

namespace icsneo {

/**
 * A message callback which injests A2BMessage PCM data and formats it into a WAV file
*/
class A2BWAVOutput : public StreamOutput {
public:
	static constexpr size_t wavBufferSize = 1024 * 32;

	/**
	 * Creates a new A2BWAVOutput object
	 * 
	 * @param filename Name of desired output WAV file
	 * @param channelMap A map which maps a channel in the output WAV file to a channel in received messages. See docs for specific channel format in messages
	 * @param bitDepth The size of the samples in the WAV file.
	 * @param numWAVChannels The number of channels in the output WAV file
	 * @param sampleRate The output WAV file sample rate
	*/
	A2BWAVOutput(
		const char* filename,
		const ChannelMap& channelMap,
		PCMType bitDepth,
		size_t numWAVChannels,
		uint32_t sampleRate = 48000
	);

	/**
	 * Creates a new A2BWAVOutput object
	 * 
	 * @param os A std::ostream object which represents this WAV file
	 * @param channelMap A map which maps a channel in the output WAV file to a channel in received messages. See docs for specific channel format in messages
	 * @param bitDepth The size of the samples in the WAV file.
	 * @param numWAVChannels The number of channels in the output WAV file
	 * @param sampleRate The output WAV file sample rate
	*/
	A2BWAVOutput(
		std::ostream& os,
		const ChannelMap& channelMap,
		PCMType bitDepth,
		size_t numWAVChannels,
		uint32_t sampleRate = 48000
	);

	bool callIfMatch(const std::shared_ptr<Message>& message) const override;

	~A2BWAVOutput() override;

protected:
	void close() const;
	bool initialize();

	/**
	 * Write and clear the current stored audio buffer
	*/
	bool writeCurrentBuffer() const;

	mutable std::vector<uint8_t> wavBuffer; // A buffer which is used to cache PCM data to write to disk later
	mutable size_t wavBufferOffset = 0; // Current offset in the above buffer, gets incremented as data is read into buffer
	uint32_t wavSampleRate; // The output WAV sample rate

	size_t bytesPerSampleWAV; // The number of bytes per sample in the output WAV file
	size_t numChannelsWAV; // The number of channels in the output WAV file

	ChannelMap chMap; // A map which maps a WAV channel to a A2BMessage channel
	size_t maxMessageChannel; // The highest message channel in the above channel map, this variable is used for error checking

	bool initialized = false;
	mutable uint32_t streamStartPos;
	mutable bool closed = false;
};

}

#endif // __cplusplus

#endif
