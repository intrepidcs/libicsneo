#ifndef __STREAMOUTPUT_H_
#define __STREAMOUTPUT_H_

#ifdef __cplusplus

#include "icsneo/communication/message/callback/messagecallback.h"
#include <memory>
#include <functional>
#include <iostream>
#include <fstream>

#define WAV_SAMPLE_RATE_44100 44100
#define WAV_SAMPLE_RATE_48000 48000

namespace icsneo {

struct WaveFileHeader {

	static constexpr uint32_t WAVE_CHUNK_ID = 0x46464952;	  // "RIFF"
	static constexpr uint32_t WAVE_FORMAT = 0x45564157;		  // "WAVE"
	static constexpr uint32_t WAVE_SUBCHUNK1_ID = 0x20746d66; // "fmt "
	static constexpr uint32_t WAVE_SUBCHUNK2_ID = 0x61746164; // "data"
	static constexpr uint16_t WAVE_SUBCHUNK1_SIZE = 16;
	static constexpr uint16_t WAVE_AUDIO_FORMAT_PCM = 1;
	static constexpr uint32_t WAVE_DEFAULT_SIZE = 0; // Default  size for streamed wav

	uint32_t chunkId = WAVE_CHUNK_ID;			  // "RIFF"
	uint32_t chunkSize = WAVE_DEFAULT_SIZE;		  // number of bytes to follow
	uint32_t format = WAVE_FORMAT;				  // "WAVE"
	uint32_t subchunk1Id = WAVE_SUBCHUNK1_ID;	  // "fmt "
	uint32_t subchunk1Size = WAVE_SUBCHUNK1_SIZE; // number of bytes in *this* subchunk (always 16)
	uint16_t audioFormat = WAVE_AUDIO_FORMAT_PCM; // 1 for PCM
	uint16_t numChannels;						  // number of channels
	uint32_t sampleRate;						  // sample rate in Hz
	uint32_t byteRate;							  // bytes per second of audio: sampleRate * numChannels * (bitsPerSample / 8)
	uint16_t blockAlign;						  // alignment of each block in bytes: numChannels * (bitsPerSample / 8)
	uint16_t bitsPerSample;						  // number of bits in each sample
	uint32_t subchunk2Id = WAVE_SUBCHUNK2_ID;	  // "data"
	uint32_t subchunk2Size = WAVE_DEFAULT_SIZE;	  // number of bytes to follow

	WaveFileHeader() = default;

	WaveFileHeader(uint16_t nChannels, uint32_t sRate, uint16_t bps, uint32_t nSamples = 0) {
		setHeader(nChannels, sRate, bps, nSamples);
	}

	void setHeader(uint16_t newNumChannels, uint32_t newSampleRate, uint16_t newBitsPerSample, uint32_t numSamples = 0) {
		numChannels = newNumChannels;
		sampleRate = newSampleRate;
		bitsPerSample = newBitsPerSample;
		blockAlign = numChannels * (bitsPerSample / 8);
		byteRate = sampleRate * numChannels * (bitsPerSample / 8);
		if(numSamples != 0) {
			setNumSamples(numSamples);
		}
	}

	void setNumSamples(uint32_t numSamples) {
		subchunk2Size = numSamples * numChannels * (bitsPerSample / 8);
		chunkSize = subchunk2Size + 36;
	}

	void write(const std::unique_ptr<std::ostream>& stream) {

		stream->write(reinterpret_cast<const char*>(&chunkId), 4);
		stream->write(reinterpret_cast<const char*>(&chunkSize), 4);
		stream->write(reinterpret_cast<const char*>(&format), 4);
		stream->write(reinterpret_cast<const char*>(&subchunk1Id), 4);
		stream->write(reinterpret_cast<const char*>(&subchunk1Size), 4);
		stream->write(reinterpret_cast<const char*>(&audioFormat), 2);
		stream->write(reinterpret_cast<const char*>(&numChannels), 2);
		stream->write(reinterpret_cast<const char*>(&sampleRate), 4);
		stream->write(reinterpret_cast<const char*>(&byteRate), 4);
		stream->write(reinterpret_cast<const char*>(&blockAlign), 2);
		stream->write(reinterpret_cast<const char*>(&bitsPerSample), 2);
		stream->write(reinterpret_cast<const char*>(&subchunk2Id), 4);
		stream->write(reinterpret_cast<const char*>(&subchunk2Size), 4);

	}
};

class StreamOutput : public MessageCallback {
public:
	StreamOutput(std::unique_ptr<std::ostream>&& os, fn_messageCallback cb, std::shared_ptr<MessageFilter> f)
		: MessageCallback(cb, f), stream(std::move(os)) {}

	StreamOutput(const char* filename, fn_messageCallback cb, std::shared_ptr<MessageFilter> f)
		: MessageCallback(cb, f) {
		stream = std::make_unique<std::ofstream>(filename, std::ios::binary);
	}

	StreamOutput(const char* filename) : MessageCallback([](std::shared_ptr<Message> msg) {}) {
		stream = std::make_unique<std::ofstream>(filename, std::ios::binary);
	}

	StreamOutput(std::unique_ptr<std::ostream>&& os) : MessageCallback([](std::shared_ptr<Message> msg) {}), stream(std::move(os)) {}

protected:
	std::unique_ptr<std::ostream> stream;

	void write(void* msg, std::streamsize size) const {
		stream->write(reinterpret_cast<const char*>(msg), size);
	}
};

}

#endif // __cplusplus

#endif
