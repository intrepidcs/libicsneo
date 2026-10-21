#ifndef __BOOTLOADER_FWLOADER_H_
#define __BOOTLOADER_FWLOADER_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"

#include <memory>
#include <string>

namespace icsneo {

namespace Bootloader {

class FWLoader {
public:
	// Detects whether the file is a firmware bundle or a fw.bin and loads it, returns nullptr on failure
	static std::unique_ptr<FWRepo> Load(const std::string& path);
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_FWLOADER_H_
