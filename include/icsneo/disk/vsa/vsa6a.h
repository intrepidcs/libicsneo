#ifndef __VSA6A_H__
#define __VSA6A_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

namespace icsneo {

/**
 * Class that contains data for a Logger Configuration Backup Record
 */
class VSA6A : public VSA {
public:
	/**
	 * Constructor to convert a bytestream to a Logger Configuration Backup Record
	 *
	 * @param bytes Bytestream to convert to Logger Configuration Backup Record
	 */
	VSA6A(uint8_t* const bytes);

	/**
	 * Get the timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 *
	 * @return The timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 */
	uint64_t getTimestamp() override { return timestamp; }

private:
	/**
	 * Perform the checksum on this record
	 *
	 * @param bytes Bytestream to test against the checksum
	 */
	void doChecksum(uint8_t* bytes) override;

	uint32_t sequenceNum; // Unknown
	uint32_t totalSectors; // Unknown
	uint32_t reserved; // Unused bytes
	uint64_t timestamp; // Timestamp of this record in 25 nanosecond ticks since January 1, 2007
	uint16_t timestampSum; // Sum of the bytes in this record's timestamp (previous 8 bytes)
	std::vector<uint8_t> data; // Payload data for this record (452 bytes)
	uint32_t checksum; // Sum of the previous 452 bytes (bytes from data)
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA6A_H__