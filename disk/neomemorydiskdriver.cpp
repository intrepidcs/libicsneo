#include "icsneo/disk/neomemorydiskdriver.h"
#include "icsneo/communication/message/neoreadmemorysdmessage.h"
#include <cstring>

using namespace icsneo;
using namespace icsneo::Disk;

optional<uint64_t> NeoMemoryDiskDriver::readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) {
	static std::shared_ptr<MessageFilter> NeoMemorySDRead = std::make_shared<MessageFilter>(Network::NetID::NeoMemorySDRead);

	if(pos % SectorSize != 0)
		return nullopt;

	if(amount != SectorSize)
		return nullopt;

	if(cachePos != pos || std::chrono::steady_clock::now() > cachedAt + CacheTime) {
		// The cache does not have this data, go get it
		const uint64_t currentSector = pos / SectorSize;
		auto msg = com.waitForMessageSync([&currentSector, &com] {
			return com.sendCommand(Command::NeoReadMemory, {
				MemoryTypeSD,
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
			return nullopt;
		}

		memcpy(cache.data(), sdmsg->data.data(), SectorSize);
		cachedAt = std::chrono::steady_clock::now();
		cachePos = pos;
	}

	memcpy(into, cache.data(), SectorSize);
	return SectorSize;
}

optional<uint64_t> NeoMemoryDiskDriver::writeLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, const uint8_t* atomicBuf, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout) {

	static std::shared_ptr<MessageFilter> NeoMemoryDone = std::make_shared<MessageFilter>(Network::NetID::NeoMemoryWriteDone);

	if(pos % SectorSize != 0)
		return nullopt;

	if(amount != SectorSize)
		return nullopt;

	// Clear the cache if we're writing to the cached sector
	if(pos == cachePos)
		cachedAt = std::chrono::time_point<std::chrono::steady_clock>();

	// Requesting an atomic operation, but neoMemory does not support it
	// Continue on anyway but warn the caller
	if(atomicBuf != nullptr)
		report(APIEvent::Type::AtomicOperationCompletedNonatomically, NonatomicSeverity);

	const uint64_t currentSector = pos / SectorSize;
	auto msg = com.waitForMessageSync([&currentSector, &com, from, amount] {
		std::vector<uint8_t> command = {
			MemoryTypeSD,
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
		return nullopt;

	return SectorSize;
}