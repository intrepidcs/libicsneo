#ifndef __A2BDECODER_H_
#define __A2BDECODER_H_

#ifdef __cplusplus

#include "icsneo/communication/message/callback/streamoutput/streamoutput.h"
#include "icsneo/communication/message/a2bmessage.h"
#include "icsneo/device/device.h"

namespace icsneo {

typedef uint8_t Channel;

class A2BAudioChannelMap {
public:

	A2BAudioChannelMap(uint8_t tdm);
	
	void set(Channel outChannel, A2BMessage::A2BDirection dir, Channel inChannel);
	void setAll(Channel inChannel);

	Channel get(Channel outChannel, A2BMessage::A2BDirection dir) const;

	size_t size() const;

	uint8_t getTDM() const;
	Channel& operator[](size_t idx);

	operator const std::vector<Channel>&() const;



private:
	
	size_t getChannelIndex(Channel channel, A2BMessage::A2BDirection dir) const;

	std::vector<Channel> rawMap;
};



class A2BDecoder {
public:

	A2BDecoder(
		std::unique_ptr<std::istream>&& streamOut,
		bool chSize16,
		const A2BAudioChannelMap& chMap
	);

	A2BDecoder(
		const char* filename,
		bool chSize16,
		const A2BAudioChannelMap& chMap
	);

	operator bool() const;

	std::shared_ptr<A2BMessage> decode();

	bool outputAll(std::shared_ptr<Device> &device);

	std::unique_ptr<std::istream> stream;
	private:

	void initializeFromHeader();

	uint8_t tdm;
	uint8_t audioBytesPerSample;
	uint8_t channelsInWave;
	bool channelSize16;
	A2BAudioChannelMap channelMap;

	std::vector<uint8_t> frame;
	std::vector<uint8_t> frameWave;

	bool initialized = false;

};

}

#endif // __cplusplus

#endif
