#ifndef __VSAPARSER_H__
#define __VSAPARSER_H__

#ifdef __cplusplus

#include "icsneo/disk/vsa/vsa.h"
#include "icsneo/disk/vsa/vsa0d.h"
#include "icsneo/disk/vsa/vsa0e.h"
#include "icsneo/disk/vsa/vsa0f.h"
#include "icsneo/communication/message/message.h"
#include "icsneo/api/eventmanager.h"

#include <vector>
#include <array>

namespace icsneo {

/**
 * Class used to parse VSA records from bytestreams
 */
class VSAParser {
public:
	/**
	 * Struct that determines which types of VSA records to extract from disk
	 */
	struct Settings {
		bool extractAA02 = true; // Extract Logdata Records
		bool extractAA03 = true; // Extract Event Records
		bool extractAA04 = true; // Extract Partition Info Records
		bool extractAA05 = true; // Extract Application Error Records
		bool extractAA06 = true; // Extract Debug/Internal 1 Records
		bool extractAA07 = true; // Extract Debug/Internal 2 Records
		bool extractAA08 = true; // Extract Buffer Info Records
		bool extractAA09 = true; // Extract Device Info Records
		bool extractAA0B = true; // Extract Message Records
		bool extractAA0C = true; // Extract PCM Audio Records
		bool extractAA0D = true; // Extract Extended Message 1 Records
		bool extractAA0E = true; // Extract Extended Message 2 Records
		bool extractAA0F = true; // Extract Extended Message 3 Records
		bool extractAA6A = true; // Extract Logger Configuration Backup Records

		std::shared_ptr<VSAMessageReadFilter> messageFilter = nullptr; // Used for post-read filtering of message records

		/**
		 * Static constructor for VSAParser::Settings that only extracts message records (AA0B, AA0D, AA0E, AA0F)
		 */
		static Settings messageRecords() { return { false, false, false, false, false, false, false, false, true, false, true, true, true, false }; }

		/**
		 * Operator overload for equivalency of VSAParser::Settings struct
		 *
		 * @param s The settings object to test against this settings object
		 *
		 * @return True if the extraction settings are the same. Does not check the filter
		 */
		bool operator==(const Settings& s)
		{
			return s.extractAA02 == this->extractAA02 && s.extractAA03 == this->extractAA03 && s.extractAA04 == this->extractAA04 &&
				   s.extractAA05 == this->extractAA05 && s.extractAA06 == this->extractAA06 && s.extractAA07 == this->extractAA07 &&
				   s.extractAA08 == this->extractAA08 && s.extractAA09 == this->extractAA09 && s.extractAA0B == this->extractAA0B &&
				   s.extractAA0C == this->extractAA0C && s.extractAA0D == this->extractAA0D && s.extractAA0E == this->extractAA0E &&
				   s.extractAA0F == this->extractAA0F && s.extractAA6A == this->extractAA6A;
		}

		/**
		 * Operator overload for non-equivalency of VSAParser::Settings struct
		 *
		 * @param s The settings object to test against this settings object
		 *
		 * @return True if the extraction settings are not the same. Does not check the filter
		 */
		bool operator!=(const Settings& s)
		{
			return !(*this == s);
		}
	};

	/**
	 * Enumerated values to determine status of attempt to parse out-of-context record from bytestream
	 */
	enum class RecordParseStatus : uint8_t {
		NotARecordStart, // Indicates first byte was not of format required for VSA records
		Pad, // This record is a pad record
		Deprecated, // This record is deprecated
		ConsecutiveExtended, // This is a consecutive extended message record (i.e., not the first record in an extended message sequence)
		FilteredOut, // This record was filtered out due to the current Settings of the VSAParser
		UnknownRecordType, // The second byte indicates a record type that is unknown/not handled
		InsufficientData, // There were not enough bytes given to the parse call
		Success // The record was successfully parsed
	};

	/**
	 * Constructor with default settings
	 *
	 * @param report Handler to report APIEvents
	 */
	VSAParser(const device_eventhandler_t& report) { this->report = report; }

	/**
	 * Constructor with non-default settings
	 *
	 * @param report Handler to report APIEvents
	 * @param settings The settings to use for this parser
	 */
	VSAParser(const device_eventhandler_t& report, const Settings& settings)
		: settings(settings) { this->report = report; }

