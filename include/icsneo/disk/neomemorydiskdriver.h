#ifndef __NEOMEMORYDISKDRIVER_H__
#define __NEOMEMORYDISKDRIVER_H__

#ifdef __cplusplus

#include "icsneo/disk/diskreaddriver.h"
#include "icsneo/disk/diskwritedriver.h"
#include <limits>
#include <chrono>

namespace icsneo {

namespace Disk {

/**
 * A disk driver which uses the neoMemory command to read from or write to the disk
 * 
 * This can only make requests per sector, so it will be very slow, but is likely supported by any device with a disk
 */
class NeoMemoryDiskDriver : public ReadDriver, public WriteDriver {
public:
	std::pair<uint32_t, uint32_t> getBlockSizeBounds() const override {
		static_assert(SectorSize <= std::numeric_limits<uint32_t>::max(), "Incorrect sector size");
		static_assert(SectorSize >= std::numeric_limits<uint32_t>::min(), "Incorrect sector size");
		return { static_cast<uint32_t>(SectorSize), static_cast<uint32_t>(SectorSize) };
	}

private:
	static constexpr const uint8_t MemoryTypeSD = 0x01; // Logical Disk

	Access getPossibleAccess() const override { return Access::VSA; }

	std::optional<uint64_t> readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) override;
	
	std::optional<uint64_t> writeLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout) override;
};

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus
#endif // __NEOMEMORYDISKDRIVER_H__