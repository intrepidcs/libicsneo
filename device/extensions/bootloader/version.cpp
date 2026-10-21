#include "icsneo/device/extensions/bootloader/version.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <limits>
#include <optional>
#include <regex>

namespace icsneo {

namespace Bootloader {

bool FirmwareSourceVersion::load(const std::string& toParse) {
	static const std::regex VersionRegex{"(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)"};
	std::smatch match;
	if(!std::regex_match(toParse, match, VersionRegex)) {
		Log::Error(__func__, "Failed to parse version");
		return false;
	}

	const auto parse = [](const std::string& input) -> std::optional<ValueType> {
		// The regex guarantees only digits, limit the length so stoul can not overflow
		if(input.size() > 3) {
			return std::nullopt;
		}
		const auto value = std::stoul(input);
		if(value > std::numeric_limits<ValueType>::max()) {
			return std::nullopt;
		}
		return static_cast<ValueType>(value);
	};

	const auto parsedMajor = parse(match[1]);
	const auto parsedMinor = parse(match[2]);
	const auto parsedRelease = parse(match[3]);
	const auto parsedBuild = parse(match[4]);
	if(!parsedMajor || !parsedMinor || !parsedRelease || !parsedBuild) {
		Log::Error(__func__, "Invalid sized version number");
		return false;
	}

	major = *parsedMajor;
	minor = *parsedMinor;
	release = *parsedRelease;
	build = *parsedBuild;
	return true;
}

} // namespace Bootloader

} // namespace icsneo
