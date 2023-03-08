#ifndef __A2BMESSAGE_H_
#define __A2BMESSAGE_H_

#ifdef __cplusplus

#include "icsneo/communication/message/message.h"
#include "icsneo/api/eventmanager.h"
#include <algorithm>
#include <cstring>
#include <iostream>


namespace icsneo {

typedef uint32_t A2BPCMSample;

class A2BMessage : public Frame {
private:
	class FrameView {
	private:
		class SampleView {
		public:
			SampleView(uint8_t* vPtr, uint8_t bps, size_t ind) :
				index(ind), viewPtr(vPtr), bytesPerSample(bps) {}

			operator A2BPCMSample() const {
				if(!viewPtr) {
					return 0;
				}	
				A2BPCMSample sample = 0;

				std::copy(viewPtr+index*bytesPerSample, viewPtr+(index+1)*bytesPerSample, (uint8_t*)&sample);
				if(bytesPerSample == 4) {
					sample = sample >> 8;
				}
				
				return sample;
			}

			SampleView& operator=(A2BPCMSample sample) {
				if(!viewPtr) {
					return *this;
				}

				if(bytesPerSample == 4) {
					sample = sample << 8;
				}
				std::copy((uint8_t*)&sample, (uint8_t*)&sample + bytesPerSample, viewPtr + index*bytesPerSample);
				return *this;
			}

			SampleView(const SampleView&) = delete;
			SampleView& operator=(const SampleView&) = delete;

		private:
		
			size_t index;
			uint8_t* viewPtr;
			uint8_t bytesPerSample;
		};

	public:
		FrameView(uint8_t* vPtr, uint8_t nChannels, uint8_t bps) : viewPtr(vPtr), tdm(nChannels), bytesPerSample(bps) {}

		SampleView operator[](size_t index) {

			if(index >= ((size_t)tdm) * 2) {
				EventManager::GetInstance().add(APIEvent(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error));
				return SampleView(nullptr, 0, 0);
			}
			return SampleView(viewPtr, bytesPerSample, index);
		}

		FrameView& operator=(const std::vector<A2BPCMSample>& samples) {
			if(!viewPtr) {
				return *this;
			}

			if(samples.size() != (size_t)(tdm)*2) {
				EventManager::GetInstance().add(APIEvent(APIEvent::Type::BufferInsufficient, APIEvent::Severity::Error));
				return *this;
			}

			for(size_t icsChannel = 0; icsChannel < ((size_t)(tdm) * 2); icsChannel++) {
				operator[](icsChannel) = samples[icsChannel];
			}

			return *this;
		}

		FrameView(const FrameView&) = delete;
		FrameView& operator=(const FrameView&) = delete;
	private:
		uint8_t* viewPtr;
		uint8_t tdm;
		uint8_t bytesPerSample;
	};

public:
	enum class A2BDirection : uint8_t {
		Downstream = 0,
		Upstream = 1
	};

	A2BMessage(uint8_t nChannels, bool chSize16, size_t size) : 
		numChannels(nChannels),
		channelSize16(chSize16)
	{	
		data.resize(std::min(roundNextMultiple(size, getFrameSize()),(size_t)maxSize), 0);
	}

	bool allocateSpace(size_t numSpaceToAdd) {
		size_t spaceToAdd = roundNextMultiple(numSpaceToAdd, getFrameSize());

		if(spaceToAdd + data.size() > maxSize) {
			return false;
		}

		data.resize(data.size() + numSpaceToAdd, 0);
		return true;
	}

	bool addFrame(const std::vector<A2BPCMSample>& frame) {
		if(frame.size() != ((size_t)numChannels)*2) {
			return false;
		}

		size_t oldSize = data.size();

		if(!allocateSpace(getFrameSize())) {
			return false;
		}
		
		auto it = data.begin() + oldSize;
		size_t offset = 0;
		for(A2BPCMSample sample: frame) {
			if(!channelSize16) {
				sample = sample << 8;
			}
			std::copy((uint8_t*)&sample, (uint8_t*)&sample + getBytesPerSample(), it + offset);
			offset+=getBytesPerSample();
		}

		return true; 
	}

