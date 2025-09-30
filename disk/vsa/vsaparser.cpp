#include "icsneo/disk/vsa/vsaparser.h"
#include "icsneo/disk/vsa/vsa02.h"
#include "icsneo/disk/vsa/vsa03.h"
#include "icsneo/disk/vsa/vsa04.h"
#include "icsneo/disk/vsa/vsa05.h"
#include "icsneo/disk/vsa/vsa06.h"
#include "icsneo/disk/vsa/vsa07.h"
#include "icsneo/disk/vsa/vsa08.h"
#include "icsneo/disk/vsa/vsa09.h"
#include "icsneo/disk/vsa/vsa0b.h"
#include "icsneo/disk/vsa/vsa0c.h"
#include "icsneo/disk/vsa/vsa0d.h"
#include "icsneo/disk/vsa/vsa0e.h"
#include "icsneo/disk/vsa/vsa0f.h"
#include "icsneo/disk/vsa/vsa6a.h"
#include "icsneo/disk/diskdriver.h"

#include <iostream>

using namespace icsneo;

bool VSAParser::parseBytes(uint8_t* const bytes, uint64_t arrLen)
{
	uint64_t bytesOffset = 0;
	while(bytesOffset + VSA::StandardRecordSize <= arrLen) { // Enough bytes to read for Standard Record
		if(bytes[bytesOffset] != 0xAAu) {
			// Invalid Input
			return false;
		}
		switch(bytes[bytesOffset + 1]) {
			case 0x00u: // Pad Record
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x01u: // Message Data (Deprecated)
				hasDeprecatedRecords = true;
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x02u: // Logdata Record
				if(settings.extractAA02) {
					vsaRecords.push_back(std::make_shared<VSA02>(bytes + bytesOffset));
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x03u: // Event Record
				if(settings.extractAA03) {
					vsaRecords.push_back(std::make_shared<VSA03>(bytes + bytesOffset));
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x04u: // Partition Info Record
				if(settings.extractAA04) {
					vsaRecords.push_back(std::make_shared<VSA04>(bytes + bytesOffset));
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x05u: // Application Error Record
				if(settings.extractAA05) {
					vsaRecords.push_back(std::make_shared<VSA05>(bytes + bytesOffset));
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x06u: // Debug/Internal
				if(settings.extractAA06) {
					vsaRecords.push_back(std::make_shared<VSA06>(bytes + bytesOffset));
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x07u: // Debug/Internal
				if(settings.extractAA07) {
					vsaRecords.push_back(std::make_shared<VSA07>(bytes + bytesOffset));
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x08u: // Buffer Info Record
				if(settings.extractAA08) {
					vsaRecords.push_back(std::make_shared<VSA08>(bytes + bytesOffset));
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x09u: // Device Info Record
				if(settings.extractAA09) {
					vsaRecords.push_back(std::make_shared<VSA09>(bytes + bytesOffset));
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x0Au: // Logger Info Configuration (Deprecated)
				hasDeprecatedRecords = true;
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x0Bu: // Message Data
				if(settings.extractAA0B) {
					auto record = std::make_shared<VSA0B>(bytes + bytesOffset);
					vsaRecords.push_back(record);
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x0Cu: // PCM Audio Data
				if(settings.extractAA0C) {
					vsaRecords.push_back(std::make_shared<VSA0C>(bytes + bytesOffset));
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x0Du: // Message Data (Extended)
				if(settings.extractAA0D) {
					if(!handleExtendedRecord(bytes, bytesOffset, VSA::Type::AA0D)) {
						return false;
					}
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x0Eu: // Message Data (Extended)
				if(settings.extractAA0E) {
					if(!handleExtendedRecord(bytes, bytesOffset, VSA::Type::AA0E)) {
						return false;
					}
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x0Fu: // Message Data (Extended)
				if(settings.extractAA0F) {
					if(!handleExtendedRecord(bytes, bytesOffset, VSA::Type::AA0F)) {
						return false;
					}
				}
				bytesOffset += VSA::StandardRecordSize;
				break;
			case 0x6Au: // Logger Configuration Backup
				if(bytesOffset + Disk::SectorSize <= arrLen) {
					if(settings.extractAA6A) {
						vsaRecords.push_back(std::make_shared<VSA6A>(bytes + bytesOffset));
					}
				}
				bytesOffset += Disk::SectorSize;
				break;
			default:
				// Unhandled VSA Record Type
				return false;
				break;
		}
	}
	return true;
}

bool VSAParser::handleExtendedRecord(uint8_t* const bytes, uint64_t& bytesOffset, VSA::Type type)
{
	// Gather info about the extended record sequence of the record contained in bytes
	std::shared_ptr<VSAExtendedMessage> first;
	uint16_t seqNum;
	ExtendedMessageState::ExtendedRecordSeqInfo* seqInfo;
	uint32_t runningChecksum = 0;
	switch(type) {
		case VSA::Type::AA0D:
			first = std::make_shared<VSA0DFirst>(bytes + bytesOffset, runningChecksum);
			seqNum = first->getSequenceNum();
			seqInfo = &state.vsa0DSeqInfo[seqNum];
			break;
		case VSA::Type::AA0E:
			first = std::make_shared<VSA0EFirst>(bytes + bytesOffset, runningChecksum);
			seqNum = first->getSequenceNum();
			seqInfo = &state.vsa0ESeqInfo[seqNum];
			break;
		case VSA::Type::AA0F:
			first = std::make_shared<VSA0FFirst>(bytes + bytesOffset, runningChecksum);
			seqNum = first->getSequenceNum();
			seqInfo = &state.vsa0FSeqInfo[seqNum];
			break;
		default:
			return false; // Invalid type was passed
	}

	if(seqInfo->nextIndex == 0 && seqInfo->records.size() == 0) { // This is the first record in the sequence
		if(first->getIndex() != 0) {
			seqInfo->clear();
			report(APIEvent::Type::VSAExtendedMessageError, APIEvent::Severity::EventWarning);
			return true; // This is not actually the first record
		}
		seqInfo->records.push_back(first);
		seqInfo->totalRecordCount = first->getRecordCount();
		seqInfo->nextIndex++;
		seqInfo->runningChecksum = runningChecksum;
	} else if(seqInfo->nextIndex < seqInfo->totalRecordCount && seqInfo->records.size() > 0) { // Consecutive Record
		std::shared_ptr<VSAExtendedMessage> consecutive;
		bool isLast = seqInfo->nextIndex == seqInfo->totalRecordCount - 1;

		// Construct the consecutive record from bytes
		switch(type) {
			case VSA::Type::AA0D:
				consecutive = std::make_shared<VSA0DConsecutive>(
					bytes + bytesOffset,
					seqInfo->runningChecksum,
					std::dynamic_pointer_cast<VSA0DFirst>(seqInfo->records[0]),
					isLast
				);
				break;
			case VSA::Type::AA0E:
				consecutive = std::make_shared<VSA0EConsecutive>(
					bytes + bytesOffset,
					seqInfo->runningChecksum,
					std::dynamic_pointer_cast<VSA0EFirst>(seqInfo->records[0]),
					isLast
				);
				break;
			case VSA::Type::AA0F:
				consecutive = std::make_shared<VSA0FConsecutive>(
					bytes + bytesOffset,
					seqInfo->runningChecksum,
					std::dynamic_pointer_cast<VSA0FFirst>(seqInfo->records[0]),
					isLast
				);
				break;
			default:
				return false;
		}

		if(consecutive->getIndex() == seqInfo->nextIndex && consecutive->getSequenceNum() == seqNum) { // This record is valid in the sequence
			seqInfo->records.push_back(consecutive);
			seqInfo->nextIndex++;
		} else { // Sequence is out of order/invalid
			// Throw away incomplete sequence and report warning
			seqInfo->clear();
			report(APIEvent::Type::VSAExtendedMessageError, APIEvent::Severity::EventWarning);
			// Save data for new sequence
			if(first->getIndex() == 0) {
				seqInfo->records.push_back(first);
				seqInfo->totalRecordCount = first->getRecordCount();
				seqInfo->nextIndex++;
				seqInfo->runningChecksum = runningChecksum;
			}
			return true;
		}
		if(seqInfo->nextIndex == seqInfo->totalRecordCount) { // This is the last record in the sequence
			if(consecutive->getChecksumFailed()) {
				// Fail out if checksum fails
				seqInfo->clear();
				return false;
			}
			vsaRecords.insert(vsaRecords.end(), seqInfo->records.begin(), seqInfo->records.end());
			seqInfo->clear();
		}
	} else {
		return false; // Undefined behavior
	}
	return true;
}

VSAParser::RecordParseStatus VSAParser::getRecordFromBytes(uint8_t* const bytes, size_t arrLen, std::shared_ptr<VSA>& record)
{
	record = nullptr;
	if(arrLen < VSA::StandardRecordSize) {
		// Not enough bytes
		return VSAParser::RecordParseStatus::InsufficientData;
	} else if(bytes[0] != 0xAAu) {
		return VSAParser::RecordParseStatus::NotARecordStart;
	} else {
		switch(bytes[1]) {
			case 0x00u: // Pad Record
				return VSAParser::RecordParseStatus::Pad;
			case 0x01u: // Message Data (Deprecated)
				return VSAParser::RecordParseStatus::Deprecated;
			case 0x02u: // Logdata Record
				if(settings.extractAA02) {
					record = std::make_shared<VSA02>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x03u: // Event Record
				if(settings.extractAA03) {
					record = std::make_shared<VSA03>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x04u: // Partition Info Record
				if(settings.extractAA04) {
					record = std::make_shared<VSA04>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x05u: // Application Error Record
				if(settings.extractAA05) {
					record = std::make_shared<VSA05>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x06u: // Debug/Internal
				if(settings.extractAA06) {
					record = std::make_shared<VSA06>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x07u: // Debug/Internal
				if(settings.extractAA07) {
					record = std::make_shared<VSA07>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x08u: // Buffer Info Record
				if(settings.extractAA08) {
					record = std::make_shared<VSA08>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x09u: // Device Info Record
				if(settings.extractAA09) {
					record = std::make_shared<VSA09>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x0Au:
				return VSAParser::RecordParseStatus::Deprecated;
			case 0x0Bu: // Message Data
				if(settings.extractAA0B) {
					record = std::make_shared<VSA0B>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x0Cu: // PCM Audio Data
				if(settings.extractAA0C) {
					record = std::make_shared<VSA0C>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			case 0x0Du: // Message Data (Extended)
				if(settings.extractAA0D) {
					uint32_t payloadChecksum = 0;
					const auto& vsa = std::make_shared<VSA0DFirst>(bytes, payloadChecksum);
					record = vsa;
					if(vsa->getIndex() == 0) {
						return VSAParser::RecordParseStatus::Success;
					}
					// This returns the consecutive record as a first record
					return VSAParser::RecordParseStatus::ConsecutiveExtended;
				}
				break;
			case 0x0Eu: // Message Data (Extended)
				if(settings.extractAA0E) {
					uint32_t payloadChecksum = 0;
					const auto& vsa = std::make_shared<VSA0EFirst>(bytes, payloadChecksum);
					record = vsa;
					if(vsa->getIndex() == 0) {
						return VSAParser::RecordParseStatus::Success;
					}
					// This returns the consecutive record as a first record
					return VSAParser::RecordParseStatus::ConsecutiveExtended;
				}
				break;
			case 0x0Fu: // Message Data (Extended)
				if(settings.extractAA0F) {
					uint32_t payloadChecksum = 0;
					const auto& vsa = std::make_shared<VSA0FFirst>(bytes, payloadChecksum);
					record = vsa;
					if(vsa->getIndex() == 0) {
						return VSAParser::RecordParseStatus::Success;
					}
					// This returns the consecutive record as a first record
					return VSAParser::RecordParseStatus::ConsecutiveExtended;
				}
				break;
			case 0x6Au: // Logger Configuration Backup
				if(settings.extractAA6A) {
					if(arrLen < Disk::SectorSize) {
						return VSAParser::RecordParseStatus::InsufficientData;
					}
					record = std::make_shared<VSA6A>(bytes);
					return VSAParser::RecordParseStatus::Success;
				}
				break;
			default:
				// Unhandled VSA Record Type
				return VSAParser::RecordParseStatus::UnknownRecordType;
				break;
		}
	}
	return VSAParser::RecordParseStatus::FilteredOut;
}

void VSAParser::clearParseState()
{
	for(size_t i = 0; i < state.vsa0DSeqInfo.size(); i++) {
		state.vsa0DSeqInfo[i].clear();
	}
	for(size_t i = 0; i < state.vsa0ESeqInfo.size(); i++) {
		state.vsa0ESeqInfo[i].clear();
	}
	for(size_t i = 0; i < state.vsa0DSeqInfo.size(); i++) {
		state.vsa0ESeqInfo[i].clear();
	}
}

bool VSAParser::extractMessagePackets(std::vector<std::shared_ptr<Packet>>& packets)
{
	if(settings != Settings::messageRecords()) {
		report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
		return false; // We do not have exclusively message records
	}
	std::shared_ptr<Packet> packet;
	bool activeExtendedMessage = false;
	VSA::Type previousRecordType = VSA::Type::Invalid;
	for(const auto& record : vsaRecords) {
		VSA::Type activeRecordType = record->getType();
		switch(activeRecordType) {
			// Handle standard message records
			case VSA::Type::AA0B: {
				if(activeExtendedMessage) {
					// Non-terminated extended message record
					// There was a failure/unexpected behavior in the parsing process
					report(APIEvent::Type::VSAExtendedMessageError, APIEvent::Severity::Error);
					return false;
				}

				std::shared_ptr<VSAMessage> messageRecord = std::dynamic_pointer_cast<VSAMessage>(record);
				if(!settings.messageFilter || messageRecord->filter(settings.messageFilter)) {
					packet = messageRecord->getPacket();
					packets.push_back(packet);
				}
				packet = nullptr;
				break;
			}

			// Handle extended message records
			case VSA::Type::AA0D:
			case VSA::Type::AA0E:
			case VSA::Type::AA0F: {
				std::shared_ptr<VSAExtendedMessage> extendedMessageRecord = std::dynamic_pointer_cast<VSAExtendedMessage>(record);

				if(!activeExtendedMessage) { // Start new extended message packet
					packet = extendedMessageRecord->getPacket();
					activeExtendedMessage = true;
					previousRecordType = extendedMessageRecord->getType();
				} else if(previousRecordType == activeRecordType) { // Continue existing extended message packet
					extendedMessageRecord->appendPacket(packet);
					if(extendedMessageRecord->getRecordCount() == static_cast<uint32_t>(extendedMessageRecord->getIndex() + 1)) { // Last record in sequence
						if(!settings.messageFilter || extendedMessageRecord->filter(settings.messageFilter)) {
							packets.push_back(packet);
						}
						activeExtendedMessage = false;
						packet = nullptr;
						previousRecordType = activeRecordType;
					}
				} else {
					// Non-terminated extended message record
					// There was a failure/unexpected behavior in the parsing process
					activeExtendedMessage = false;
					packet = nullptr;
					previousRecordType = VSA::Type::Invalid;
					report(APIEvent::Type::VSAOtherError, APIEvent::Severity::Error);
					return false;
				}
				break;
			}

			default:
				// Non-message record discovered
				report(APIEvent::Type::VSAOtherError, APIEvent::Severity::Error);
				return false;
		}
	}
	vsaRecords.clear();
	return true;
}