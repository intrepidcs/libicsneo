#include "icsneo/disk/extextractordiskreaddriver.h"
#include "icsneo/communication/message/diskdatamessage.h"

//#define ICSNEO_EXTENDED_EXTRACTOR_DEBUG_PRINTS
#ifdef ICSNEO_EXTENDED_EXTRACTOR_DEBUG_PRINTS
#include <iostream>
#endif

using namespace icsneo;
using namespace icsneo::Disk;

std::optional<uint64_t> ExtExtractorDiskReadDriver::readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout, MemoryType memType) {

	if(amount > getBlockSizeBounds().second)
		return std::nullopt;

	if(amount % getBlockSizeBounds().first != 0)
		return std::nullopt;

	if(pos % getBlockSizeBounds().first != 0)
		return std::nullopt;

	std::optional<uint64_t> ret;
	unsigned int attempts = 4;
	while (attempts-- > 0)
	{
		ret = attemptReadLogicalDiskAligned(com, report, pos, into, amount, timeout, memType);
		if (ret.has_value())
			break;
	}
	return ret;
}

std::optional<uint64_t> ExtExtractorDiskReadDriver::attemptReadLogicalDiskAligned(Communication& com, device_eventhandler_t report,
	uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout, MemoryType) {
	static std::shared_ptr<MessageFilter> NeoMemorySDRead = std::make_shared<MessageFilter>(Network::NetID::NeoMemorySDRead);

	uint64_t sector = pos / SectorSize;

	uint64_t largeSectorCount = amount / SectorSize;
	uint32_t sectorCount = uint32_t(largeSectorCount);
	if (largeSectorCount != uint64_t(sectorCount))
		return std::nullopt;

	std::condition_variable cv;
	std::mutex mutex;
	uint8_t* intoOffset = into;
	int64_t remaining = amount;
	bool complete = false;

	const auto handle = com.addMessageCallback(std::make_shared<MessageCallback>([&](std::shared_ptr<Message> message) {
		if(remaining > 0) {
			const auto diskdata = std::static_pointer_cast<DiskDataMessage>(message);

			const auto& data = diskdata->data;
			std::copy(data.data(), data.data() + data.size(), intoOffset);

			intoOffset += data.size();
			remaining -= data.size();

			if(remaining == 0) {
				{
					std::scoped_lock<std::mutex> lk(mutex);
					complete = true;
				}
				cv.notify_all();
			}
		}
	}, std::make_shared<MessageFilter>(Network::NetID::DiskData)));

	if(!com.sendCommand(ExtendedCommand::Extract, {
		uint8_t(sector & 0xff),
		uint8_t((sector >> 8) & 0xff),
		uint8_t((sector >> 16) & 0xff),
		uint8_t((sector >> 24) & 0xff),
		uint8_t((sector >> 32) & 0xff),
		uint8_t((sector >> 40) & 0xff),
		uint8_t((sector >> 48) & 0xff),
		uint8_t((sector >> 56) & 0xff),
		uint8_t(sectorCount & 0xff),
		uint8_t((sectorCount >> 8) & 0xff),
		uint8_t((sectorCount >> 16) & 0xff),
		uint8_t((sectorCount >> 24) & 0xff),
	})) {
		return std::nullopt;
	}

	std::unique_lock<std::mutex> lk(mutex);
	const auto successful = cv.wait_for(lk, timeout, [&](){ return complete; });
	com.removeMessageCallback(handle);

	if(!successful)
		return std::nullopt;

	return amount - remaining;
}
