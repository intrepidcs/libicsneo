#ifndef __BOOTLOADER_API_H_
#define __BOOTLOADER_API_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/chip.h"
#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"
#include "icsneo/device/extensions/bootloader/parser/ief.h"
#include "icsneo/device/device.h"

#include <string>
#include <optional>

namespace icsneo {

namespace Bootloader {

using Logger = icsneo::Device::OpenStatusHandler;

bool requiresUpdate(icsneo::Device& device, const FWRepo& fw);
bool update(
	icsneo::Device& device,
	const FWRepo& fw,
	Logger logger = [](icsneo::Device::OpenStatusType, const std::string&, std::optional<double>) { return icsneo::Device::OpenDirective::Continue; },
	bool alreadyInBootloader = false,
	bool forceFlash = false
);

bool enterBootloader(icsneo::Device& device);
bool exitBootloader(icsneo::Device& device, Logger logger, bool updatedChips);
bool verifyConnection(icsneo::Device& device, Logger logger);

bool isSFPChip(const icsneo::Bootloader::Chip& chip);
bool isSFPIEF(const icsneo::Bootloader::IEF& ief);

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_API_H_
