#include "icsneo/disk/vsa/vsa0f.h"

#include <algorithm>

using namespace icsneo;

static constexpr auto FirstPayloadOffset = 18;
static constexpr auto FirstPayloadSize = 14;
static constexpr auto LastPayloadSize = 24;
static constexpr auto OtherPayloadSize = 28;

// Parent class functions

VSA0F::VSA0F(uint8_t* const recordBytes, uint8_t* const messageBytes, size_t numBytes, uint32_t& runningChecksum, Network::CoreMini networkId)
	: VSAExtendedMessage(messageBytes, numBytes, networkId)
{
	static constexpr auto DWordSize = 4;
	setType(VSA::Type::AA0F);
	setIndex(*reinterpret_cast<uint16_t*>(recordBytes + 2) & 0x01FFu);
	setSequenceNum((*reinterpret_cast<uint16_t*>(recordBytes + 2) & 0xFE00u) >> 9);
	if(getIndex() == 0) {
		runningChecksum = (static_cast<uint32_t>(payload[0]) << 16) | (static_cast<uint32_t>(payload[1]) << 24);
		uint32_t* dwords = reinterpret_cast<uint32_t*>(payload.data() + 2);
		for (size_t i = 0; i < (payload.size() - 2) / DWordSize; i++) {
			runningChecksum += dwords[i];
		}
	} else {
		uint32_t* dwords = reinterpret_cast<uint32_t*>(recordBytes);
		for (size_t i = 0; i < 8; i++) {
			runningChecksum += dwords[i];
		}
	}
}

// First Record Functions

VSA0FFirst::VSA0FFirst(uint8_t* const recordBytes, uint32_t& runningChecksum)
	: VSA0F(recordBytes, recordBytes + FirstPayloadOffset, FirstPayloadSize, runningChecksum)
{
	captureBitfield = *reinterpret_cast<uint16_t*>(recordBytes + 4);
	uint16_t byteCount = *reinterpret_cast<uint16_t*>(recordBytes + 6);
	uint16_t recordCount;
	if(byteCount <= FirstPayloadSize) {
		recordCount = 1;
	} else if(byteCount <= FirstPayloadSize + LastPayloadSize) {
		recordCount = 2;
	} else {
		byteCount -= FirstPayloadSize + LastPayloadSize;
		recordCount = 2 + byteCount / OtherPayloadSize;
		if (byteCount % OtherPayloadSize != 0) {
			recordCount += 1;
		}
	}
	setRecordCount(recordCount);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 8) & UINT63_MAX;
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 16);
	doChecksum(recordBytes);
	// Network ID is not found in first record for AA0F
	// Only the subsequent records have the Network ID in the payload
}

void VSA0FFirst::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for (size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}

void VSA0FFirst::reservePacketData(std::shared_ptr<Packet>& packet) const
{
	uint32_t numMessageBytes = (getRecordCount() - 2) * OtherPayloadSize + FirstPayloadSize + LastPayloadSize;
	packet->data.reserve(numMessageBytes);
}

bool VSA0FFirst::filter(const std::shared_ptr<VSAMessageReadFilter> filter)
{
	if(filter->captureBitfield != captureBitfield ||
		getICSTimestampFromTimepoint(filter->readRange.first) > timestamp ||
		getICSTimestampFromTimepoint(filter->readRange.second) < timestamp) {
		return false;
	}
	return true;
}

// Consecutive Record Functions

VSA0FConsecutive::VSA0FConsecutive(uint8_t* const recordBytes, uint32_t& runningChecksum, std::shared_ptr<VSA0FFirst> first, bool isLastRecord)
	: VSA0F(recordBytes, recordBytes + 4, isLastRecord ? LastPayloadSize : OtherPayloadSize, runningChecksum)
{
	this->first = first;
	calculatedChecksum = runningChecksum;
	if(isLastRecord) {
		doChecksum(recordBytes);
	} else {
		network = Network(static_cast<Network::CoreMini>(*reinterpret_cast<uint16_t*>(recordBytes + 28))); // Network ID is stored in 25th and 26th recordBytes of payload
	}
	setRecordCount(first->getRecordCount());
}

void VSA0FConsecutive::doChecksum(uint8_t* recordBytes)
{
	setChecksumFailed(recordBytes && calculatedChecksum != 0);
}