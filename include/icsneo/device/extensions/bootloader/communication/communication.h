#ifndef __BOOTLOADER_COMMUNICATION_H_
#define __BOOTLOADER_COMMUNICATION_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/chip.h"
#include "icsneo/device/device.h"

#include <cstdint>

namespace icsneo {

namespace Bootloader {

class FWRepo;

/**
 * A bootloader protocol for one chip.
 *
 * Authentication is a challenge/response handled by libicsbl: icsbl_generate_seed_v1() derives
 * a challenge (seed.a) and the reply the device must send back (seed.b) from the serial number.
 * Firmware is encrypted for a specific device with icsbl_encrypt_v1(), also keyed by the serial number.
 */
class Communication {
public:
	Communication(icsneo::Device& dev) : device(dev) {}
	virtual ~Communication() = default;

	virtual bool enterApplication() = 0;
	virtual bool hasFinalizationStep() const = 0;
	virtual bool finalize() = 0;
	virtual bool flash(const FWRepo& repo, const icsneo::Device::OpenStatusHandler& handler) = 0;

	virtual bool open() = 0;
	virtual bool close() = 0;

protected:
	static uint32_t readLE32(const uint8_t* data);
	static void writeLE32(uint8_t* data, uint32_t val);
	icsneo::Device& device;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_COMMUNICATION_H_
