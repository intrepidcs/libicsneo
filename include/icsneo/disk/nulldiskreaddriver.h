#ifndef __NULLDISKREADDRIVER_H__
#define __NULLDISKREADDRIVER_H__

#ifdef __cplusplus

#include "icsneo/disk/diskreaddriver.h"

namespace icsneo {

/**
 * A disk driver which always returns the requested disk as unsupported
 * 
 * Used for devices which do not have a disk, or do not provide any means for accessing it
 */
class NullDiskReadDriver : public DiskReadDriver {
public:
	optional<uint64_t> readLogicalDisk(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout = DefaultTimeout) override;
	Access getAccess() const override { return Access::None; }
	std::pair<uint32_t, uint32_t> getBlockSizeBounds() const override { return {SectorSize, SectorSize}; }

private:
	optional<uint64_t> readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) override;
};

}

#endif // __cplusplus
#endif // __NULLDISKREADDRIVER_H__