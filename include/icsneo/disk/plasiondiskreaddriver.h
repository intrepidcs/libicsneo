#ifndef __PLASIONDISKREADDRIVER_H__
#define __PLASIONDISKREADDRIVER_H__

#ifdef __cplusplus

#include "icsneo/disk/diskreaddriver.h"
#include <limits>
#include <chrono>

namespace icsneo {

namespace Disk {

/**
 * A disk read driver which uses the PLASION HID command set to read from the disk
 */
class PlasionDiskReadDriver : public ReadDriver {
public:
	std::pair<uint32_t, uint32_t> getBlockSizeBounds() const override {
		static_assert(SectorSize <= std::numeric_limits<uint32_t>::max(), "Incorrect sector size");
		static_assert(SectorSize >= std::numeric_limits<uint32_t>::min(), "Incorrect sector size");
		return { static_cast<uint32_t>(SectorSize), static_cast<uint32_t>(MaxSize) };
	}

private:
	static constexpr const uint32_t MaxSize = 65024;

	Access getPossibleAccess() const override { return Access::EntireCard; }

	std::optional<uint64_t> readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) override;
};

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus
#endif // __NEOMEMORYDISKREADDRIVER_H__