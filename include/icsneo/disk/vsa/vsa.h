#ifndef __VSA_H__
#define __VSA_H__

#ifdef __cplusplus

#include "icsneo/communication/network.h"
#include "icsneo/communication/packet.h"
#include <vector>
#include <memory>
#include <chrono>
#include "stdint.h"

namespace icsneo {

using CaptureBitfield = uint16_t;

static constexpr uint64_t ICSEpochHoursSinceUnix = 13514 * 24; // Number of hours between the start of the Unix epoch and the start of the ICS epoch
static constexpr uint64_t UINT63_MAX = 0x7FFFFFFFFFFFFFFFu;

/**
 * Struct that meets Clock format requirements from STL Chrono library.
 * Indicates time for for the ICS Epoch (January 1, 2007) in 25 nanosecond ticks.
 */
struct ICSClock {
	using rep = uint64_t; // Type for tick count
	using period = std::ratio_multiply<std::ratio<25>, std::nano>; // Ratio of tick length to seconds (25 nanoseconds)
	using duration = std::chrono::duration<rep, period>; // Type for duration in 25 nanosecond ticks
	using time_point = std::chrono::time_point<ICSClock>; // Type for a point in time with respect to ICSClock

	static constexpr bool is_steady = true; // This clock does not move backwards

	/**
	 * Get the time_point at the current time with respect to ICSClock
	 *
	 * @return Time point at the current time with respect to ICSClock
	 */
	static time_point now() noexcept
	{
		return time_point { std::chrono::duration_cast<duration>(std::chrono::system_clock::now().time_since_epoch()) -
							std::chrono::hours(ICSEpochHoursSinceUnix) };
	}
};

using Timestamp = ICSClock::time_point; // Point in time to start or stop read

class VSA;

/**
 * Holds metadata for the VSA log file
 */
struct VSAMetadata {
	uint64_t firstRecordLocation = UINT64_MAX; // Location of the record with lowest timestamp in ring buffer
	std::shared_ptr<VSA> firstRecord = nullptr; // The record with lowest timestamp
	uint64_t lastRecordLocation = UINT64_MAX; // Location of the record with the highest timestamp in ring buffer
	std::shared_ptr<VSA> lastRecord = nullptr; // The record with the highest timestamp
	uint64_t bufferEnd = UINT64_MAX; // One byte beyond the last byte of the sequence started from lastRecordLocation
	uint64_t diskSize = 0; // The size of the vsa log file on the disk
	bool isOverlapped = false; // Determines if VSA ring buffer has looped to beginning
	uint64_t coreMiniTimestamp = UINT64_MAX; // Timestamp of the CoreMini message in 25 nanosecond ticks since January 1, 2007
};

/**
 * Struct used to exclude VSA message records from parse
 */
struct VSAMessageReadFilter {
	CaptureBitfield captureBitfield = UINT16_MAX; // The capture from which to gather VSA message records. UINT16_MAX indicates 'all captures'

	// The range of timestamps to collect record data from
	std::pair<Timestamp, Timestamp> readRange = std::make_pair(Timestamp(ICSClock::duration(0x0ull)), Timestamp(ICSClock::duration(UINT64_MAX)));

	static constexpr Timestamp MinTimestamp = Timestamp(ICSClock::duration(0x0ull));
	static constexpr Timestamp MaxTimestamp = Timestamp(ICSClock::duration(UINT64_MAX));
};

struct VSAExtractionSettings {
	bool parseOldRecords = false;
	bool stopCoreMini = true;
	std::vector<VSAMessageReadFilter> filters;
};

/**
 * Abstract VSA base class to store VSA record data read from VSA log file on disk
 */
class VSA {
public:
	virtual ~VSA() = default;
	static constexpr size_t StandardRecordSize = 32; // Size of most VSA records
	static constexpr uint64_t RecordStartOffset = 0x06000000u; // Offset of VSA record ring buffer from start of VSA log file

