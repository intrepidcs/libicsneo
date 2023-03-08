#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/communication/message/neoreadmemorysdmessage.h"
#include <cstring>
#include <iostream>

using namespace icsneo;
using namespace icsneo::Disk;

std::optional<uint64_t> NeoMemoryDiskDriver::readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout, MemoryType memType) {
	static std::shared_ptr<MessageFilter> NeoMemorySDRead = std::make_shared<MessageFilter>(Network::NetID::NeoMemorySDRead);

	if(pos % SectorSize != 0)
		return std::nullopt;

	if(amount != SectorSize)
		return std::nullopt;

	const uint64_t currentSector = pos / SectorSize;
	const uint8_t memLocation = (uint8_t)memType;
	
	auto msg = com.waitForMessageSync([&currentSector, &memLocation, &com] {
		return com.sendCommand(Command::NeoReadMemory, {
			memLocation,
			uint8_t(currentSector & 0xFF),
			uint8_t((currentSector >> 8) & 0xFF),
			uint8_t((currentSector >> 16) & 0xFF),
			uint8_t((currentSector >> 24) & 0xFF),
			uint8_t(SectorSize & 0xFF),
			uint8_t((SectorSize >> 8) & 0xFF),
			uint8_t((SectorSize >> 16) & 0xFF),
			uint8_t((SectorSize >> 24) & 0xFF)
		});
	}, NeoMemorySDRead, timeout);

	if(!msg)
		return 0;

	const auto sdmsg = std::dynamic_pointer_cast<NeoReadMemorySDMessage>(msg);
	if(!sdmsg || sdmsg->data.size() != SectorSize) {
		report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::Error);
		return std::nullopt;
	}

	memcpy(into, sdmsg->data.data(), SectorSize);
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

	auto msg = com.waitForMessageSync([&currentSector, &memLocation, &com, from, amount] {
		std::vector<uint8_t> command = {
			memLocation,
			uint8_t(currentSector & 0xFF),
			uint8_t((currentSector >> 8) & 0xFF),
			uint8_t((currentSector >> 16) & 0xFF),
			uint8_t((currentSector >> 24) & 0xFF),
			uint8_t(SectorSize & 0xFF),
			uint8_t((SectorSize >> 8) & 0xFF),
		};
		command.insert(command.end(), from, from + amount);
		return com.sendCommand(Command::NeoWriteMemory, command);
	}, NeoMemoryDone, timeout);

	if(!msg)
		return std::nullopt;

	return SectorSize;
}