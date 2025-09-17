#include "icsneo/communication/packet/spipacket.h"
#include "icsneo/communication/command.h"
#include <cstring>
#include <vector>

using namespace icsneo;

static size_t SPISubHeaderLength = 5u;

std::shared_ptr<Message> HardwareSPIPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	if(bytestream.size() < sizeof(HardwareSPIPacket)) {
		return nullptr;
	}
	const HardwareSPIPacket* packet = (const HardwareSPIPacket*)bytestream.data();
	size_t totalPackedLength = static_cast<size_t>(bytestream.size()) - sizeof(HardwareSPIPacket); // First 28 bytes are message header.
	if(totalPackedLength < SPISubHeaderLength) {
		return nullptr;
	}
	
	const uint8_t* bytes = bytestream.data() + sizeof(HardwareSPIPacket);
	std::shared_ptr<SPIMessage> msg = std::make_shared<SPIMessage>();
	msg->direction = static_cast<SPIMessage::Direction>(bytes[0]);
	msg->address = *reinterpret_cast<const uint16_t*>(&bytes[1]);
	msg->mms = bytes[3];
	msg->stats = packet->stats;
	msg->timestamp = packet->timestamp.TS;
	
	size_t numWords = (totalPackedLength - SPISubHeaderLength) / 4;
	msg->payload.reserve(numWords);
	for(size_t offset = SPISubHeaderLength; offset < totalPackedLength; offset += 4) {
		msg->payload.push_back(*reinterpret_cast<const uint32_t*>(bytes + offset));
	}
	return msg;
}

bool HardwareSPIPacket::EncodeFromMessage(const SPIMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t& /*report*/) {
	// Payload length is everything excluding cmdHeader (note at the beginning there is an offset of 2)
	uint16_t payloadLength = static_cast<uint16_t>(
		2 + 
		sizeof(HardwareSPIPacket) + 
		SPISubHeaderLength + 
		message.payload.size() * sizeof(uint32_t)
	);
	if(payloadLength % 2) {
		// Pad payload to even number
		payloadLength++;
	}
	// +1 for AA, another +1 for firmware nuance
	uint16_t fullSize = 1 + sizeof(ExtendedCommandHeader) + payloadLength + 1;
	uint16_t unwrappedSize = sizeof(ExtendedCommandHeader) + payloadLength; // fullSize without AA and firmware nuance

	bytestream.resize(unwrappedSize, 0);
	uint32_t offset = 0;
	auto* cmdHeader = reinterpret_cast<ExtendedCommandHeader*>(bytestream.data() + offset);
	cmdHeader->netid = static_cast<uint8_t>(Network::NetID::Main51);
	cmdHeader->fullLength = fullSize;
	cmdHeader->command = static_cast<uint8_t>(Command::Extended);
	cmdHeader->extendedCommand = static_cast<uint16_t>(ExtendedCommand::TransmitCoreminiMessage);
	cmdHeader->payloadLength = payloadLength;
	
	offset += sizeof(ExtendedCommandHeader) + 2; // Offset of 2 between header and packet
	auto* packet = reinterpret_cast<HardwareSPIPacket*>(bytestream.data() + offset);
	packet->header.frameLength = static_cast<uint16_t>(SPISubHeaderLength + message.payload.size() * sizeof(uint32_t));
	packet->networkID = static_cast<uint16_t>(message.network.getNetID());
	packet->length = packet->header.frameLength;
	packet->timestamp.IsExtended = 1;

	offset += sizeof(HardwareSPIPacket);
	// Write the sub header details
	bytestream[offset++] = static_cast<uint8_t>(message.direction);
	bytestream[offset++] = static_cast<uint8_t>(message.address & 0xFF);
	bytestream[offset++] = static_cast<uint8_t>((message.address >> 8) & 0xFF);
	bytestream[offset++] = static_cast<uint8_t>(message.mms);
	bytestream[offset++] = static_cast<uint8_t>(message.payload.size());

	// Write the words
	for(uint32_t word : message.payload) {
		*reinterpret_cast<uint32_t*>(bytestream.data() + offset) = word;
		offset += sizeof(uint32_t);
	}
	return true;
}
