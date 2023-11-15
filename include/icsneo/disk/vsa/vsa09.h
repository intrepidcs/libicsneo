#ifndef __VSA09_H__
#define __VSA09_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

namespace icsneo {

/**
 * Class used to store data for a Device Information VSA Record
 */
class VSA09 : public VSA {
public:
	/**
	 * Constructor to convert bytestream to Device Information VSA Record
	 *
	 * @param bytes Bytestream to convert into Device Information Record
	 */
	VSA09(uint8_t* const bytes);

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

	uint32_t serialNumber; // Decimal representation of Base-36 serial number
	uint8_t firmwareMajorVersion; // Major version of firmware (A for A.B version)
	uint8_t firmwareMinorVersion; // Minor version of firmware (B for A.B version)
	uint8_t manufactureMajorRevision; // Major version of manufacture revision (A for A.B revision)
	uint8_t manufactureMinorRevision; // Minor version of manufacture revision (B for A.B revision)
	uint8_t bootloaderMajorVersion; // Major version of bootloader (A for A.B version)
	uint8_t bootloaderMinorVersion; // Minor version of bootloader (B for A.B version)
	std::vector<uint8_t> reserved0; // Unused bytes (6 bytes)
	enum class HardwareID : uint8_t {
		NeoVIRED = 0,
		NeoVIFIRE = 1,
		NeoVIION = 11,
		RADGalaxy = 19,
		RADMars = 29,
		ValueLOG = 31,
		NeoVIRED2FIRE3 = 33,
		RADGigastar = 36
	} hardwareID; // Identifier for specific hardware device type
	std::vector<uint8_t> reserved1; // Unused bytes (3 bytes)
	uint64_t timestamp; // Timestamp for this record in 25 nanosecond ticks since January 1, 2007
	uint16_t checksum; // Sum of the previous 15 words
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA09_H__