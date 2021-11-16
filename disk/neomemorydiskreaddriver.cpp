#include "icsneo/disk/neomemorydiskreaddriver.h"
#include "icsneo/communication/message/neoreadmemorysdmessage.h"
#include <cstring>

using namespace icsneo;

optional<uint64_t> NeoMemoryDiskReadDriver::readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) {
	static std::shared_ptr<MessageFilter> NeoMemorySDRead = std::make_shared<MessageFilter>(Network::NetID::NeoMemorySDRead);

	if(amount != SectorSize)
		return 0;

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

	memcpy(into, sdmsg->data.data(), SectorSize);
	return SectorSize;
}