	/**
	 * Convert the given time_point object to a timestamp in 25 nanosecond ticks since January 1, 2007
	 *
	 * @return Timestamp of the given time_point in 25 nanosecond ticks since January 1, 2007
	 */
	static uint64_t getICSTimestampFromTimepoint(const Timestamp& point) noexcept { return point.time_since_epoch().count(); }

	/**
	 * Enum to determine what type of record is underlying VSA parent class
	 */
	enum class Type : uint16_t {
		AA00 = 0xAA00u, // Pad
		AA01 = 0xAA01u, // Message Data (Deprecated)
		AA02 = 0xAA02u, // 'Logdata'
		AA03 = 0xAA03u, // Event
		AA04 = 0xAA04u, // Partition Info
		AA05 = 0xAA05u, // Application Error
		AA06 = 0xAA06u, // Internal/Debug
		AA07 = 0xAA07u, // Internal/Debug
		AA08 = 0xAA08u, // Buffer Info
		AA09 = 0xAA09u, // Device Info
		AA0A = 0xAA0Au, // Logger Configuration Info (Deprecated)
		AA0B = 0xAA0Bu, // Message Data
		AA0C = 0xAA0Cu, // PCM Audio Data
		AA0D = 0XAA0Du, // Message Data (Extended)
		AA0E = 0xAA0Eu, // Message Data (Extended)
		AA0F = 0xAA0Fu, // Message Data (Extended)
		AA6A = 0xAA6Au, // Logger Configuration Backup (512 Bytes)
		Invalid = UINT16_MAX // Used to indicate unset or unhandled VSA record types
	};

	/**
	 * Get the record type
	 *
	 * @return Type of record
	 */
	Type getType() const { return type; }

	/**
	 * Get the timestamp stored in this record
	 *
	 * @return The record's timestamp in 25 nanosecond ticks since January 1, 2007
	 */
	virtual uint64_t getTimestamp() = 0;

	/**
	 * Determine whether this record has a valid timestamp. All invalid timestamps are set to the maximum value for a uint64_t.
	 *
	 * @return True if the timestamp is set to a valid number
	 */
	bool isTimestampValid() { return getTimestamp() != UINT64_MAX && !checksumFailed; }

	/**
	 * Determine if the checksum for this record failed
	 *
	 * @return True if the checksum does not pass
	 */
	bool getChecksumFailed() { return checksumFailed; }

	/**
	 * Get the timestamp of this record in C++ native std::chrono::time_point
	 *
	 * @return Timestamp of record as an std::chrono::time_point
	 */
	Timestamp getTimestampICSClock()
	{
		return Timestamp(std::chrono::duration_cast<ICSClock::duration>(std::chrono::nanoseconds(getTimestamp() * 25)));
	}

protected:
	/**
	 * Used to construct a VSA record from child class
	 */
	VSA() {}

	/**
	 * Used to set the type of this record in child class constructors
	 *
	 * @param recordType The type of this record
	 */
	void setType(Type recordType) { this->type = recordType; }

	/**
	 * Set whether the checksum was passed for this record. This is called in each child class constructor.
	 *
	 * @param fail True if checksum did not pass, else false
	 */
	void setChecksumFailed(bool fail) { checksumFailed = fail; }

private:
	/**
	 * Performs checksum on data in specific record type. Calls VSA::setChecksumFailed(...) from child class.
	 *
	 * @param recordBytes Bytestream of record to perform checksum with
	 */
	virtual void doChecksum(uint8_t* recordBytes) = 0;

	Type type = Type::Invalid; // The type of this record
	bool checksumFailed = false; // Determines if checksum failed
};

/**
 * Interface class for handling common functionality of VSAMessage record types (AA0B, AA0D, AA0E, AA0F)
 */
class VSAMessage : public VSA {
public:
	static constexpr size_t CoreMiniPayloadSize = 24; // Size of CoreMini message (payload)

