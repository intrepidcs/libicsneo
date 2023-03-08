#include "icsneo/communication/message/callback/streamoutput/a2bwavoutput.h"
#include "icsneo/device/tree/rada2b/rada2b.h"
#include "icsneo/icsneocpp.h"

namespace icsneo {

void A2BWAVOutput::writeHeader(const std::shared_ptr<A2BMessage>& firstMsg) const {

	WaveFileHeader header = WaveFileHeader(2 * firstMsg->getNumChannels(), wavSampleRate, firstMsg->getBitDepth());
	header.write(stream);
	streamStartPos = static_cast<uint32_t>(stream->tellp());
	
}

bool A2BWAVOutput::callIfMatch(const std::shared_ptr<Message>& message) const {
	
	if(closed) {
		return false;
	}

	if(message->type != Message::Type::Frame) {
		return false;
	}

	const auto& frame = std::static_pointer_cast<Frame>(message);

	if(frame->network.getType() != Network::Type::A2B)
		return false;

	const auto& a2bmsg = std::static_pointer_cast<A2BMessage>(frame);

	if(firstMessageFlag) {
		writeHeader(a2bmsg);
		firstMessageFlag = false;
	}

	// Might need to readd this block of code later if sample alignment fix is necessary
	/*
	std::streamsize bps = (std::streamsize)a2bmsg->getBytesPerSample();
	for(size_t i=0; i<a2bmsg->getNumSamples(); i++) {
		A2BPCMSample samp = *(a2bmsg->getSample(i));
		write((void*)&samp, bps);
	}
	*/

	write((void*)a2bmsg->getAudioBuffer(), a2bmsg->getAudioBufferSize());

	return true;
}

void A2BWAVOutput::close() const {
	if(closed) {
		return;
	}

	uint32_t streamEndPos = static_cast<uint32_t>(stream->tellp());

	uint32_t subChunk2Size = streamEndPos - streamStartPos;
	uint32_t chunkSize = streamEndPos - 8;

	stream->seekp(streamStartPos - 4);
	write((void*)&subChunk2Size, 4);
	stream->seekp(4, std::ios::beg);
	write((void*)&chunkSize, 4);

	closed = true;
}

}