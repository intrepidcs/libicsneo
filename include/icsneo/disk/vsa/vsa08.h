#ifndef __VSA08_H__
#define __VSA08_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

namespace icsneo {

/**
 * Class to store data for a buffer information record
 */
class VSA08 : public VSA {
public:
	/**
	 * Constructor to convert a bytestream to a buffer info record
	 *
	 * @param bytes Bytestream to convert into a buffer record
	 */
	VSA08(uint8_t* const bytes);

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

	std::vector<uint8_t> troubleSramCount; // Unknown (4 bytes)
	std::vector<uint32_t> troubleSectors; // Unknown (16 bytes)
	uint64_t timestamp; // Timestamp for this record in 25 nanosecond ticks since January 1, 2007
	uint16_t checksum; // Sum of the previous 15 words
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA08_H__