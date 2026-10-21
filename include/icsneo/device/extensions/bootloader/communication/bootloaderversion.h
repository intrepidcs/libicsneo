#ifndef __BOOTLOADER_BOOTLOADERVERSION_H_
#define __BOOTLOADER_BOOTLOADERVERSION_H_

#ifdef __cplusplus

#include <cstdint>

namespace icsneo {

namespace Bootloader {

#pragma pack(push, 1)
struct BootloaderVersionResponse {
	uint8_t blMajor;
	uint8_t blMinor;
	uint16_t reserved1;
	uint8_t hwMajor;
	uint8_t hwMinor;
	uint8_t hwPlatform;
	uint16_t reserved2;
	uint8_t manDay;
	uint8_t manMonth;
	uint16_t manYear;
	uint8_t serialNum[8];
	uint8_t hwId;
	uint8_t reserved3;
	uint8_t zynqId;
};
#pragma pack(pop)

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_BOOTLOADERVERSION_H_
