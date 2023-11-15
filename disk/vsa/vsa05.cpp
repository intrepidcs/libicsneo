#include "icsneo/disk/vsa/vsa05.h"

using namespace icsneo;

VSA05::VSA05(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA05);
	errorType = static_cast<ErrorType>(*reinterpret_cast<uint16_t*>(recordBytes + 2));
	errorNetwork = *reinterpret_cast<uint16_t*>(recordBytes + 4);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 6) & UINT63_MAX;
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 14);
}

void VSA05::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 7; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}