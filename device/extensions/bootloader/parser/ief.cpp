#include "icsneo/device/extensions/bootloader/parser/ief.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <cstring>

namespace icsneo {

namespace Bootloader {

/**
 * IEF file format (all integers little-endian):
 *
 *   uint8_t  magic                0xAA
 *   Version  version
 *   uint8_t  configLength
 *   uint8_t  config[configLength] Unused
 *   Then sectors until the end of the file:
 *     uint8_t  magic              0xAA
 *     uint32_t index              Sector index, see the communication for how this maps to an address
 *     uint32_t length
 *     uint8_t  data[length]
 */
bool IEF::load(std::vector<uint8_t>&& bytes) {
	static constexpr uint8_t Magic = 0xAA;
	// Magic, version, config byte count
	static constexpr size_t FixedHeaderSize = 1 + sizeof(Version) + 1;
	// Magic, index, size
	static constexpr size_t SectorHeaderSize = 1 + sizeof(uint32_t) + sizeof(uint32_t);

	data = std::move(bytes);
	sectors.clear();
	if(data.size() < FixedHeaderSize) {
		Log::Error(__func__, "IEF too short");
		return false;
	}

	if(data.front() != Magic) {
		Log::Error(__func__, "Invalid IEF magic");
		return false;
	}

	std::memcpy(&version, data.data() + 1, sizeof(Version));

	const size_t headerSize = FixedHeaderSize + data[FixedHeaderSize - 1];
	if(data.size() <= headerSize) {
		Log::Error(__func__, "Invalid IEF config bytes");
		return false;
	}

	size_t offset = headerSize;
	while(data.size() - offset >= SectorHeaderSize) {
		if(data[offset] != Magic) {
			Log::Error(__func__, "Invalid IEF sector magic");
			return false;
		}
		uint32_t index;
		uint32_t size;
		std::memcpy(&index, data.data() + offset + 1, sizeof(index));
		std::memcpy(&size, data.data() + offset + 1 + sizeof(index), sizeof(size));
		offset += SectorHeaderSize;
		if(data.size() - offset < size) {
			Log::Error(__func__, "Invalid IEF sector");
			return false;
		}

		sectors.emplace_back(index, BytesView(data.data() + offset, size));
		offset += size;
	}
	return true;
}

} // namespace Bootloader

} // namespace icsneo
