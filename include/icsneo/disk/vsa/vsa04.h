#ifndef __VSA04_H__
#define __VSA04_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

namespace icsneo {

/**
 * Class that contains a partition info record
 */
class VSA04 : public VSA {
public:
	/**
	 * Constructor that created partition info record from bytestream
	 *
	 * @param bytes Bytestream to create this record from
	 */
	VSA04(uint8_t* const bytes);

	/**
	 * Get the timestamp of this record in 25 nanosecond ticks since January 1, 2007
	 *
	 * @return Timestamp of this record in 25 nanosecond ticks since January 1, 2007
	 */
	uint64_t getTimestamp() override { return timestamp & UINT63_MAX; }

private:
	/**
	 * Perform the checksum for this record
	 *
	 * @param bytes Bytestream to check against the checksum
	 */
	void doChecksum(uint8_t* bytes) override;

	struct Flags {
		bool invalidRequestDetected : 1; // Indicates if an invalid request was detected
		uint16_t reserved : 15; // Empty flag bits
	} flags; // Mostly empty field for flags
	uint16_t partitionIndex; // The index of the partition containing this record
	uint64_t timestamp; // Timestamp of this record in 25 nanosecond ticks since January 1, 2007
	uint16_t checksum; // Sum of the previous 7 words
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA04_H__