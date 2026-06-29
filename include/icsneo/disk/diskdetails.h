#ifndef __DISKDETAILS_H__
#define __DISKDETAILS_H__

#ifdef __cplusplus

#include <cstdint>
#include <vector>
#include <icsneo/api/eventmanager.h>
#include <icsneo/icsneoc2types.h>

namespace icsneo {

enum class DiskLayout : uint8_t {
	Spanned = icsneoc2_disk_layout_spanned,
	RAID0 = icsneoc2_disk_layout_raid0
};

struct DiskInfo {
	bool present; // Disk is connected
	bool initialized; // Disk is initialized
	// getDiskDetails: disk is formatted | formatDisk: disk to format | forceDiskConfigUpdate: disk enabled in layout (also reported as formatted)
	bool formatted;

	uint64_t sectors;
	uint32_t bytesPerSector;
	
	uint64_t size() const {
		return sectors * bytesPerSector;
	}
};

struct DiskDetails {
	DiskLayout layout;
	bool fullFormat;
	std::vector<DiskInfo> disks;

	static std::vector<uint8_t> Encode(const DiskDetails& config);
	static std::shared_ptr<DiskDetails> Decode(const std::vector<uint8_t>& bytes, size_t diskCount, device_eventhandler_t report);
};


} // namespace icsneo

#endif // __cplusplus

#endif