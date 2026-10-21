#include "icsneo/device/extensions/bootloader/parser/fwbundle.h"
#include "icsneo/device/extensions/bootloader/parser/fwbin.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"
#include "icsneo/core/crc32.h"
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>

namespace icsneo {

namespace Bootloader {

/**
 * Firmware bundle file format (all integers little-endian):
 *
 *   FWBundleHeader
 *   Then chips until the end of the file:
 *     FWBundleChip
 *     uint8_t data[FWBundleChip::length]
 *
 * The CRC32 covers the whole file with the crc field set to zero.
 */
#pragma pack(push, 1)
struct FWBundleHeader {
	char magic[2]; // "FW"
	uint16_t fmtVersion;
	uint32_t crc;
	uint8_t build;
	uint8_t maintenance;
	uint8_t minor;
	uint8_t major;
	uint8_t type; // Build type, unused
};

struct FWBundleChip {
	uint32_t hash; // FWBin::HashDjb2() of the chip's IEF name
	uint8_t build;
	uint8_t maintenance;
	uint8_t minor;
	uint8_t major;
	uint32_t length;
};
#pragma pack(pop)

bool FWBundle::IsFWBundle(const std::string& path) {
	std::ifstream file(path, std::ios::binary);
	if(!file.is_open())
		return false;
	// Only check the magic, so a truncated bundle is still reported as a bundle error by load()
	char magic[sizeof(FWBundleHeader::magic)];
	file.read(magic, sizeof(magic));
	if(file.fail())
		return false;
	return magic[0] == 'F' && magic[1] == 'W';
}

bool FWBundle::load(const std::string& path) {
	std::ifstream file(path, std::ios::binary);
	if(!file.is_open()) {
		Log::Error(__func__, "Could not open file at " + path);
		return false;
	}
	std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	if(file.bad()) {
		Log::Error(__func__, "Could not read file at " + path);
		return false;
	}

	if(bytes.size() < sizeof(FWBundleHeader) || bytes.size() > std::numeric_limits<uint32_t>::max()) {
		Log::Error(__func__, "The firmware bundle is smaller than its header or larger than 4 GiB");
		return false;
	}

	FWBundleHeader header;
	std::memcpy(&header, bytes.data(), sizeof(header));
	if(header.magic[0] != 'F' || header.magic[1] != 'W') {
		Log::Error(__func__, "The file is not a firmware bundle, bad magic");
		return false;
	}

	// The CRC is computed with the CRC field zeroed
	std::memset(bytes.data() + offsetof(FWBundleHeader, crc), 0, sizeof(header.crc));
	if(icsneo::crc32(0, bytes.data(), static_cast<uint32_t>(bytes.size())) != header.crc) {
		Log::Error(__func__, "The firmware bundle failed CRC validation");
		return false;
	}

	std::unordered_map<uint32_t, ChipBinary> parsed;
	size_t offset = sizeof(FWBundleHeader);
	while(offset < bytes.size()) {
		if(bytes.size() - offset < sizeof(FWBundleChip)) {
			Log::Error(__func__, "The firmware bundle is truncated, could not read a chip header");
			return false;
		}
		FWBundleChip chip;
		std::memcpy(&chip, bytes.data() + offset, sizeof(chip));
		offset += sizeof(FWBundleChip);

		if(bytes.size() - offset < chip.length) {
			Log::Error(__func__, "The firmware bundle is truncated, a chip binary extends past the end of the file");
			return false;
		}

		auto& bin = parsed[chip.hash];
		bin.data.assign(bytes.begin() + offset, bytes.begin() + offset + chip.length);
		bin.version.major = chip.major;
		bin.version.minor = chip.minor;
		bin.version.release = chip.maintenance;
		bin.version.build = chip.build;
		offset += chip.length;
	}

	version.major = header.major;
	version.minor = header.minor;
	version.release = header.maintenance;
	version.build = header.build;
	binaries = std::move(parsed);
	return true;
}

bool FWBundle::getFW(const std::string& iefName, std::vector<uint8_t>& fw) const {
	const auto it = binaries.find(FWBin::HashDjb2(iefName));
	if(it == binaries.end()) {
		Log::Error(__func__, "The chip binary could not be found");
		return false;
	}
	fw = it->second.data;
	return true;
}

std::optional<icsneo::DeviceAppVersion> FWBundle::getChipVersion(const std::string& iefName) const {
	const auto it = binaries.find(FWBin::HashDjb2(iefName));
	if(it == binaries.end())
		return std::nullopt;
	return icsneo::DeviceAppVersion(it->second.version.major, it->second.version.minor);
}

} // namespace Bootloader

} // namespace icsneo
