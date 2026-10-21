#ifndef __BOOTLOADER_VERSION_H_
#define __BOOTLOADER_VERSION_H_

#ifdef __cplusplus

#include <cstdint>
#include <string>

namespace icsneo {

namespace Bootloader {

/**
 * Tracks the Vehicle Spy 3 version that a firmware
 * package is intended for.
 */
struct FirmwareSourceVersion {
	using ValueType = uint8_t;
	ValueType major = 0;
	ValueType minor = 0;
	ValueType release = 0;
	ValueType build = 0;
	bool load(const std::string& toParse);
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_VERSION_H_
