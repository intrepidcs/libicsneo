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

enum class StatusType {
	Progress,
	Failed
};

/**
 * @brief Alias of callback function for progress of a bootloader pipeline task
 * 
 * The progress value passed into the callback function indicates the following:
 * 1) If the value is std::nullopt, this implies the status message is for an asynchronous task or a task in which the current progress is unknown.
 * 2) If the value is between 0 and 1 (i.e., 0 <= progress <= 1), this implies the actual progress of the current task as a percentage.
 * 3) Values not between 0 and 1 (i.e., progress < 0 or progress > 1) are undefined.
 */
using StatusHandler = std::function<void(StatusType type, const std::string& status, std::optional<double> progress)>;

bool requiresUpdate(icsneo::Device& device, const FWRepo& fw);
bool update(
	icsneo::Device& device,
	const FWRepo& fw,
	StatusHandler statusHandler = [](StatusType, const std::string&, std::optional<double>) {},
	bool alreadyInBootloader = false,
	bool forceFlash = false
);

bool enterBootloader(icsneo::Device& device);
bool exitBootloader(icsneo::Device& device, StatusHandler statusHandler, bool updatedChips);
bool verifyConnection(icsneo::Device& device, StatusHandler statusHandler);

bool isSFPChip(const icsneo::Bootloader::Chip& chip);
bool isSFPIEF(const icsneo::Bootloader::IEF& ief);

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_API_H_
