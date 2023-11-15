#include "icsneo/disk/vsa/vsa03.h"

using namespace icsneo;

VSA03::VSA03(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA03);
	eventType = static_cast<EventType>(*reinterpret_cast<uint16_t*>(recordBytes + 2));
	eventData = *reinterpret_cast<uint16_t*>(recordBytes + 4);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 6) & UINT63_MAX;
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 14);
	doChecksum(recordBytes);
}

void VSA03::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 7; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}