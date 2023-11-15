#include "icsneo/disk/vsa/vsa02.h"

using namespace icsneo;

VSA02::VSA02(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA02);
	constantIndex = *reinterpret_cast<uint16_t*>(recordBytes + 2);
	flags = *reinterpret_cast<Flags*>(recordBytes + 4);
	pieceCount = recordBytes[5];
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 6) & UINT63_MAX;
	samples.insert(samples.end(), recordBytes + 14, recordBytes + 30);
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 30);
	doChecksum(recordBytes);
}

void VSA02::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}