	bool setFrame(const std::vector<A2BPCMSample>& frame, size_t frameNum) {
		if(frame.size() != ((size_t)numChannels)*2 || frameNum >= getNumFrames()) {
			return false;
		}
		
		auto it = data.begin() + frameNum*getFrameSize();
		size_t offset = 0;
		for(A2BPCMSample sample: frame) {
			if(!channelSize16) {
				sample = sample << 8;
			}
			std::copy((uint8_t*)&sample, (uint8_t*)&sample + getBytesPerSample(), it + offset);
			offset+=getBytesPerSample();
		}

		return true;
	}

	bool fillChannelAudioBuffer(A2BDirection dir, uint8_t channel, std::vector<uint8_t>& channelBuffer) const {
		if(channel >= numChannels) {
			return false;
		}

		size_t offset = getChannelIndex(dir, channel)*getBytesPerSample();

		for(size_t frame = 0; frame < getNumFrames(); frame++, offset += getFrameSize()) {
			std::copy(data.begin() + offset, data.end() + offset + getBytesPerSample(), std::back_inserter(channelBuffer));
		}

		return true;
	}

	bool fillChannelStream(A2BDirection dir, uint8_t channel, std::unique_ptr<std::ostream>& channelStream) const {
		if(channel >= numChannels) {
			return false;
		}

		size_t offset = getChannelIndex(dir, channel)*getBytesPerSample();

		for(size_t frame = 0; frame < getNumFrames(); frame++, offset += getFrameSize()) {
			channelStream->write((const char*)(data.data() + offset), getBytesPerSample());
		}

		return true;
	}

	void fill(A2BPCMSample sample) {
		uint8_t* buf = data.data();
		
		if(channelSize16) {
			uint16_t sample16bit = sample & 0xFF;
			uint16_t* samps = (uint16_t*)buf;
			std::fill(samps, samps + data.size()/2, sample16bit);
		}
		else {
			A2BPCMSample* samps = (A2BPCMSample*)buf;
			sample = sample << 8;
			std::fill(samps, samps + data.size()/4, sample);
		}
	}

	bool fillFrame(A2BPCMSample sample, size_t frame) {
		if(frame >= getNumFrames()) {
			return false;
		}

		uint8_t* buf = data.data();
		size_t start = 2 * numChannels * frame; 
		size_t end = 2 * numChannels * (frame+1);

		if(channelSize16) {
			uint16_t sample16bit = sample & 0xFF;
			uint16_t* samps = (uint16_t*)buf;
			std::fill(samps+start, samps + end, sample16bit);
		}
		else {
			A2BPCMSample* samps = (A2BPCMSample*)buf;
			sample = sample << 8;
			std::fill(samps+start, samps + end, sample);
		}
		return true;
	}

	template<typename Iterator>
	bool setAudioBuffer(Iterator begin, Iterator end, A2BDirection dir, uint8_t channel, uint32_t frame) {
		size_t offset = getChannelIndex(dir, channel)*getBytesPerSample() + frame * getFrameSize();
		size_t dist = (size_t)(std::distance(begin, end));

		if(dist > (data.size() - offset)) {
			return false;
		}

		std::copy(begin, end, data.begin() + offset);
		return true;
	}
	
	template<typename Iterator>
	bool setAudioBuffer(Iterator begin, Iterator end) {
		return setAudioBuffer(begin, end, A2BMessage::A2BDirection::Downstream, 0, 0);
	}

	std::optional<A2BPCMSample> getSample(A2BDirection dir, uint8_t channel, uint32_t frame) const {
		if(
			channel >= numChannels ||
			frame >= getNumFrames()
		) {
			return std::nullopt;
		}

		A2BPCMSample sample = 0;
		size_t offset = getChannelIndex(dir, channel)*getBytesPerSample() + frame * getFrameSize();

		std::copy(data.begin() + offset, data.begin() + offset + getBytesPerSample(), (uint8_t*)&sample);
		if(channelSize16) {
			sample = sample >> 8;
		}

		return sample;
	}

