#include "icsneo/disk/vsa/vsa09.h"

using namespace icsneo;

VSA09::VSA09(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA09);
	serialNumber = *reinterpret_cast<uint32_t*>(recordBytes + 2);
	// Each version is a little-endian uint16_t (major << 8 | minor), so the minor byte comes first
	firmwareMinorVersion = recordBytes[6];
	firmwareMajorVersion = recordBytes[7];
	manufactureMinorRevision = recordBytes[8];
	manufactureMajorRevision = recordBytes[9];
	bootloaderMinorVersion = recordBytes[10];
	bootloaderMajorVersion = recordBytes[11];
	reserved0.insert(reserved0.end(), recordBytes + 12, recordBytes + 18);
	hardwareID = static_cast<HardwareID>(recordBytes[18]);
	reserved1.insert(reserved1.end(), recordBytes + 19, recordBytes + 22);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 22);
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