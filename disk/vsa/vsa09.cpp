#include "icsneo/disk/vsa/vsa09.h"

using namespace icsneo;

VSA09::VSA09(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA09);
	serialNumber = *reinterpret_cast<uint32_t*>(recordBytes + 2);
	firmwareMajorVersion = recordBytes[6];
	firmwareMinorVersion = recordBytes[7];
	manufactureMajorRevision = recordBytes[8];
	manufactureMinorRevision = recordBytes[9];
	bootloaderMajorVersion = recordBytes[10];
	bootloaderMinorVersion = recordBytes[11];
	reserved0.insert(reserved0.end(), recordBytes + 12, recordBytes + 18);
	hardwareID = static_cast<HardwareID>(recordBytes[18]);
	reserved1.insert(reserved1.end(), recordBytes + 19, recordBytes + 22);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 22) & UINT63_MAX;
	checksum = *reinterpret_cast<uint16_t*>(recordBytes + 30);
	doChecksum(recordBytes);
}

void VSA09::doChecksum(uint8_t* recordBytes)
{
	uint16_t* words = reinterpret_cast<uint16_t*>(recordBytes);
	uint16_t sum = 0;
	for(size_t i = 0; i < 15; i++) {
		sum += words[i];
	}
	setChecksumFailed(sum != checksum);
}