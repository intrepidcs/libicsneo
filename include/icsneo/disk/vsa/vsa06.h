#ifndef __VSA06_H__
#define __VSA06_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

#include <vector>

namespace icsneo {

/**
 * Class that holds data for an internal/debug VSA record
 */
class VSA06 : public VSA {
public:
	/**
	 * Constructor to convert bytestream into internal/debug record
	 *
	 * @param bytes Bytestream to parse into internal/debug record
	 */
	VSA06(uint8_t* const bytes);

	/**
	 * Get the timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 *
	 * @return The timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 */
	uint64_t getTimestamp() override { return timestamp & UINT63_MAX; }

private:
	/**
	 * Perform the checksum on this record
	 *
	 * @param bytes Bytestream to test against the checksum
	 */
	void doChecksum(uint8_t* bytes) override;

	std::vector<uint32_t> savedSectors; // Unknown
	uint16_t error; // Unknown
	uint16_t savedSectorsHigh; // Unknown
	uint64_t timestamp; // Timestamp for this record in 25 nanosecond ticks since January 1, 2007
	uint16_t checksum; // Sum of the previous 15 words
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA06_H__