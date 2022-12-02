#include "icsneo/communication/message/callback/streamoutput/a2bwavoutput.h"

namespace icsneo
{

void A2BWAVOutput::writeHeader(const std::shared_ptr<A2BMessage>& firstMsg) const {

	WaveFileHeader header = WaveFileHeader(2 * firstMsg->getNumChannels(), wavSampleRate, firstMsg->getBitDepth());
	header.write(stream);
	streamStartPos = static_cast<uint32_t>(stream->tellp());
}

bool A2BWAVOutput::callIfMatch(const std::shared_ptr<Message>& message) const {
	if(closed)
	{
		return false;
	}

	if(message->type != Message::Type::Frame)
		return false;

	const auto& frame = std::static_pointer_cast<Frame>(message);

	if(frame->network.getType() != Network::Type::A2B)
		return false;

	const auto& a2bmsg = std::static_pointer_cast<A2BMessage>(frame);

	if(firstMessageFlag) {
		writeHeader(a2bmsg);
		firstMessageFlag = false;
	}

	if(!writeSamples(a2bmsg, A2BMessage::A2BDirection::DownStream)) {
		close();

		return false;
	}

	if(!writeSamples(a2bmsg, A2BMessage::A2BDirection::UpStream)) {
		close();

		return false;
	}

	return true;
}

void A2BWAVOutput::close() const
{
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

bool A2BWAVOutput::writeSamples(const std::shared_ptr<A2BMessage>& msg, A2BMessage::A2BDirection dir) const
{
	uint8_t numChannels = msg->getNumChannels();

	uint8_t channel = 0;
	uint32_t frame = 0;
	uint8_t bitDepth = msg->getBitDepth();

	while(true) {
		auto sample = msg->getSample(dir, channel, frame);

		if(!sample) {
			if(channel == 0) {
				break;
			}

			return false;
		}

		uint32_t audioSample = sample.value() >> (32 - bitDepth);

		write((void*)(&audioSample), A2BPCM_SAMPLE_SIZE);

		channel = (channel + 1) % numChannels;
		if(channel == 0) {
			frame++;
		}
	}

	return true;
}

}