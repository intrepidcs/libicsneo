#ifndef __BOOTLOADER_FWBIN_H_
#define __BOOTLOADER_FWBIN_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"

#include <cstdint>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace icsneo {

namespace Bootloader {

class FWBin : public FWRepo {
public:
	FWBin() = default;

	bool load(const std::string& path);

	const FirmwareSourceVersion& getVersion() const override { return version; }
	bool getFW(const std::string& iefName, std::vector<uint8_t>& fw) const override;
	std::optional<icsneo::DeviceAppVersion> getChipVersion(const std::string& iefName) const override;

	static uint32_t HashDjb2(const std::string& str);

private:
	static constexpr const uint32_t SignatureHash = 0;

	struct VersionTags {
		const char* majorTag = nullptr;
		const char* minorTag = nullptr;

		operator bool() const {
			return majorTag && minorTag;
		}
	};

	static VersionTags GetVersionTags(const std::string& iefName);

	struct TagData {
		enum class Type : uint8_t {
			DataBytes = 0,
			DataLength = 1,
			IEFVersion = 2,
			Signature = 3,
			Unknown = 0xFF
		};

		Type type = Type::Unknown;
		std::streampos arrayPosition = 0;
		uint32_t arrayLength = 0;

		bool read(std::ifstream& stream, std::vector<uint8_t>& out) const;
	};

	bool readTag(const std::string& tag, std::vector<uint8_t>& out) const;

	mutable std::ifstream file;
	FirmwareSourceVersion version;
	std::map<uint32_t, TagData> tags;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_FWBIN_H_
