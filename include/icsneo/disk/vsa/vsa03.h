#ifndef __VSA03_H__
#define __VSA03_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

namespace icsneo {

/**
 * Class used to store event
 */
class VSA03 : public VSA {
public:
	/**
	 * Constructor that extracts data from the given bytestream
	 *
	 * @param bytes Bytestream to extract VSA record data from
	 */
	VSA03(uint8_t* const bytes);

	/**
	 * Get the timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 *
	 * @return The timestamp for this record in 25 nanosecond ticks since January 1, 2007
	 */
	uint64_t getTimestamp() override { return timestamp; }

private:
	/**
	 * Perform checksum for this record
	 *
	 * @param bytes Bytestream to test against the checksum
	 */
	void doChecksum(uint8_t* bytes) override;

	enum class EventType : uint16_t {
		CaptureStarted = 0,
		StorageReconnected = 3,
		FileSystemBufferOverflow = 4,
		LoggerWentToSleep = 5,
		Internal = 7,
		CaptureStopped = 8,
		LoggerPowerEvent = 9
	} eventType; // Enumerated value indicating which type of event occurred
	uint16_t eventData; // Information about the event that is dependent on eventType
	uint64_t timestamp; // Timestamp of this record in 25 nanosecond ticks since January 1, 2007
	uint16_t checksum; // The sum of the previous 7 words
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA03_H__