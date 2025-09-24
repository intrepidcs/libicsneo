#include "icsneo/disk/vsa/vsa0c.h"

using namespace icsneo;

VSA0C::VSA0C(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA0C);
	captureBitfield = *reinterpret_cast<uint16_t*>(recordBytes + 2);
	audioPreamble = recordBytes[4];
	audioHeader = recordBytes[5];
	pcmData.insert(pcmData.end(), recordBytes + 6, recordBytes + 20);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 20);
	vNetBitfield = *reinterpret_cast<VSA0C::VNet*>(recordBytes + 28);
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 30);
	doChecksum(recordBytes);
}

void VSA0C::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}