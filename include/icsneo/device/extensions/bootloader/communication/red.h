#ifndef __BOOTLOADER_RED_H_
#define __BOOTLOADER_RED_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/communication/redbase.h"

#include <cstddef>
#include <cstdint>

namespace icsneo {

namespace Bootloader {

/**
 * The bootloader communication protocol used for non-FPGA
 * chips on neoVI and ValueCAN devices.
 *
 * Zynq doesn't count as an FPGA chip for the sake of
 * this, so it's handled here.
 */
class REDCommunication : public REDCommunicationBase {
public:
	REDCommunication(icsneo::Device& dev, const ChipInfo& chip, bool auth, bool encryptWithDeviceKey = true);
	bool enterApplication() override { return sendCommand(uint8_t(Command::EnterApplication)); }
	bool flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) override;
	bool open() override;

private:
	const bool doAuthChecks;
	const bool encryptFirmwareWithDeviceKey;
	size_t blockSize = 2048;

	enum class Command : uint8_t {
		GetVersion = 0x12,
		Authenticate = 0x13,
		Flash = 0x14,
		ValidateFirmware = 0x16,
		Erase = 0x20,
		EnterApplication = 0x21,
		ValidateSWVersion = 0x27,
	};

	// The reply to Command::GetVersion
	struct VersionResponse {
		uint8_t bl_major, bl_minor, app_major, app_minor, flags, prod_id;
		uint8_t hardware_rev_major, hardware_rev_minor;
		uint8_t man_day, man_month;
		uint16_t man_year;
		uint8_t serial_num[8];
		uint16_t block_size;
		uint8_t last_error_code;
		uint8_t pad_byte;

		static constexpr size_t EncodedSize = 24;

		VersionResponse() = default;
		VersionResponse(const REDPacketizer::RxFrame& r);
	} currentVersion;

	// Called from open()
	bool authenticate(bool firstTime = true);
	bool getVersion();
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_RED_H_
