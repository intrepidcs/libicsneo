#include "icsneo/device/extensions/bootloader/parser/fwbin.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"
#include "icsneo/device/extensions/bootloader/parser/ief.h"

#include <unordered_map>

namespace icsneo {

namespace Bootloader {

/**
 * fw.bin file format (all integers little-endian):
 *
 *   uint8_t  versionLength
 *   char     version[versionLength]  Vehicle Spy 3 version, "major.minor.release.build"
 *   uint8_t  hashLength
 *   char     hash[hashLength]        Build hash, unused
 *   Then tags until the end of the file:
 *     uint32_t nameHash              HashDjb2() of the tag name, e.g. "fire2_core"
 *     uint8_t  type                  TagData::Type
 *     uint32_t length
 *     uint8_t  data[length]
 *
 * Exactly one Signature tag is required, it uses the reserved name hash 0.
 */

uint32_t FWBin::HashDjb2(const std::string& tag) {
	uint32_t retv = 5381;
	for(char c : tag)
		retv = ((retv << 5) + retv) + c;
	return retv;
}

bool FWBin::load(const std::string& path) {
	file.open(path, std::ios::binary);
	if(!file.is_open() || file.fail() || file.eof()) {
		Log::Error(__func__, "Could not open file at " + path);
		return false;
	}
	// Read out the version string, it is a single byte length followed by a string
	uint8_t size;
	file.read((char*)&size, 1);
	if(file.fail() || file.eof()) {
		Log::Error(__func__, "The fw.bin is truncated, could not read the version length");
		return false;
	}
	std::string unparsedVersion;
	unparsedVersion.resize(size);
	file.read(unparsedVersion.data(), size);
	if(file.fail() || file.eof()) {
		Log::Error(__func__, "The fw.bin is truncated, could not read the version string");
		return false;
	}

	if(!version.load(unparsedVersion)) {
		Log::Error(__func__, "The fw.bin version string is invalid: " + unparsedVersion);
		return false;
	}

	// Read out the hash string, it is a single byte length followed by a string
	file.read((char*)&size, 1);
	if(file.fail() || file.eof()) {
		Log::Error(__func__, "The fw.bin is truncated, could not read the hash length");
		return false;
	}

	// The hash string is not used, skip over it
	file.seekg(size, file.cur);
	if(file.fail() || file.eof()) {
		Log::Error(__func__, "The fw.bin is truncated, could not read the hash string");
		return false;
	}

	while(true) {
		uint32_t tagHash;
		file.read((char*)&tagHash, sizeof(uint32_t));
		if(file.eof()) {
			break;
		}
		if(file.fail()) {
			Log::Error(__func__, "The fw.bin is truncated, could not read a tag name hash");
			return false;
		}

		if(tags.count(tagHash)) {
			// Duplicate tag
			Log::Error(__func__, "The fw.bin contains a duplicate tag");
			return false;
		}
		TagData& current = tags[tagHash];

		// Read the type field
		file.read((char*)&current.type, sizeof(TagData::Type));
		if(file.fail() || file.eof()) {
			Log::Error(__func__, "The fw.bin is truncated, could not read a tag type");
			return false;
		}

		switch(current.type) {
			case TagData::Type::DataBytes:
			case TagData::Type::DataLength:
			case TagData::Type::IEFVersion:
				if(tagHash == SignatureHash) {
					Log::Error(__func__, "The fw.bin contains a data tag using the reserved signature name hash");
					return false;
				}
				break;
			case TagData::Type::Signature:
				if(tagHash != SignatureHash) {
					Log::Error(__func__, "The fw.bin contains a signature tag with a non-signature name hash");
					return false;
				}
				break;
			default: // Unknown type
				Log::Error(__func__, "The fw.bin contains a tag of unknown type " + std::to_string(static_cast<int>(current.type)));
				return false;
		}

		// Read the length field
		file.read((char*)&current.arrayLength, sizeof(uint32_t));
		if(file.fail() || file.eof()) {
			Log::Error(__func__, "The fw.bin is truncated, could not read a tag length");
			return false;
		}

		// Note the current position so we can jump back to it and read it later
		current.arrayPosition = file.tellg();
		file.seekg(current.arrayLength, file.cur);
		if(file.fail()) {
			Log::Error(__func__, "The fw.bin is truncated, a tag extends past the end of the file");
			return false;
		}
	}
	file.clear();

	// The signature tag is required to be present, but it is not verified
	if(tags.count(SignatureHash) == 0) {
		Log::Error(__func__, "The fw.bin is missing its signature tag");
		return false;
	}
	return true;
}

bool FWBin::readTag(const std::string& tag, std::vector<uint8_t>& out) const {
	const auto hash = HashDjb2(tag);
	if(hash == SignatureHash) {
		Log::Error(__func__, "The signature tag can not be read as data");
		return false;
	}
	const auto it = tags.find(hash);
	if(it == tags.end()) {
		Log::Error(__func__, "The fw.bin does not contain the tag " + tag);
		return false;
	}
	return it->second.read(file, out);
}

bool FWBin::TagData::read(std::ifstream& stream, std::vector<uint8_t>& out) const {
	stream.clear();
	stream.seekg(arrayPosition, stream.beg);
	out.resize(arrayLength);
	stream.read((char*)out.data(), arrayLength);
	if(stream.fail()) {
		Log::Error(__func__, "The fw.bin is truncated, could not read tag data");
		return false;
	}
	return true;
}

bool FWBin::getFW(const std::string& iefName, std::vector<uint8_t>& fw) const {
	return readTag(iefName, fw);
}

FWBin::VersionTags FWBin::GetVersionTags(const std::string& iefName) {
	static const std::unordered_map<std::string, VersionTags> VersionTable = {
		{"fire2_core", {"fire2_core_version_major", "fire2_core_version_minor"}},
		{"fire2_core_sg4", {"fire2_core_sg4_version_major", "fire2_core_sg4_version_minor"}},
		{"RADA2B_SW_bin", {"RADA2B_SW_Version_Major", "RADA2B_SW_Version_Minor"}},
		{"RADA2B_REVB_SW_bin", {"RADA2B_SW_Version_Major", "RADA2B_SW_Version_Minor"}},
		{"RADComet_SW_bin", {"RADComet_SW_Version_Major", "RADComet_SW_Version_Minor"}},
		{"RADComet3_SW_bin", {"RADComet3_SW_Version_Major", "RADComet3_SW_Version_Minor"}},
		{"RADGalaxy_SG2_SW_bin", {"RADGalaxy_SG2_SW_Version_Major", "RADGalaxy_SG2_SW_Version_Minor"}},
		{"RADGalaxy_SG2_FFG_SW_bin", {"RADGalaxy_SG2_FFG_SW_Version_Major", "RADGalaxy_SG2_SW_Version_Minor"}},
		{"RADGalaxy2_SW_bin_p1", {"RADGalaxy2_SW_Version_Major", "RADGalaxy2_SW_Version_Minor"}},
		{"RADGalaxy2_SW_bin_p2", {"RADGalaxy2_SW_Version_Major", "RADGalaxy2_SW_Version_Minor"}},
		{"RADGigastar_SW_bin", {"RADGigastar_SW_Version_Major", "RADGigastar_SW_Version_Minor"}},
		{"RADGigastar_FFG_SW_bin", {"RADGigastar_SW_Version_Major", "RADGigastar_SW_Version_Minor"}},
		{"RADGigastar_USBz_SW_bin", {"RADGigastar_USBz_SW_Version_Major", "RADGigastar_USBz_SW_Version_Minor"}},
		{"RADGigastar_USBz_Z7010_SW_bin", {"RADGigastar_USBz_SW_Version_Major", "RADGigastar_USBz_SW_Version_Minor"}},
		{"RADGigastar_USBz_Z7007s_SW_bin", {"RADGigastar_USBz_SW_Version_Major", "RADGigastar_USBz_SW_Version_Minor"}},
		{"RADGigastar2_T1S_LIN_SW_bin", {"RADGigastar2_T1S_LIN_SW_Version_Major", "RADGigastar2_T1S_LIN_SW_Version_Minor"}},
		{"RADGigastar2_T1S_CAN_SW_bin", {"RADGigastar2_T1S_CAN_SW_Version_Major", "RADGigastar2_T1S_CAN_SW_Version_Minor"}},
		{"RADMoon2_SW_bin", {"RADMoon2_SW_Version_Major", "RADMoon2_SW_Version_Minor"}},
		{"RADMoon2_Z7010_SW_bin", {"RADMoon2_SW_Version_Major", "RADMoon2_SW_Version_Minor"}},
		{"RADMoonT1S_SW_bin", {"RADMoonT1S_SW_Version_Major", "RADMoonT1S_SW_Version_Minor"}},
		{"vcan44_core_ief", {"vcan44_core_version_major", "vcan44_core_version_minor"}}
	};
	auto iter = VersionTable.find(iefName);
	return iter != VersionTable.end() ? iter->second : VersionTags();
}

std::optional<icsneo::DeviceAppVersion> FWBin::getChipVersion(const std::string& iefName) const {
	auto versionTags = GetVersionTags(iefName);

	if(versionTags) {
		std::vector<uint8_t> majData;
		std::vector<uint8_t> minData;

		if(!readTag(versionTags.majorTag, majData)) {
			return std::nullopt;
		}

		if(!readTag(versionTags.minorTag, minData)) {
			return std::nullopt;
		}

		if(majData.size() != 1)
			return std::nullopt;

		if(minData.size() != 1)
			return std::nullopt;

		icsneo::DeviceAppVersion chipVersion;
		chipVersion.major = majData[0];
		chipVersion.minor = minData[0];
		return chipVersion;
	}

	std::vector<uint8_t> buf;

	if(!readTag(iefName, buf)) {
		return std::nullopt;
	}

	IEF ief;

	if(!ief.load(std::move(buf))) {
		return std::nullopt;
	}

	icsneo::DeviceAppVersion chipVersion;
	chipVersion.major = ief.getVersion().appMajor;
	chipVersion.minor = ief.getVersion().appMinor;
	return chipVersion;
}

} // namespace Bootloader

} // namespace icsneo
