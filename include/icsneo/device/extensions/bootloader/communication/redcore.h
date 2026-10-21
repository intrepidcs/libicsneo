#ifndef __BOOTLOADER_REDCORE_H_
#define __BOOTLOADER_REDCORE_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/communication/redbase.h"

#include <cstdint>
#include <vector>

namespace icsneo {

namespace Bootloader {

/**
 * The bootloader communication protocol used for FPGA
 * chips on neoVI and ValueCAN devices.
 *
 * Zynq doesn't count as an FPGA chip for the sake of
 * this, it's just targetting the non-hybrid FPGAs.
 *
 * This was previously called FIRE 2 Core Flasher, but
 * it does more than just FIRE 2 nowadays.
 */
class REDCoreCommunication : public REDCommunicationBase {
public:
	using REDCommunicationBase::REDCommunicationBase;

	bool enterApplication() override { return false; }
	bool flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) override;
private:
	/**
	 * Core flasher commands are tunneled through a RED request using CoreFlasherCommand,
	 * the arguments are CoreFlasherMagic, a 16-bit little-endian length, then the core command and its arguments
	 */
	static constexpr uint8_t CoreFlasherCommand = 0xCD;
	static constexpr uint8_t CoreFlasherMagic = 0xEF;

	enum class Command : uint8_t {
		SendFirmware = 0,
		StartFirmwareFlash = 1,
		GetFlashProgress = 2,
	};

	bool sendCommandRaw(uint8_t cmd, std::vector<uint8_t> args = {}) override;
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_REDCORE_H_
