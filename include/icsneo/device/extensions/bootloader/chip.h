#ifndef __BOOTLOADER_CHIP_H_
#define __BOOTLOADER_CHIP_H_

#ifdef __cplusplus

#include "icsneo/device/chipinfo.h"

#include <cstdint>
#include <string>
#include <vector>

namespace icsneo {

namespace Bootloader {

struct Chip {
	uint8_t expansionSlot = 0;
	ChipInfo info;

	Chip(const ChipInfo& info) : info(info) {}

	ChipID getID() const { return info.id; }
	std::string getName() const { return info.name ? info.name : ""; }
	std::string getIEFName() const { return info.iefName ? info.iefName : ""; }
	const std::vector<const char*>& getMultiIEFName() const { return info.iefSegments; }
	FirmwareType getFirmwareType() const { return info.fwType; }
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_CHIP_H_
