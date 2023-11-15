#include "icsneo/disk/vsa/vsa06.h"

using namespace icsneo;

VSA06::VSA06(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA06);
	savedSectors.insert(savedSectors.end(), reinterpret_cast<uint32_t*>(recordBytes + 2), reinterpret_cast<uint32_t*>(recordBytes + 18));
	error = *reinterpret_cast<uint16_t*>(recordBytes + 18);
	savedSectorsHigh = *reinterpret_cast<uint16_t*>(recordBytes + 20);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 22) & UINT63_MAX;
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 30);
	doChecksum(recordBytes);
}

void VSA06::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}