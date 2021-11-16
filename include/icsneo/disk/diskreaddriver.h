#ifndef __DISKREADDRIVER_H__
#define __DISKREADDRIVER_H__

#ifdef __cplusplus

#include "icsneo/platform/optional.h"
#include "icsneo/communication/communication.h"
#include "icsneo/api/eventmanager.h"
#include <cstdint>
#include <chrono>

namespace icsneo {

/**
 * Interface for drivers which read block data from devices
 */
class DiskReadDriver {
public:
	static constexpr const std::chrono::milliseconds DefaultTimeout{2000};
	static constexpr const size_t SectorSize = 512;
	enum class Access {
		None,
		EntireCard,
		VSA
	};

	virtual ~DiskReadDriver() = default;
	virtual optional<uint64_t> readLogicalDisk(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout = DefaultTimeout);
	virtual Access getAccess() const = 0;
	virtual std::pair<uint32_t, uint32_t> getBlockSizeBounds() const = 0;

protected:
	/**
	 * Perform a read which the driver can do in one shot.
	 * 
	 * The `pos` requested must be sector-aligned, and the `amount` must be
	 * within the block size bounds provided by the driver.
	 */
	virtual optional<uint64_t> readLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) = 0;
};

}

#endif // __cplusplus
#endif // __DISKREADDRIVER_H__