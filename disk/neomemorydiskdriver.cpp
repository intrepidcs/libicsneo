#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/communication/message/neoreadmemorysdmessage.h"
#include "icsneo/communication/message/flashmemorymessage.h"
#include <cstring>
#include <iostream>

using namespace icsneo;
using namespace icsneo::Disk;

std::optional<uint64_t> NeoMemoryDiskDriver::readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout, MemoryType memType) {
	const auto filter = std::make_shared<MessageFilter>((memType == MemoryType::SD ? Network::NetID::NeoMemorySDRead : Network::NetID::RED_INT_MEMORYREAD));
	filter->includeInternalInAny = true;

	if(pos % SectorSize != 0)
		return std::nullopt;

	if(amount != SectorSize)
		return std::nullopt;

	const uint64_t currentSector = pos / SectorSize;
	const uint8_t memLocation = (uint8_t)memType;

	uint64_t numWords = amount / 2;

	auto msg = com.waitForMessageSync([&currentSector, &memLocation, &com, &numWords] {
		return com.sendCommand(Command::NeoReadMemory, {
			memLocation,
			uint8_t(currentSector & 0xFF),
			uint8_t((currentSector >> 8) & 0xFF),
			uint8_t((currentSector >> 16) & 0xFF),
			uint8_t((currentSector >> 24) & 0xFF),
			uint8_t(numWords & 0xFF),
			uint8_t((numWords >> 8) & 0xFF),
			uint8_t((numWords >> 16) & 0xFF),
			uint8_t((numWords >> 24) & 0xFF)
		});
	}, filter, timeout);

	if(!msg)
		return 0;

	if(memType == MemoryType::SD) {
		const auto mem = std::dynamic_pointer_cast<NeoReadMemorySDMessage>(msg);
		if(!mem || mem->data.size() != SectorSize) {
			report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
			return std::nullopt;
		}
		memcpy(into, mem->data.data(), SectorSize);
	} else { // flash
		const auto mem = std::dynamic_pointer_cast<FlashMemoryMessage>(msg);
		if(!mem || mem->data.size() != SectorSize) {
			report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
			return std::nullopt;
		}
		memcpy(into, mem->data.data(), SectorSize);
	}
	return SectorSize;
}

std::optional<uint64_t> NeoMemoryDiskDriver::writeLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout, MemoryType memType) {

	static std::shared_ptr<MessageFilter> NeoMemoryDone = std::make_shared<MessageFilter>(Network::NetID::NeoMemoryWriteDone);

	if(pos % SectorSize != 0)
		return std::nullopt;

	if(amount != SectorSize)
		return std::nullopt;

	const uint64_t currentSector = pos / SectorSize;
	const uint8_t memLocation = (uint8_t)memType;

	uint64_t numWords = amount / 2;

	auto msg = com.waitForMessageSync([&currentSector, &memLocation, &com, from, amount, &numWords] {
		std::vector<uint8_t> command = {
			memLocation,
			uint8_t(currentSector & 0xFF),
			uint8_t((currentSector >> 8) & 0xFF),
			uint8_t((currentSector >> 16) & 0xFF),
			uint8_t((currentSector >> 24) & 0xFF),
			uint8_t(numWords & 0xFF),
			uint8_t((numWords >> 8) & 0xFF),
		};
		command.insert(command.end(), from, from + amount);
		return com.sendCommand(Command::NeoWriteMemory, command);
	}, NeoMemoryDone, timeout);

	if(!msg)
		return std::nullopt;

	return SectorSize;
}