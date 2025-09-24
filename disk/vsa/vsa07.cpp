#include "icsneo/disk/vsa/vsa07.h"

using namespace icsneo;

VSA07::VSA07(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA07);
	lastSector = *reinterpret_cast<uint32_t*>(recordBytes + 2);
	currentSector = *reinterpret_cast<uint32_t*>(recordBytes + 6);
	reserved.insert(reserved.end(), recordBytes + 10, recordBytes + 22);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 22);
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 30);
	doChecksum(recordBytes);
}

void VSA07::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}