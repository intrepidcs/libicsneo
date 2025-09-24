#ifndef __VSA0C_H__
#define __VSA0C_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

#include <vector>

namespace icsneo {

/**
 * Class used to hold data for a PCM Audio VSA Record
 */
class VSA0C : public VSA {
public:
	/**
	 * Constructor to convert bytestream to PCM Audio Record
	 *
	 * @param bytes Bytestream to parse
	 */
	VSA0C(uint8_t* const bytes);

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

	uint16_t captureBitfield; // Capture this record is a member of (Unused)
	uint8_t audioPreamble; // Unknown
	uint8_t audioHeader; // Unknown
	std::vector<uint8_t> pcmData; // Audio data payload (14 bytes)
	uint64_t timestamp; // Timestamp of this record in 25 nanosecond ticks since January 1, 2007
	struct VNet {
		uint16_t vNetSlot : 2; // Bits to identify VNet slot of this record
		uint16_t reserved : 14; // Unused bits
	} vNetBitfield; // Struct to ensure VNetSlot is only 2 bits
	uint16_t checksum; // Sum of the previous 15 words
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA0C_H__