	/**
	 * Construct a packet from the message payload and network
	 *
	 * @return Packet constructed from payload and network
	 */
	std::shared_ptr<Packet> getPacket() const;

	/**
	 * Reserve enough memory to store a CoreMini message in the given packet
	 *
	 * @param packet The packet in which we are reserving memory for a message
	 */
	virtual void reservePacketData(std::shared_ptr<Packet>& packet) const { packet->data.reserve(CoreMiniPayloadSize); }

	/**
	 * Determine whether to filter out this VSAMessage record during parsing
	 *
	 * @param filter The filter struct to check this message record against
	 *
	 * @return True if this message passes the given filter
	 */
	virtual bool filter(const std::shared_ptr<VSAMessageReadFilter> filter) = 0;

protected:
	/**
	 * Constructor for normal instance of VSAMessage class
	 *
	 * @param messageBytes Bytestream that begins at the start of the message payload
	 * @param numBytes The number of bytes that the message payload contains
	 * @param networkId The CoreMini ID of the network for this message
	 */
	VSAMessage(uint8_t* const messageBytes, size_t numBytes, Network::CoreMini networkId = static_cast<Network::CoreMini>(UINT16_MAX))
		: VSA(), payload(messageBytes, messageBytes + numBytes), network(networkId) {}

	std::vector<uint8_t> payload; // CoreMini message/payload of VSA record containing message data
	Network network; // CoreMini network of this message
};

/**
 * Interface class for handling common functionality of VSA Extended Message records (AA0D, AA0E, AA0F)
 */
class VSAExtendedMessage : public VSAMessage {
public:
	static void truncatePacket(std::shared_ptr<Packet> packet);

	/**
	 * Appends the payload for this message to the given packet.
	 * Also sets the network of the packet if unset (used primarily for AA0F records which do not contain the network in the first extended message record).
	 *
	 * @param packet The packet to append this record's payload to
	 */
	void appendPacket(std::shared_ptr<Packet> packet) const;

	/**
	 * Get the total number of records for this extended message
	 *
	 * @return Total number of records that this message spans
	 */
	uint32_t getRecordCount() const { return totalRecordCount; }

	/**
	 * Get the index of this record in the extended message sequence
	 *
	 * @return The index of this record
	 */
	uint16_t getIndex() { return index; };

	/**
	 * Get the numerical id of the sequence of extended message records this record is a part of
	 *
	 * @return The sequence number of this extended message record
	 */
	uint16_t getSequenceNum() { return sequenceNum; }

protected:
	/**
	 * Constructor for normal instance of VSAExtendedMessage
	 *
	 * @param messageBytes Bytestream that begins at the start of the message payload
	 * @param numBytes The length of the message payload in bytes
	 * @param networkId The CoreMini ID of the network for this message
	 */
	VSAExtendedMessage(uint8_t* const messageBytes, size_t numBytes, Network::CoreMini networkId = static_cast<Network::CoreMini>(UINT16_MAX))
		: VSAMessage(messageBytes, numBytes, networkId) {}

	/**
	 * Set the total number of records for this message
	 *
	 * @param recordCount Total number of records for this message
	 */
	void setRecordCount(uint32_t recordCount) { totalRecordCount = recordCount; }

	/**
	 * Set the index of this record
	 *
	 * @param recordIndex The index of this record in its extended message sequence
	 */
	void setIndex(uint16_t recordIndex) { this->index = recordIndex; }

	/**
	 * Set the sequence number of this record
	 *
	 * @param seq The id for the extended message sequence this record is a part of
	 */
	void setSequenceNum(uint16_t seq) { sequenceNum = seq; }

private:
	uint32_t totalRecordCount; // The total number of records for the extended message
	uint16_t index; // The index of this record in its extended message sequence
	uint16_t sequenceNum; // The id of the sequence of records this record is a part of
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSA_H__