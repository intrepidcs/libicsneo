#ifndef __NULLDISKREADDRIVER_H__
#define __NULLDISKREADDRIVER_H__

#ifdef __cplusplus

#include "icsneo/disk/diskreaddriver.h"
#include "icsneo/disk/diskwritedriver.h"
#include <limits>

namespace icsneo {

namespace Disk {

/**
 * A disk driver which always returns the requested disk as unsupported
 * 
 * Used for devices which do not have a disk, or do not provide any means for accessing it
 */
class NullDriver : public ReadDriver, public WriteDriver {
public:
	optional<uint64_t> readLogicalDisk(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout = DefaultTimeout) override;
	optional<uint64_t> writeLogicalDisk(Communication& com, device_eventhandler_t report, ReadDriver& readDriver,
		uint64_t pos, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout = DefaultTimeout) override;
	Access getAccess() const override { return Access::None; }
	std::pair<uint32_t, uint32_t> getBlockSizeBounds() const override {
		static_assert(SectorSize <= std::numeric_limits<uint32_t>::max(), "Incorrect sector size");
		static_assert(SectorSize >= std::numeric_limits<uint32_t>::min(), "Incorrect sector size");
		return { static_cast<uint32_t>(SectorSize), static_cast<uint32_t>(SectorSize) };
	}

private:
	optional<uint64_t> readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) override;
	optional<uint64_t> writeLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, const uint8_t* atomicBuf, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout) override;
};

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus
#endif // __NULLDISKREADDRIVER_H__