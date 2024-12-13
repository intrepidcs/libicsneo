#include "icsneo/communication/packet/ethernetpacket.h"
#include <cstring> // memcpy
#include <iostream>

using namespace icsneo;

std::shared_ptr<EthernetMessage> HardwareEthernetPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream, const device_eventhandler_t& report) {
	const HardwareEthernetPacket* packet = (const HardwareEthernetPacket*)((const void*)bytestream.data());
	const uint16_t* rawWords = (const uint16_t*)bytestream.data();
	
	// Make sure we have enough to read the packet length first
	if(bytestream.size() < sizeof(HardwareEthernetPacket))
		return nullptr;

	// packet->Length will also encompass the two uint16_t's at the end of the struct, make sure that at least they are here
	if(packet->Length < 4)
		return nullptr;
	
	const size_t fcsSize = packet->header.FCS_AVAIL ? 4 : 0;
	const size_t bytestreamExpectedSize = sizeof(HardwareEthernetPacket) + packet->Length;
	const size_t bytestreamActualSize = bytestream.size();
	if(bytestreamActualSize < bytestreamExpectedSize)
		return nullptr;

	// Check for oversized packets, noting that some devices will send an extra byte to have an even number of bytes
	if(bytestreamActualSize > bytestreamExpectedSize + 1)
		report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::EventWarning);

	auto messagePtr = std::make_shared<EthernetMessage>();
	EthernetMessage& message = *messagePtr;

	message.transmitted = packet->eid.TXMSG;
	if(message.transmitted)
		message.description = packet->stats;

	message.preemptionEnabled = packet->header.PREEMPTION_ENABLED;
	if(message.preemptionEnabled)
		message.preemptionFlags = (uint8_t)((rawWords[0] & 0x03F8) >> 4);

	message.frameTooShort = packet->header.RUNT_FRAME;
	if(message.frameTooShort)
		message.error = true;

	// This timestamp is raw off the device (in timestampResolution increments)
	// Decoder will fix as it has information about the timestampResolution increments
	message.timestamp = packet->timestamp.TS;

	const std::vector<uint8_t>::const_iterator databegin = bytestream.begin() + sizeof(HardwareEthernetPacket);
	const std::vector<uint8_t>::const_iterator dataend = databegin + packet->Length - fcsSize;
	message.data.insert(message.data.begin(), databegin, dataend);
	
	if(fcsSize) {
		uint32_t& fcs = message.fcs.emplace();
		std::copy(dataend, dataend + fcsSize, (uint8_t*)&fcs);
	}

	return messagePtr;
}

bool HardwareEthernetPacket::EncodeFromMessage(const EthernetMessage& message, std::vector<uint8_t>& bytestream, const device_eventhandler_t&) {
	const size_t unpaddedSize = message.data.size();
	size_t paddedSize = unpaddedSize;
	uint16_t description = message.description;

	if(!message.noPadding && unpaddedSize < 60)
		paddedSize = 60; // Pad out short messages

	size_t sizeWithHeader = paddedSize + 4; // DescriptionID and Padded Count

	// Description ID Most Significant bit is used to identify preemption frames
	if(description & 0x8000)
		return false;

	if(message.preemptionEnabled) {
		sizeWithHeader++; // Make space for the preemption flags
		description |= 0x8000;
	}
	
	bytestream.reserve(sizeWithHeader + 8); // Also reserve space for the bytes we'll use later on
	bytestream.resize(sizeWithHeader);
	size_t index = 0;

	// Padded size, little endian
	bytestream[index++] = uint8_t(paddedSize);
	bytestream[index++] = uint8_t(paddedSize >> 8);

	// Description ID, big endian
	bytestream[index++] = uint8_t(description >> 8);
	bytestream[index++] = uint8_t(description);

	// The header is one byte larger if preemption is enabled, shifting the data
	if(message.preemptionEnabled)
		bytestream[index++] = message.preemptionFlags;

	// We only copy in the unpadded size, the rest will be 0
	memcpy(bytestream.data() + index, message.data.data(), unpaddedSize);

	return true;
}