	/**
	 * Parse the given bytestream into VSA records and store them in vsaRecords.
	 * Non-terminated extended message record sequences are stored in a temporary buffer until they terminate.
	 *
	 * @param bytes Bytestream to parse VSA records from
	 * @param arrLen The number of bytes in the bytestream
	 *
	 * @return True if there was no failure or unhandled behavior during parse, else false
	 */
	bool parseBytes(uint8_t* const bytes, uint64_t arrLen);

	/**
	 * Get the last fully-parsed record in the parser
	 */
	std::shared_ptr<VSA>& back() { return vsaRecords.back(); }

	/**
	 * Get the number of records contained within the parser
	 *
	 * @return Size of the vector of VSA records
	 */
	size_t size() { return vsaRecords.size(); }

	/**
	 * Determine if number of records contained within the parser is 0
	 *
	 * @return True if the parser record container is empty
	 */
	bool empty() { return vsaRecords.empty(); }

	/**
	 * Clear all fully-parsed records from the parser. Does not affect non-terminated extended message records stored in buffers.
	 */
	void clearRecords() { vsaRecords.clear(); }

	/**
	 * Parse first record from the given bytestream.
	 *
	 * @param bytes The bytestream to read from
	 * @param arrLen Length of the bytestream
	 * @param record Variable to pass out the record if able to parse
	 *
	 * @return The status of the record parse
	 */
	RecordParseStatus getRecordFromBytes(uint8_t* const bytes, size_t arrLen, std::shared_ptr<VSA>& record);

	/**
	 * Set a message filter for the Settings for this VSAParser
	 *
	 * @param filter The message filter to set for this VSAParser
	 */
	void setMessageFilter(const VSAMessageReadFilter& filter) { settings.messageFilter = std::make_shared<VSAMessageReadFilter>(filter); }

	/**
	 * Remove the message filter for the Settings from this parser
	 */
	void clearMessageFilter() { settings.messageFilter = nullptr; }

	/**
	 * Clear all extended message buffers and parse states
	 */
	void clearParseState();

	/**
	 * Extract all packets from fully-parsed VSA records and store them in the given buffer
	 *
	 * @param packets The vector in which to store the packets from fully-parsed records
	 *
	 * @return True if packets were successfully extracted
	 */
	bool extractMessagePackets(std::vector<std::shared_ptr<Packet>>& packets);

private:
	/**
	 * Holds the state of all possible extended message record sequences (most will be empty/null)
	 */
	struct ExtendedMessageState {
		/**
		 * Holds the state of a single extended message record sequence
		 */
		struct ExtendedRecordSeqInfo {
			/**
			 * Reset the state of this record sequence
			 */
			void clear()
			{
				nextIndex = 0;
				totalRecordCount = 0;
				runningChecksum = 0;
				records.clear();
				records.shrink_to_fit();
			}

			uint16_t nextIndex = 0; // The next index to be parsed in this sequence
			uint32_t totalRecordCount = 0; // The total number of records that are in this sequence
			uint32_t runningChecksum = 0; // The running calculated checksum for this sequence

			std::vector<std::shared_ptr<VSAExtendedMessage>> records; // All of the records in this sequence
		};

		std::array<ExtendedRecordSeqInfo, 128> vsa0DSeqInfo; // Holds state for each possible sequence ID for VSA0D
		std::array<ExtendedRecordSeqInfo, 256> vsa0ESeqInfo; // Holds state for each possible sequence ID for VSA0E
		std::array<ExtendedRecordSeqInfo, 128> vsa0FSeqInfo; // Holds state for each possible sequence ID for VSA0F
	};

	/**
	 * Handle parsing of extended message records
	 *
	 * @param bytes The bytestream to parse the extended message record from
	 * @param bytesOffset The offset in the bytestream to read the record from
	 * @param type The type of VSA extended message record that we are parsing (AA0D, AA0E, AA0F)
	 *
	 * @return True if no unhandled failures to parse occurred
	 */
	bool handleExtendedRecord(uint8_t* const bytes, uint64_t& bytesOffset, VSA::Type type);

	std::vector<std::shared_ptr<VSA>> vsaRecords; // The vector of records that this parser has parsed
	bool hasDeprecatedRecords = false; // Indicates whether records of deprecated types are present in the disk
	Settings settings; // The settings used to determine which records to save to records vector
	ExtendedMessageState state; // The parse state of all possible extended message sequences
	device_eventhandler_t report; // Event handler to report APIEvents
};

} // namespace icsneo

#endif // __cplusplus
#endif // __VSAPARSER_H__