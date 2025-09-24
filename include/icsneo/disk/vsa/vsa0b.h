#ifndef __VSA0B_H__
#define __VSA0B_H__

#ifdef __cplusplus

#include <vector>
#include "icsneo/disk/vsa/vsa.h"

namespace icsneo {

/**
 * Class that holds single-record message records
 */
class VSA0B : public VSAMessage {
public:
	/**
	 * Constructor that reads message record data from bytestream
	 *
	 * @param bytes Bytestream to read message record data from
	 */
	VSA0B(uint8_t* const bytes);

	/**
	 * Determine whether to filter out this message record
	 *
	 * @param filter The filter to check this record against
	 *
	 * @return True if this record has passed the filter (i.e., is not being filtered out)
	 */
	bool filter(const std::shared_ptr<VSAMessageReadFilter> filter) override;

	/**
	 * Get the timestamp of this record in 25 nanosecond ticks since January 1, 2007
	 *
	 * @return Timestamp of this record in 25 nanosecond ticks since January 1, 2007
	 */
	uint64_t getTimestamp() override { return timestamp & UINT63_MAX; }

private:
	/**
	 * Perform a checksum on this record
	 *
	 * @param bytes Bytestream to test against the checksum
	 */
	void doChecksum(uint8_t* bytes) override;

	uint16_t captureBitfield; // The capture that this record is a part of
	uint8_t reserved; // Unused bytes
	uint16_t checksum; // Sum of the previous 15 half words

	uint64_t timestamp; // Timestamp of this record in 25 nanosecond ticks since January 1, 2007 (extracted from CoreMini message payload)
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA0B_H__