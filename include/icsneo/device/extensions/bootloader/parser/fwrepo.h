#ifndef __BOOTLOADER_FWREPO_H_
#define __BOOTLOADER_FWREPO_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/version.h"
#include "icsneo/device/deviceversion.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace icsneo {

namespace Bootloader {

class FWRepo {
public:
	FWRepo() = default;
	virtual ~FWRepo() = default;

	virtual bool getFW(const std::string& iefName, std::vector<uint8_t>& fw) const = 0;
	virtual std::optional<icsneo::DeviceAppVersion> getChipVersion(const std::string& iefName) const = 0;
	virtual const FirmwareSourceVersion& getVersion() const = 0;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_FWREPO_H_
