#include "icsneo/disk/vsa/vsa0d.h"

#include <algorithm>

using namespace icsneo;

static constexpr auto FirstPayloadOffset = 8;
static constexpr auto FirstPayloadSize = 12;
static constexpr auto ConsecutivePayloadOffset = 4;
static constexpr auto LastPayloadSize = 24;
static constexpr auto OtherPayloadSize = 28;

// Parent class functions

VSA0D::VSA0D(uint8_t* const recordBytes, uint8_t* const messageBytes, size_t numBytes, uint32_t& runningChecksum, Network::CoreMini networkId)
	: VSAExtendedMessage(messageBytes, numBytes, networkId)
{
	static constexpr auto DWordSize = 4;
	setType(VSA::Type::AA0D);
	setIndex(*reinterpret_cast<uint16_t*>(recordBytes + 2) & 0x01FFu);
	setSequenceNum((*reinterpret_cast<uint16_t*>(recordBytes + 2) & 0xFE00u) >> 9);
	uint32_t* dwords = reinterpret_cast<uint32_t*>(payload.data());
	for(size_t i = 0; i < payload.size() / DWordSize; i++) {
		runningChecksum += dwords[i];
	}
}

// First Record Functions

VSA0DFirst::VSA0DFirst(uint8_t* const recordBytes, uint32_t& runningChecksum)
	: VSA0D(recordBytes, recordBytes + FirstPayloadOffset, FirstPayloadSize, runningChecksum, static_cast<Network::CoreMini>(recordBytes[29]))
{
	captureBitfield = *reinterpret_cast<uint16_t*>(recordBytes + 4);
	setRecordCount(*reinterpret_cast<uint16_t*>(recordBytes + 6));
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 20) & UINT63_MAX;
	vNetInfo = *reinterpret_cast<VNet*>(recordBytes + 28);
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 30);
	doChecksum(recordBytes);
	uint32_t* const timestampDWords = reinterpret_cast<uint32_t*>(timestamp);
	runningChecksum += timestampDWords[0];
	runningChecksum += timestampDWords[1];
}

void VSA0DFirst::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}

void VSA0DFirst::reservePacketData(std::shared_ptr<Packet>& packet) const
{
	uint32_t numMessageBytes = (getRecordCount() - 2) * OtherPayloadSize + FirstPayloadSize + LastPayloadSize;
	packet->data.reserve(numMessageBytes);
}

void VSA0DFirst::reorderPayload(std::vector<uint8_t>& secondPayload)
{
	std::vector<uint8_t> tempPayload;
	tempPayload.insert(tempPayload.end(), secondPayload.begin(), secondPayload.begin() + 4);
	uint8_t* timestampBytes = reinterpret_cast<uint8_t*>(&timestamp);
	if(timestampIsExtended) {
		timestampBytes[7] += 0x80;
	}
	tempPayload.insert(tempPayload.end(), timestampBytes, timestampBytes + 8);
	tempPayload.insert(tempPayload.end(), secondPayload.begin() + 4, secondPayload.end());
	payload.clear();
	secondPayload.clear();
	payload.insert(payload.end(), tempPayload.begin(), tempPayload.begin() + 12); // This is done because the capacity of payload is already 12
	secondPayload.insert(secondPayload.end(), tempPayload.begin() + 12, tempPayload.end());
}

bool VSA0DFirst::filter(const std::shared_ptr<VSAMessageReadFilter> filter)
{
	if((filter->captureBitfield != captureBitfield && filter->captureBitfield != UINT16_MAX) ||
		getICSTimestampFromTimepoint(filter->readRange.first) > timestamp ||
		getICSTimestampFromTimepoint(filter->readRange.second) < timestamp) {
		return false;
	}
	return true;
}

// Consecutive Record Functions

VSA0DConsecutive::VSA0DConsecutive(uint8_t* const recordBytes, uint32_t& runningChecksum, std::shared_ptr<VSA0DFirst> first, bool isLastRecord)
	: VSA0D(recordBytes, recordBytes + ConsecutivePayloadOffset, isLastRecord ? LastPayloadSize : OtherPayloadSize, runningChecksum)
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

void VSA0DConsecutive::doChecksum(uint8_t* recordBytes)
{
	setChecksumFailed(recordBytes && calculatedChecksum != recordChecksum);
}