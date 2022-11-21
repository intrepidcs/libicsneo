#ifndef __DISKWRITEDRIVER_H__
#define __DISKWRITEDRIVER_H__

#ifdef __cplusplus

#include "icsneo/communication/communication.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/disk/diskreaddriver.h"
#include <cstdint>
#include <chrono>
#include <optional>

namespace icsneo {

namespace Disk {

/**
 * Interface for drivers which write block data from devices
 */
class WriteDriver : public virtual Driver {
public:
	virtual std::optional<uint64_t> writeLogicalDisk(Communication& com, device_eventhandler_t report, ReadDriver& readDriver,
		uint64_t pos, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout = DefaultTimeout);

protected:
	/**
	 * Perform a write which the driver can do in one shot.
	 * 
	 * The `pos` requested must be sector-aligned, and the `amount` must be
	 * within the block size bounds provided by the driver.
	 */
	virtual std::optional<uint64_t> writeLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout) = 0;
};

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus
#endif // __DISKWRITEDRIVER_H__