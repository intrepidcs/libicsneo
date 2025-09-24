#include "icsneo/disk/vsa/vsa6a.h"
#include "icsneo/disk/diskdriver.h"

using namespace icsneo;

static constexpr auto PayloadOffset = 56;
static constexpr auto PayloadSize = 452;
static constexpr auto TimestampOffset = 48;
static constexpr auto TimestampSize = 8;

VSA6A::VSA6A(uint8_t* const recordBytes)
	: VSA()
{
	setType(VSA::Type::AA6A);

	sequenceNum = *reinterpret_cast<uint32_t*>(recordBytes + 34);
	totalSectors = *reinterpret_cast<uint32_t*>(recordBytes + 38);
	reserved = *reinterpret_cast<uint32_t*>(recordBytes + 42);
	timestamp = *reinterpret_cast<uint64_t*>(recordBytes + 46);
	timestampSum = *reinterpret_cast<uint16_t*>(recordBytes + 54);
	data.insert(data.end(), recordBytes + 56, recordBytes + 508);
	checksum = *reinterpret_cast<uint32_t*>(recordBytes + 508);
	doChecksum(recordBytes);
}

void VSA6A::doChecksum(uint8_t* recordBytes)
{
	uint32_t sum = 0;
	for(size_t i = PayloadOffset; i < PayloadOffset+ PayloadSize; i++) {
		sum += recordBytes[i];
	}
	uint16_t tSum = 0;
	for(size_t i = TimestampOffset; i < TimestampOffset + TimestampSize; i++) {
		tSum += recordBytes[i];
	}
	setChecksumFailed(sum != checksum || tSum != timestampSum);
}