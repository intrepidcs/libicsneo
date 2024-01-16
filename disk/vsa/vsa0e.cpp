#include "icsneo/disk/vsa/vsa0e.h"

#include <algorithm>

using namespace icsneo;

static constexpr auto FirstPayloadOffset = 10;
static constexpr auto FirstPayloadSize = 10;
static constexpr auto ConsecutivePayloadOffset = 4;
static constexpr auto LastPayloadSize = 24;
static constexpr auto OtherPayloadSize = 28;

// Parent class functions

VSA0E::VSA0E(uint8_t* const recordBytes, uint8_t* const messageBytes, size_t numBytes, uint32_t& runningChecksum, Network::CoreMini networkId)
	: VSAExtendedMessage(messageBytes, numBytes, networkId)
{
	static constexpr auto DWordSize = 4;
	setType(VSA::Type::AA0E);
	setIndex(static_cast<uint16_t>(recordBytes[2]));
	setSequenceNum(static_cast<uint16_t>(recordBytes[3]));
	if(getIndex() == 0) {
		runningChecksum = (static_cast<uint32_t>(payload[0]) << 16) | (static_cast<uint32_t>(payload[1]) << 24);
		uint32_t* dwords = reinterpret_cast<uint32_t*>(payload.data() + 2);
		for(size_t i = 0; i < (payload.size() - 2) / DWordSize; i++) {
			runningChecksum += dwords[i];
		}
	} else {
		uint32_t* dwords = reinterpret_cast<uint32_t*>(payload.data());
		for(size_t i = 0; i < payload.size() / DWordSize; i++) {
			runningChecksum += dwords[i];
		}
	}
}

// First Record Functions

VSA0EFirst::VSA0EFirst(uint8_t* const recordBytes, uint32_t& runningChecksum)
	: VSA0E(recordBytes, recordBytes + FirstPayloadOffset, FirstPayloadSize, runningChecksum,
			static_cast<Network::CoreMini>(*reinterpret_cast<uint16_t*>(recordBytes + 28)))
{
	captureBitfield = *reinterpret_cast<uint16_t*>(recordBytes + 4);
	setRecordCount(*reinterpret_cast<uint32_t*>(recordBytes + 6));
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 20) & UINT63_MAX;
	timestampIsExtended = (bool)(*reinterpret_cast<uint64_t*>(recordBytes + 20) & (0x8000000000000000));
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 30);
	doChecksum(recordBytes);
}

void VSA0EFirst::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}

void VSA0EFirst::reservePacketData(std::shared_ptr<Packet>& packet) const
{
	uint32_t numMessageBytes = (getRecordCount() - 2) * OtherPayloadSize + FirstPayloadSize + LastPayloadSize;
	packet->data.reserve(numMessageBytes);
}

bool VSA0EFirst::filter(const std::shared_ptr<VSAMessageReadFilter> filter)
{
	if((filter->captureBitfield != captureBitfield && filter->captureBitfield != UINT16_MAX) ||
		getICSTimestampFromTimepoint(filter->readRange.first) > timestamp ||
		getICSTimestampFromTimepoint(filter->readRange.second) < timestamp) {
		return false;
	}
	return true;
}

void VSA0EFirst::reorderPayload(std::vector<uint8_t>& secondPayload)
{
	std::vector<uint8_t> tempPayload;
	tempPayload.insert(tempPayload.end(), payload.begin(), payload.end());
	tempPayload.insert(tempPayload.end(), secondPayload.begin(), secondPayload.begin() + 6);
	uint8_t* timestampBytes = reinterpret_cast<uint8_t*>(&timestamp);
	if(timestampIsExtended) {
		timestampBytes[7] += 0x80;
	}
	tempPayload.insert(tempPayload.end(), timestampBytes, timestampBytes + 8);
	tempPayload.insert(tempPayload.end(), secondPayload.begin() + 6, secondPayload.end());
	payload.clear();
	secondPayload.clear();
	payload.insert(payload.end(), tempPayload.begin(), tempPayload.begin() + 10); // This is done because the capacity of payload is already 10
	secondPayload.insert(secondPayload.end(), tempPayload.begin() + 10, tempPayload.end());
}

// Consecutive Record Functions

VSA0EConsecutive::VSA0EConsecutive(uint8_t* const recordBytes, uint32_t& runningChecksum, std::shared_ptr<VSA0EFirst> first, bool isLastRecord)
	: VSA0E(recordBytes, recordBytes + ConsecutivePayloadOffset, isLastRecord ? LastPayloadSize : OtherPayloadSize, runningChecksum)
{
	this->first = first;
	calculatedChecksum = runningChecksum;
	if(getIndex() == 1) {
		first->reorderPayload(payload);
	} else if(isLastRecord) {
		recordChecksum = *reinterpret_cast<uint32_t*>(recordBytes + 28);
		doChecksum(recordBytes);
	} else {
		setChecksumFailed(first->getChecksumFailed());
	}
	setRecordCount(first->getRecordCount());
}

void VSA0EConsecutive::doChecksum(uint8_t* recordBytes)
{
	setChecksumFailed(recordBytes && calculatedChecksum != recordChecksum);
}