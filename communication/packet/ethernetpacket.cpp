#include "icsneo/communication/packet/ethernetpacket.h"
#include <algorithm>
#include <iostream>
#include <optional>

using namespace icsneo;

std::shared_ptr<EthernetMessage> HardwareEthernetPacket::DecodeToMessage(const std::vector<uint8_t>& bytestream) {
	const HardwareEthernetPacket* packet = (const HardwareEthernetPacket*)((const void*)bytestream.data());
	const uint16_t* rawWords = (const uint16_t*)bytestream.data();
	// Make sure we have enough to read the packet length first
	if(bytestream.size() < sizeof(HardwareEthernetPacket))
		return nullptr;
	const size_t fcsSize = packet->header.FCS_AVAIL ? 4 : 0;
	// Ensure Length is sufficient for FCS extraction to avoid invalid iterator arithmetic
	if(packet->Length < fcsSize)
		return nullptr;
	const size_t bytestreamExpectedSize = sizeof(HardwareEthernetPacket) + packet->Length;
	const size_t bytestreamActualSize = bytestream.size();
	if(bytestreamActualSize < bytestreamExpectedSize)
		return nullptr;
	auto messagePtr = std::make_shared<EthernetMessage>();
	EthernetMessage& message = *messagePtr;
	// Standard Ethernet fields
	message.transmitted = packet->eid.TXMSG;
	if(message.transmitted)
		message.description = packet->stats;
	message.frameTooShort = packet->header.RUNT_FRAME;
	message.noPadding = !packet->header.ENABLE_PADDING;
	message.fcsVerified = packet->header.FCS_VERIFIED;
	message.txAborted = packet->eid.TXAborted;
	message.crcError = packet->header.CRC_ERROR;

	if(message.frameTooShort)
		message.error = true;
	// This timestamp is raw off the device (in timestampResolution increments)
	// Decoder will fix as it has information about the timestampResolution increments
	message.timestamp = packet->timestamp.TS;

	// Ethernet Frame Preemption for TSN
	if(packet->header.PREEMPTION_ENABLED) {
		message.preemptionFlags = static_cast<uint8_t>((rawWords[0] & 0x03F8) >> 4);
	}

	// Check if this is a T1S packet and populate T1S-specific fields
	if(packet->header.T1S_ETHERNET) {
		auto& t1s = message.t1s.emplace();

		t1s.isSymbol = packet->eid.T1S_SYMBOL;
		t1s.isBurst = packet->eid.T1S_BURST;
		t1s.txCollision = packet->t1s_status.TXCollision;
		t1s.isWake = packet->t1s_status.T1SWake;
		t1s.nodeId = packet->t1s_node.T1S_NODE_ID;
		t1s.burstCount = packet->t1s_node.T1S_BURST_COUNT;
	}

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
	if(unpaddedSize == 0)
		return false;

	// Description ID Most Significant bit is used to identify preemption frames
	uint16_t description = message.description;
	if(description & 0x8000)
		return false;

	const bool preempt = message.preemptionFlags.has_value();
	// full header including parent
	const size_t headerByteCount = preempt ? 15 : 14;
	// local header for netID, description, and flags
	const size_t localHeader = preempt ? 10 : 9;
	// allocate space for fcs override
	const size_t fcsSize = message.fcs ? 4 : 0;

	if(preempt)
		description |= 0x8000;

	size_t paddedSize = unpaddedSize;
	if(!message.noPadding && unpaddedSize < 60)
		paddedSize = 60;

	// size of full payload including optional fcs
	size_t payloadSize = paddedSize + fcsSize;


	// totalBufferSize is local header + ethernet payload and option fcs
	const size_t totalBufferSize = localHeader + paddedSize + fcsSize;

	bytestream.clear();
	bytestream.reserve(totalBufferSize);

	// Header size field (little endian)
	bytestream.push_back(static_cast<uint8_t>(payloadSize & 0xFF));
	bytestream.push_back(static_cast<uint8_t>((payloadSize >> 8) & 0xFF));

	// Description (big endian)
	bytestream.push_back(static_cast<uint8_t>(description >> 8));
	bytestream.push_back(static_cast<uint8_t>(description));

	bytestream.push_back(0x00);
	bytestream.push_back(static_cast<uint8_t>(headerByteCount));

	// Network ID (little endian)
	uint16_t realID = static_cast<uint16_t>(message.network.getNetID());
	bytestream.push_back(static_cast<uint8_t>(realID & 0xFF));
	bytestream.push_back(static_cast<uint8_t>((realID >> 8) & 0xFF));

	// Flags
	constexpr uint8_t FLAG_PADDING = 0x01;
	constexpr uint8_t FLAG_FCS = 0x04;
	constexpr uint8_t FLAG_PREEMPTION = 0x08;

	uint8_t flags = 0x00;
	if(!message.noPadding) flags |= FLAG_PADDING;
	if(message.fcs) flags |= FLAG_FCS;
	if(message.preemptionFlags.has_value()) {
		flags |= FLAG_PREEMPTION;
	}

	bytestream.push_back(flags);

	if(preempt) {
		bytestream.push_back(message.preemptionFlags.value());
	}

	bytestream.insert(bytestream.end(), message.data.begin(), message.data.end());

	// Only zero-fill when we want padding
	if(!message.noPadding && unpaddedSize < 60) {
		size_t paddingNeeded = 60 - unpaddedSize;
		bytestream.insert(bytestream.end(), paddingNeeded, 0); // Zero-fill for padding
	}

	if(message.fcs) {
		uint32_t fcs = message.fcs.value();
		const uint8_t* fcsBytes = reinterpret_cast<const uint8_t*>(&fcs);
		bytestream.insert(bytestream.end(), fcsBytes, fcsBytes + sizeof(fcs));
	}

	return true;
}
