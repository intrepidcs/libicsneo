#include "icsneo/disk/vsa/vsa0b.h"

#include <algorithm>

using namespace icsneo;

static constexpr auto PayloadOffset = 4;

VSA0B::VSA0B(uint8_t* const recordBytes)
	: VSAMessage(recordBytes + PayloadOffset, CoreMiniPayloadSize, static_cast<Network::CoreMini>(recordBytes[29]))
{
	setType(VSA::Type::AA0B);
	captureBitfield = reinterpret_cast<uint16_t*>(recordBytes)[1];
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 20);
	reserved = recordBytes[28];
	checksum = reinterpret_cast<uint16_t*>(recordBytes)[15];
	doChecksum(recordBytes);
}

void VSA0B::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}

bool VSA0B::filter(const std::shared_ptr<VSAMessageReadFilter> filter)
{
	if((filter->captureBitfield != captureBitfield && filter->captureBitfield != UINT16_MAX) ||
		getICSTimestampFromTimepoint(filter->readRange.first) > getTimestamp() ||
		getICSTimestampFromTimepoint(filter->readRange.second) < getTimestamp()) {
		return false;
	}
	return true;
}