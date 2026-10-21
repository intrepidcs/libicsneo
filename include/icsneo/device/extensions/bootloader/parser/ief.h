#ifndef __BOOTLOADER_IEF_H_
#define __BOOTLOADER_IEF_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/bytesview.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace icsneo {

namespace Bootloader {

class IEF {
public:
	struct Version {
		uint8_t appMajor;
		uint8_t appMinor;
		uint8_t prodId;
		uint8_t chipId;
	};
	using Sector = std::pair<uint32_t, BytesView>;

	IEF() = default;

	bool load(std::vector<uint8_t>&& bytes);

	const Version& getVersion() const { return version; }
	const std::vector<Sector>& getSectors() const { return sectors; }
	// IEFs are comprised of equally sized blocks of data - take the size of the first
	uint32_t getBlockSize() const { return sectors.empty() ? 0 : static_cast<uint32_t>(sectors.front().second.size()); }

private:
	std::vector<uint8_t> data;
	Version version = {};
	std::vector<Sector> sectors;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_IEF_H_
