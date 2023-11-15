#include "icsneo/disk/vsa/vsa08.h"

using namespace icsneo;

VSA08::VSA08(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA08);
	troubleSramCount.insert(troubleSramCount.end(), recordBytes + 2, recordBytes + 6);
	troubleSectors.insert(troubleSectors.end(), reinterpret_cast<uint32_t*>(recordBytes + 6), reinterpret_cast<uint32_t*>(recordBytes + 20));
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 22) & UINT63_MAX;
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 30);
	doChecksum(recordBytes);
}

void VSA08::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}