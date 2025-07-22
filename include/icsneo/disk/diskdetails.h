#ifndef __DISKDETAILS_H__
#define __DISKDETAILS_H__

#ifdef __cplusplus

#include <cstdint>
#include <vector>
#include <icsneo/api/eventmanager.h>

namespace icsneo {

enum class DiskLayout : uint8_t {
	Spanned = 0,
	RAID0 = 1
};

struct DiskInfo {
	bool present; // Disk is connected
	bool initialized; // Disk is initialized
	bool formatted; // Disk is formatted

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