	std::optional<A2BPCMSample> getSample(size_t sampleNum) const {
		if(sampleNum >= getNumSamples()) {
			return std::nullopt;
		}


		A2BPCMSample sample = 0;
		size_t offset = sampleNum*getBytesPerSample();

		std::copy(data.begin() + offset, data.begin() + offset + getBytesPerSample(), (uint8_t*)&sample);

		if(channelSize16) {
			sample = sample >> 8;
		}

		return sample;
	}

	bool setSample(A2BDirection dir, uint8_t channel, uint32_t frame, A2BPCMSample sample) {
		if(
			channel >= numChannels ||
			frame >= getNumFrames()
		) {
			return false;
		}

		size_t offset = getChannelIndex(dir, channel)*getBytesPerSample() + frame * getFrameSize();
		
		if(!channelSize16) {
			sample = sample << 8;
		}
		uint8_t* sampToBytes = (uint8_t*)&sample;
		std::copy(sampToBytes,sampToBytes+getBytesPerSample(), data.begin() + offset);

		return true;
	}


	bool setSample(uint8_t icsChannel, uint32_t frame, A2BPCMSample sample) {
		if(
			icsChannel >= (2*numChannels) ||
			frame >= getNumFrames()
		) {
			return false;
		}

		size_t offset = ((size_t)icsChannel)*getBytesPerSample() + frame * getFrameSize();
		
		if(!channelSize16) {
			sample = sample << 8;
		}
		uint8_t* sampToBytes = (uint8_t*)&sample;
		std::copy(sampToBytes,sampToBytes+getBytesPerSample(), data.begin() + offset);		
		
		return true;
	}

	FrameView operator[](size_t index) {
		if(index >= getNumFrames()) {
			EventManager::GetInstance().add(APIEvent(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error));
			return FrameView(nullptr, 0, 0);
		}

		return FrameView(data.data() + index*getFrameSize(), numChannels, getBytesPerSample());
	}

	size_t getNumSamples() const {
		return data.size()/((size_t)getBytesPerSample());
	}

	uint8_t getNumChannels() const {
		return numChannels;
	}

	uint8_t getBitDepth() const {
		return channelSize16 ? 16 : 24;
	}

	uint8_t getBytesPerSample() const {
		return channelSize16 ? 2 : 4;
	}

	bool isTxMsg() const {
		return txmsg;
	}

	void setTxMsgBit(bool bit) {
		txmsg = bit;
	}

	bool isMonitorMsg() const {
		return monitor;
	}

	void setMonitorBit(bool bit) {
		monitor = bit;
	}

	bool isErrIndicator() const {
		return errIndicator;
	}

	void setErrIndicatorBit(bool bit)  {
		errIndicator = bit;
	}

	bool isSyncFrame() const {
		return syncFrame;
	}

	void setSyncFrameBit(bool bit) {
		syncFrame = bit;
	}

	uint16_t getRFU2() const {
		return rfu2;
	}

	void setRFU2(uint16_t newRfu2) {
		rfu2 = newRfu2;
	}

	size_t getFrameSize() const {
		return 2*((size_t)numChannels) * ((size_t)getBytesPerSample());
	}

	size_t getNumFrames() const {
		return data.size() / getFrameSize();
	}

	size_t getAudioBufferSize() const {
		return data.size();
	}

	const uint8_t* getAudioBuffer() const {
		return data.data();
	}


	static constexpr uint32_t maxSize = 2048;
private:

	uint8_t numChannels = 0;
	bool channelSize16 = false;
	bool monitor = false;
	bool txmsg = false;
	bool errIndicator = false;
	bool syncFrame = false;
	uint16_t rfu2 = 0;

	size_t roundNextMultiple(size_t x, size_t y) const {
		if(y==0) {
			return 0;
		}
		else if(x%y == 0) {
			return x;
		}

		return x + y - (x%y);
	}

	size_t getChannelIndex(A2BDirection dir, uint8_t channel) const {
		size_t channelIndex = 2 * ((size_t)channel);
		
		if(dir == A2BDirection::Upstream) {
			channelIndex++;
		}

		return channelIndex; 
	}
};

}

#endif // __cplusplus

#endif