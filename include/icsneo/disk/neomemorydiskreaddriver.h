#ifndef __NEOMEMORYDISKREADDRIVER_H__
#define __NEOMEMORYDISKREADDRIVER_H__

#ifdef __cplusplus

#include "icsneo/disk/diskreaddriver.h"
#include <limits>

namespace icsneo {

/**
 * A disk read driver which uses the neoMemory command to read from the disk
 * 
 * This can only request reads by sector, so it will be very slow, but is likely supported by any device with a disk
 */
class NeoMemoryDiskReadDriver : public DiskReadDriver {
public:
	Access getAccess() const override { return Access::VSA; }
	std::pair<uint32_t, uint32_t> getBlockSizeBounds() const override {
		static_assert(SectorSize <= std::numeric_limits<uint32_t>::max(), "Incorrect sector size");
		static_assert(SectorSize >= std::numeric_limits<uint32_t>::min(), "Incorrect sector size");
		return { static_cast<uint32_t>(SectorSize), static_cast<uint32_t>(SectorSize) };
	}

private:
	static constexpr const uint8_t MemoryTypeSD = 0x01; // Logical Disk

	optional<uint64_t> readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) override;
};

}

#endif // __cplusplus
#endif // __NEOMEMORYDISKREADDRIVER_H__