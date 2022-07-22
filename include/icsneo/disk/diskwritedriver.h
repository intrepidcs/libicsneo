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
	 * Flag returned from writeLogicalDiskAligned when the
	 * operation failed to be performed atomically and can
	 * be retried after rereading.
	 */
	static const uint64_t RetryAtomic;

	/**
	 * The severity to report with when an atomic operation
	 * is requested that the driver is unable to attempt.
	 */
	static const APIEvent::Severity NonatomicSeverity;

	/**
	 * Perform a write which the driver can do in one shot.
	 * 
	 * The `pos` requested must be sector-aligned, and the `amount` must be
	 * within the block size bounds provided by the driver.
	 * 
	 * If `atomicBuf` is provided, it will be used to ensure that the disk
	 * data changes from `atomicBuf` to `from` without trampling any reads
	 * that may have happened while modifying the data.
	 * 
	 * The flag `RetryAtomic` is returned if the operation was attempted
	 * atomically but failed.
	 * 
	 * If the driver does not support atomic operations, but `atomicBuf`
	 * is non-null, an APIEvent::AtomicOperationCompletedNonatomically
	 * should be reported with `NonatomicSeverity`.
	 */
	virtual std::optional<uint64_t> writeLogicalDiskAligned(Communication& com, device_eventhandler_t report,
		uint64_t pos, const uint8_t* atomicBuf, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout) = 0;
};

} // namespace Disk

} // namespace icsneo

#endif // __cplusplus
#endif // __DISKWRITEDRIVER_H__