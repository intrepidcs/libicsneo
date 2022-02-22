#include "icsneo/communication/packet/iso9141packet.h"
#include "icsneo/communication/packetizer.h"
#include <algorithm>

using namespace icsneo;

bool HardwareISO9141Packet::EncodeFromMessage(const ISO9141Message& message, std::vector<uint8_t>& bytestream,
	const device_eventhandler_t& report, const Packetizer& packetizer)
{
	size_t bytesToSend = message.data.size();
	if (message.isInit || message.isBreak)
		bytesToSend = 0;

	if(bytesToSend > 4200) {
		report(APIEvent::Type::MessageMaxLengthExceeded, APIEvent::Severity::Error);
		return false; // Too much data for the protocol
	}

	bytestream.clear();

	std::vector<uint8_t> packet;
	packet.reserve(16);

	size_t currentStart = 0;
	do {
		const bool firstPacket = currentStart == 0;
		const uint8_t maxSize = (firstPacket ? 9 : 12);
		uint8_t currentSize = maxSize;
		if(bytesToSend - currentStart < maxSize)
			currentSize = (uint8_t)(bytesToSend - currentStart);

		packet.insert(packet.begin(), {
			(uint8_t)Network::NetID::RED, // 0x0C for long message
			(uint8_t)0, // Size, little endian 16-bit, filled later
			(uint8_t)0,
			(uint8_t)message.network.getNetID(), // NetID, little endian 16-bit
			(uint8_t)(uint16_t(message.network.getNetID()) >> 8)
		});
		packet.push_back(uint8_t(message.network.getNetID()) + uint8_t((currentSize + (firstPacket ? 6 : 3)) << 4));
		packet.push_back(uint8_t(currentSize + (firstPacket ? 5 : 2)));
		if(bytesToSend - currentStart > maxSize) // More packets are coming
			packet.back() |= 0x40;

		if(firstPacket) {
			if(message.isInit)
				packet.back() |= 0x80;
			if(message.isBreak)
				packet.back() |= 0x20;
		}

		// Two bytes for Description ID, big endian
		packet.insert(packet.end(), { uint8_t(message.description >> 8), uint8_t(message.description) });

		// If we're the first packet and not init/break only, we should put the header in
		if(firstPacket && !message.isInit && !message.isBreak)
			packet.insert(packet.end(), message.header.begin(), message.header.end());

		// Now the data
		auto dataIt = message.data.begin() + currentStart;
		if(currentSize)
			packet.insert(packet.end(), dataIt, dataIt + currentSize);

		// Advance for the next packet
		currentStart += currentSize;

		const uint16_t size = uint16_t(packet.size()) + 2;
		packet[1] = uint8_t(size & 0xFF);
		packet[2] = uint8_t((size >> 8) & 0xFF);

		packetizer.packetWrap(packet, false);
		bytestream.insert(bytestream.end(), packet.begin(), packet.end());
		packet.clear();
	} while(currentStart < bytesToSend);
	return true;
}

std::shared_ptr<ISO9141Message> HardwareISO9141Packet::Decoder::decodeToMessage(const std::vector<uint8_t>& bytestream) {
	const HardwareISO9141Packet& packet = *reinterpret_cast<const HardwareISO9141Packet*>(bytestream.data());

	if(!mMsg) {
		mMsg = std::make_shared<ISO9141Message>();
		mGotPackets = 0;
	}

	mGotPackets++;

	const bool morePacketsComing = packet.c3.frm == 0;
	const uint8_t bytesInCurrentMessage = packet.c3.len;
	if(mMsg->data.size() + bytesInCurrentMessage > 500) {
		mMsg.reset();
		return std::shared_ptr<ISO9141Message>();
	}

	// This timestamp is raw off the device (in timestampResolution increments)
	// Decoder will fix as it has information about the timestampResolution increments
	mMsg->timestamp = packet.timestamp.TS;

	auto* dataStart = packet.data;
	if(mGotPackets == 1) {
		// Header
		if(bytesInCurrentMessage < 3) {
			mMsg.reset(); // We don't have the header for some reason
			return std::shared_ptr<ISO9141Message>();
		}

		std::copy(packet.data, packet.data + 3, mMsg->header.begin());
		dataStart += 3;
	}

	// Data
	mMsg->data.insert(mMsg->data.end(), dataStart, packet.data + (bytesInCurrentMessage > 8 ? 8 : bytesInCurrentMessage));
	if(bytesInCurrentMessage > 8)
		mMsg->data.push_back(packet.c1.d8);
	if(bytesInCurrentMessage > 9)
		mMsg->data.push_back(packet.c2.d9);
	if(bytesInCurrentMessage > 10)
		mMsg->data.push_back(packet.c2.d10);
	if(bytesInCurrentMessage > 11)
		mMsg->data.push_back(packet.c3.d11);

	if(morePacketsComing)
		return std::shared_ptr<ISO9141Message>();

	mMsg->transmitted = packet.c1.tx;
	mMsg->isInit = packet.c3.init;
	mMsg->framingError = packet.c1.options & 0x1;
	mMsg->overflowError = packet.c1.options & 0x2;
	mMsg->parityError = packet.c1.options & 0x4;
	mMsg->rxTimeoutError = packet.c1.options & 0x8;
	mMsg->description = packet.stats;

	auto ret = mMsg;
	mMsg.reset();
	return ret;
}