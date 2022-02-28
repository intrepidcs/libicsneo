#ifndef __NEOMEMORYDISKREADDRIVER_H__
#define __NEOMEMORYDISKREADDRIVER_H__

#ifdef __cplusplus

#include "icsneo/disk/diskreaddriver.h"
#include <limits>
#include <chrono>

namespace icsneo {

namespace Disk {

/**
 * A disk read driver which uses the neoMemory command to read from the disk
 * 
 * This can only request reads by sector, so it will be very slow, but is likely supported by any device with a disk
 */
class NeoMemoryDiskReadDriver : public ReadDriver {
public:
	Access getAccess() const override { return Access::VSA; }
	std::pair<uint32_t, uint32_t> getBlockSizeBounds() const override {
		static_assert(SectorSize <= std::numeric_limits<uint32_t>::max(), "Incorrect sector size");
		static_assert(SectorSize >= std::numeric_limits<uint32_t>::min(), "Incorrect sector size");
		return { static_cast<uint32_t>(SectorSize), static_cast<uint32_t>(SectorSize) };
	}

private:
	static constexpr const uint8_t MemoryTypeSD = 0x01; // Logical Disk
	static constexpr const std::chrono::duration CacheTime = std::chrono::seconds(1);

	std::array<uint8_t, SectorSize> cache;
	uint64_t cachePos = 0;
	std::chrono::time_point<std::chrono::steady_clock> cachedAt;

	optional<uint64_t> readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) override;
};

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus
#endif // __NEOMEMORYDISKREADDRIVER_H__