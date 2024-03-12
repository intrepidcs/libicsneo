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

#pragma pack(push, 1)
struct WAVHeader {

	static constexpr uint32_t WAV_CHUNK_ID = 0x46464952;	  // "RIFF"
	static constexpr uint32_t WAV_FORMAT = 0x45564157;		  // "WAV"
	static constexpr uint32_t WAV_SUBCHUNK1_ID = 0x20746d66; // "fmt "
	static constexpr uint32_t WAV_SUBCHUNK2_ID = 0x61746164; // "data"
	static constexpr uint16_t WAV_SUBCHUNK1_SIZE = 16;
	static constexpr uint16_t WAV_AUDIO_FORMAT_PCM = 1;
	static constexpr uint32_t WAV_DEFAULT_SIZE = 0; // Default  size for streamed wav

	uint32_t chunkId = WAV_CHUNK_ID;			  // "RIFF"
	uint32_t chunkSize = WAV_DEFAULT_SIZE;		  // number of bytes to follow
	uint32_t format = WAV_FORMAT;				  // "WAV"
	uint32_t subchunk1Id = WAV_SUBCHUNK1_ID;	  // "fmt "
	uint32_t subchunk1Size = WAV_SUBCHUNK1_SIZE; // number of bytes in *this* subchunk (always 16)
	uint16_t audioFormat = WAV_AUDIO_FORMAT_PCM; // 1 for PCM
	uint16_t numChannels;						  // number of channels
	uint32_t sampleRate;						  // sample rate in Hz
	uint32_t byteRate;							  // bytes per second of audio: sampleRate * numChannels * (bitsPerSample / 8)
	uint16_t blockAlign;						  // alignment of each block in bytes: numChannels * (bitsPerSample / 8)
	uint16_t bitsPerSample;						  // number of bits in each sample
	uint32_t subchunk2Id = WAV_SUBCHUNK2_ID;	  // "data"
	uint32_t subchunk2Size = WAV_DEFAULT_SIZE;	  // number of bytes to follow

	WAVHeader() = default;

	WAVHeader(uint16_t nChannels, uint32_t sRate, uint16_t bps, uint32_t nSamples = 0) {
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
};

#pragma pack(pop)

class IWAVStream {
private:
	std::unique_ptr<std::istream, std::function<void(std::istream*)>> stream; 
	bool initialized = false;
public:
	WAVHeader header;
	
	IWAVStream(std::istream& WAVInput) 
		: stream(&WAVInput, [](std::istream*){}) {

		if(initialize()) {
			initialized = true;
		}
	}

	IWAVStream(const char* filename) 
		: stream(new std::ifstream(filename, std::ios::in | std::ios::binary), std::default_delete<std::istream>()) {

		if(initialize()) {
			initialized = true;
		}
	}

	bool initialize() {
		return !(!stream->read(reinterpret_cast<char*>(&header), sizeof(WAVHeader)));
	}

	operator bool() const {
		return initialized && stream && stream->good();
	}

	bool read(char* into, std::streamsize num) {
		return !(!stream->read(into, num));
	}

	/**
	 * Set stream immediately after WAV header
	*/
	void reset() {
		if(!(*this)) {
			return;
		}

		stream->clear();
		stream->seekg(sizeof(icsneo::WAVHeader), std::ios::beg);
	}
};

class StreamOutput : public MessageCallback {
public:
	StreamOutput(std::ostream& os, fn_messageCallback cb, std::shared_ptr<MessageFilter> f)
		: MessageCallback(cb, f), stream(&os, [](std::ostream*){}) {}

	StreamOutput(const char* filename, fn_messageCallback cb, std::shared_ptr<MessageFilter> f)
		: 
		MessageCallback(cb, f),
		stream(
			new std::ofstream(filename, std::ios::binary),
			std::default_delete<std::ostream>()
		) {}

	StreamOutput(const char* filename) : 
		MessageCallback([](std::shared_ptr<Message> msg) {}),
		stream(
			new std::ofstream(filename, std::ios::binary),
			std::default_delete<std::ostream>()
		) {}

	StreamOutput(std::ostream& os) : MessageCallback([](std::shared_ptr<Message> msg) {}), stream(&os, [](std::ostream*){}) {}

protected:
	std::unique_ptr<std::ostream, std::function<void(std::ostream*)>> stream;
};

}

#endif // __cplusplus

#endif
