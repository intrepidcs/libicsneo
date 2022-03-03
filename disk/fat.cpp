#include "icsneo/disk/fat.h"
#include "icsneo/disk/diskdriver.h"
#include "ff.h"
#include "diskio.h"
#include <mutex>

using namespace icsneo;

// The FAT driver can only be accessed by one caller at a time, since it relies on globals
static std::mutex fatDriverMutex;
static std::function< optional<uint64_t>(uint64_t pos, uint8_t* into, uint64_t amount) > diskReadFn;

extern "C" DRESULT disk_read(BYTE, BYTE* buff, LBA_t sector, UINT count) {
	static_assert(Disk::SectorSize == 512, "FatFs expects 512 byte sectors");

	const uint64_t expected = count * uint64_t(Disk::SectorSize);
	const auto res = diskReadFn(sector * uint64_t(Disk::SectorSize), buff, expected);
	if (!res.has_value())
		return RES_NOTRDY;
	return res == expected ? RES_OK : RES_ERROR;
}

extern "C" DSTATUS disk_initialize(BYTE) {
	return RES_OK;
}

extern "C" DSTATUS disk_status(BYTE) {
	return RES_OK;
}

static uint64_t ClusterToSector(const FATFS& fs, DWORD cluster) {
	return fs.database + (LBA_t)fs.csize * (cluster - 2);
}

optional<uint64_t> Disk::FindVSAInFAT(std::function< optional<uint64_t>(uint64_t pos, uint8_t* into, uint64_t amount) > diskRead) {
	std::lock_guard<std::mutex> lk(fatDriverMutex);
	diskReadFn = diskRead;

	FATFS fs = {};
	if (f_mount(&fs, (const TCHAR*)_TEXT(""), 0) != FR_OK)
		return nullopt;

	FIL logData = {};
	if (f_open(&logData, (const TCHAR*)_TEXT("0:\\LOG_DATA.VSA"), FA_READ) != FR_OK)
		return nullopt;

	return ClusterToSector(fs, logData.obj.sclust) * uint64_t(Disk::SectorSize);
}