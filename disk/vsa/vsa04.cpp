#include "icsneo/disk/vsa/vsa04.h"

using namespace icsneo;

VSA04::VSA04(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA04);
	flags = *reinterpret_cast<Flags*>(recordBytes + 2);
	partitionIndex = *reinterpret_cast<uint16_t*>(recordBytes + 4);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 6) & UINT63_MAX;
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 14);
	doChecksum(recordBytes);
}

void VSA04::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 7; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}