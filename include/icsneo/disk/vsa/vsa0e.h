#ifndef __VSA0E_H__
#define __VSA0E_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"

namespace icsneo {

/**
 * Base class for VSA0E extended message record types
 */
class VSA0E : public VSAExtendedMessage {
public:
	/**
	 * Constructor for VSA0E parent class. Passes most information to VSAExtendedMessage constructor.
	 *
	 * @param bytes Bytestream to read record data from
	 * @param messageBytes Bytestream starting at the payload of this message record
	 * @param numBytes The length of the payload of this message record
	 * @param runningChecksum Checksum for the payload bytes of this sequence of extended message records
	 * @param networkId The CoreMini Network ID for this record
	 */
	VSA0E(uint8_t* const bytes, uint8_t* const messageBytes, size_t numBytes, uint32_t& runningChecksum, Network::CoreMini networkId = static_cast<Network::CoreMini>(0xFFFFu));
};

/**
 * Class holding data for the first record in a series of VSA0E extended message records
 */
class VSA0EFirst : public VSA0E {
public:
	/**
	 * Constructor that parses first 32 bytes of bytestream into readable data.
	 *
	 * @param bytes Bytestream to parse VSA record from
	 */
	VSA0EFirst(uint8_t* const bytes, uint32_t& runningChecksum);

	/**
	 * Reserve memory in the packet data vector to store message data from this record and subsequent consecutive records.
	 *
	 * @param packet The packet to reserve memory in
	 */
	void reservePacketData(std::shared_ptr<Packet>& packet) const override;

	/**
	 * Determine whether to filter out this message record
	 *
	 * @param filter The filter to check this record against
	 *
	 * @return True if the record passes the filter
	 */
	bool filter(const std::shared_ptr<VSAMessageReadFilter> filter) override;

	/**
	 * Get the timestamp of this record. Timestamp indicates number of 25 nanosecond ticks since January 1, 2007.
	 *
	 * @return The timestamp of this record
	 */
	uint64_t getTimestamp() override { return timestamp; }

	/**
	 * Reorder bytes in the payload between this record and the second record in the sequence. The bytes are reordered to allow
	 * simple concatenation of payload bytes from records before creating and dispatching a packet.
	 *
	 * @param secondPayload Reference to the payload from the second record in the sequence
	 */
	void reorderPayload(std::vector<uint8_t>& secondPayload);

private:
	/**
	 * Perform the checksum on this record
	 *
	 * @param bytes Bytestream to test against the checksum
	 */
	void doChecksum(uint8_t* bytes) override;

	uint16_t captureBitfield; // The data capture this record is a part of
	uint64_t timestamp; // Timestamp of this record in 25 nanosecond ticks since January 1, 2007
	uint16_t checksum; // Sum of the previous 15 words
};

/**
 * Class holding data for subsequent records in series of VSA0E extended message records
 */
class VSA0EConsecutive : public VSA0E {
public:
	/**
	 * Constructor that parses first 32 bytes of VSA0E record
	 * @param bytes Bytestream to read VSA record data from
	 * @param first The first record in this series of VSA0E records
	 * @param isLastRecord Determines if this record is the last record in this series of extended message records
	 */
	VSA0EConsecutive(uint8_t* const bytes, uint32_t& runningChecksum, std::shared_ptr<VSA0EFirst> first, bool isLastRecord = false);

	/**
	 * Determine whether to filter out this message record. Utilizes the filter from the first record.
	 *
	 * @param filter The filter to check this record against
	 *
	 * @return True if this record passes the filter
	 */
	bool filter(const std::shared_ptr<VSAMessageReadFilter> filter) override { return first ? first->filter(filter) : false; }

	/**
	 * Get the timestamp of this record in 25 nanosecond ticks since January 1, 2007.
	 *
	 * @return Timestamp in 25 nanosecond ticks since January 1, 2007.
	 */
	uint64_t getTimestamp() override { return first ? first->getTimestamp() : UINT64_MAX; }

private:
	/**
	 * Perform checksum on this record with the given bytestream
	 *
	 * @param bytes The bytestream to test the checksum against
	 */
	void doChecksum(uint8_t* bytes) override;

	uint32_t recordChecksum; // The checksum for this extended record sequence found in the last record (not used if not last record)

	uint32_t calculatedChecksum; // Running checksum total for the extended record sequence of this record

	std::shared_ptr<VSA0EFirst> first = nullptr; // The first record in this series of extended message records
};

} // namespace icsneo

#endif //__cplusplus
#endif // __VSA0E_H__