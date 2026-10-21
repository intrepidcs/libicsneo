#ifndef __BOOTLOADER_FWBUNDLE_H_
#define __BOOTLOADER_FWBUNDLE_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace icsneo {

namespace Bootloader {

class FWBundle : public FWRepo {
public:
	FWBundle() = default;

	bool load(const std::string& path);
	static bool IsFWBundle(const std::string& path);

	const FirmwareSourceVersion& getVersion() const override { return version; }
	bool getFW(const std::string& iefName, std::vector<uint8_t>& fw) const override;
	std::optional<icsneo::DeviceAppVersion> getChipVersion(const std::string& iefName) const override;

private:
	struct ChipBinary {
		std::vector<uint8_t> data;
		FirmwareSourceVersion version;
	};

	FirmwareSourceVersion version;
	std::unordered_map<uint32_t, ChipBinary> binaries;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_FWBUNDLE_H_
