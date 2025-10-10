#ifndef __CHIP_INFO_H_
#define __CHIP_INFO_H_

#ifdef __cplusplus

#include "icsneo/device/chipid.h"

#include <vector>

namespace icsneo {

enum class FirmwareType {
	IEF,
	Zip
};

struct ChipInfo {
	/**
	 * The default enabled field is for devices which can't deduce the specific ChipID of a device
	 * until entering the bootloader.
	 * 
	 * If defaultEnabled is set to true, the bootloader and version retrieval functions assume that
	 * the chip is enabled, so it will perform version deduction based on the default enabled chip. If
	 * defaultEnabled is set to false, then this chip info may or may not apply to this device
	 * 
	 * Example: RADA2B RevA and RevB chips
	 * 
	 * Sometimes we can't deduce whether we have a RevA or RevB chip before entering the bootloader if the
	 * device does not support component versions. These chips track the same version, so we assume that the
	 * chip is RevA (RevA defaultEnabled=true, RevB defaultEnabled=false) and check during the bootloader 
	 * if this chip is RevA or RevB.
	 */
	ChipID id;
	bool defaultEnabled = false;
	const char* name = nullptr; // Chip user displayable name
	const char* iefName = nullptr; // IEF name for a single segment chip (example RADA2B ZCHIP)
	size_t versionIndex = 0; // Main and Secondary version index
	FirmwareType fwType; // Firmwaare storagae type
	std::vector<const char*> iefSegments; // IEF names for a multi segment chip (example RADGalaxy2 ZCHIP)

	ChipInfo() = default;

	ChipInfo(ChipID id, bool defaultEnabled, const char* name, const char* iefName, size_t versionIndex, FirmwareType fwType)
		: id(id), defaultEnabled(defaultEnabled), name(name), iefName(iefName), versionIndex(versionIndex), fwType(fwType) {}

	ChipInfo(ChipID id, bool defaultEnabled, const char* name, const char* iefName, const std::vector<const char*>& iefSegments, size_t versionIndex, FirmwareType fwType)
		: id(id), defaultEnabled(defaultEnabled), name(name), iefName(iefName), versionIndex(versionIndex), fwType(fwType), iefSegments(iefSegments) {}

	bool isMultiIEF() const {
		return !iefSegments.empty();
	}

};

}

#endif // __cplusplus

#endif