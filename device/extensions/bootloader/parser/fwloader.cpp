#include "icsneo/device/extensions/bootloader/parser/fwloader.h"
#include "icsneo/device/extensions/bootloader/parser/fwbin.h"
#include "icsneo/device/extensions/bootloader/parser/fwbundle.h"

namespace icsneo {

namespace Bootloader {

std::unique_ptr<FWRepo> FWLoader::Load(const std::string& path) {
	if(FWBundle::IsFWBundle(path)) {
		auto bundle = std::make_unique<FWBundle>();
		if(!bundle->load(path))
			return nullptr;
		return bundle;
	}

	auto bin = std::make_unique<FWBin>();
	if(!bin->load(path))
		return nullptr;
	return bin;
}

} // namespace Bootloader

} // namespace icsneo
