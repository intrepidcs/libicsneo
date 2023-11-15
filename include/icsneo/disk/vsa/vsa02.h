#ifndef __VSA02_H__
#define __VSA02_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

namespace icsneo {

/**
 * Class that contains data for Logdata records
 */
class VSA02 : public VSA {
public:
	/**
	 * Constructor that parses the given bytestream
	 *
	 * @param bytes Bystream that contains data for Logdata VSA records
	 */
	VSA02(uint8_t* const bytes);

	/**
	 * Get the timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 *
	 * @return The timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 */
	uint64_t getTimestamp() override { return timestamp; }

private:
	/**
	 * Perform the checksum for this record
	 *
	 * @param bytes Bystream to test against the checksum
	 */
	void doChecksum(uint8_t* bytes) override;

	uint16_t constantIndex; // Index into CoreMini binary where constant data for this record can be found
	struct Flags {
		bool hasMoreData : 1; // Set to true if there are further Logdata records expected to terminate this "chain"
		uint8_t numSamples : 3; // Number of valid samples in samples
		bool isAscii : 1; // Set to true if the processing code should treat samples as an ASCII string
		bool prefixTime : 1; // Set to true if the function block step that created this record requested that the timestamp be prepended on the output
		bool sample0IsHex : 1; // Set to true if the value in sample 0 should be written as hex
		bool sample1IsHex : 1; // Set to true if the value in sample 1 shoudl be written as hex
	} flags; // Series of flags for this record
	uint8_t pieceCount; // Value of the rolling counter for this "chain" of logdata records
	uint64_t timestamp; // Timestamp in 25 nanosecond ticks since January 1, 2007
	std::vector<uint8_t> samples; // Data for this record that varies based on the above flags. Either 2 32.32 fixed point values or 16 byte ASCII string
	uint16_t checksum; // The sum of the previous 15 words
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA02_H__