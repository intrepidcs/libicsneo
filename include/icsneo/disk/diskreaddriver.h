#ifndef __DISKREADDRIVER_H__
#define __DISKREADDRIVER_H__

#ifdef __cplusplus

#include "icsneo/communication/communication.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/disk/diskdriver.h"
#include <optional>
#include <cstdint>
#include <chrono>

namespace icsneo {

namespace Disk {

/**
 * Interface for drivers which read block data from devices
 */
class ReadDriver : public virtual Driver {
public:
	virtual std::optional<uint64_t> readLogicalDisk(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout = DefaultTimeout, MemoryType memType = MemoryType::SD);

	void invalidateCache(uint64_t pos = 0,
		uint64_t amount = std::numeric_limits<uint32_t>::max() /* large value, but avoid overflow */, MemoryType memType = MemoryType::SD);

protected:
	/**
	 * Perform a read which the driver can do in one shot.
	 * 
	 * The `pos` requested must be sector-aligned, and the `amount` must be
	 * within the block size bounds provided by the driver.
	 */
	virtual std::optional<uint64_t> readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout, MemoryType memType = MemoryType::SD) = 0;

private:
	std::vector<uint8_t> cacheSD;
	std::vector<uint8_t> cacheEEPROM;

	uint64_t cachePosSD = 0;
	uint64_t cachePosEEPROM = 0;
	std::chrono::time_point<std::chrono::steady_clock> cachedAt;

	static constexpr const std::chrono::milliseconds CacheTime = std::chrono::milliseconds(1000);

	std::optional<uint64_t> readFromCache(uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds staleAfter = CacheTime, MemoryType memType = MemoryType::SD);
};

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus
#endif // __DISKREADDRIVER_H__