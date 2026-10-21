#include "icsneo/device/extensions/bootloader/communication/red.h"
#include "icsneo/device/extensions/bootloader/parser/ief.h"
#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <cstring>
#include <optional>

#include <icsbl/icsbl.h>

namespace icsneo {

namespace Bootloader {

REDCommunication::REDCommunication(icsneo::Device& dev, const ChipInfo& chip, bool auth, bool encryptWithDeviceKey)
	: REDCommunicationBase(dev, chip), doAuthChecks(auth), encryptFirmwareWithDeviceKey(encryptWithDeviceKey) {}

bool REDCommunication::open() {
	device.com->pauseReads();

	if(doAuthChecks) {
		if(!authenticate()) {
			return false;
		}
	}

	return getVersion();
}

bool REDCommunication::authenticate(bool firstTime) {
	icsbl_seed_v1 seed = {};
	if(!icsbl_generate_seed_v1(device.getSerialNumber(), &seed)) {
		Log::Error(__func__, "libicsbl could not generate an authentication challenge");
		return false;
	}

	// The challenge is sent as: uint8_t 0x09, uint16_t 0 (a length on top of the protocol length, always zero here),
	// one padding byte, then seed.a
	std::vector<uint8_t> challenge(3 + sizeof(seed.a));
	challenge[0] = 0x09;
	std::memcpy(&challenge[3], &seed.a, sizeof(seed.a));

	// Ask the bootloader to decode the seed
	Log::Debug(__func__, "Authenticating");
	if(!sendCommand(uint8_t(Command::Authenticate), challenge, std::chrono::seconds(1), false)) {
		if(firstTime) {
			// Attempt to flush out any buffers
			Log::Debug(__func__, "Authentication failed, flushing buffers");
			std::vector<uint8_t> flush(4096, PaddingByte);
			device.com->sendPacket(flush);

			// Allow any failure messages to come back
			const bool any = waitForRx(std::chrono::milliseconds(100));
			Log::Trace(__func__, any ? "A message came back" : "No communication came back");

			lastData.clear();

			// Now try again
			return authenticate(false); // Either way, we don't need to continue authenticating
		} else {
			Log::Error(__func__, "The bootloader rejected the authentication challenge twice");
			enterApplication();
			return false;
		}
	}

	// The reply is one byte followed by seed.b
	if(lastData.back().data.size() < 1 + sizeof(seed.b)) {
		Log::Error(__func__, "The authentication reply is too short");
		enterApplication();
		return false;
	}
	if(std::memcmp(&lastData.back().data[1], &seed.b, sizeof(seed.b)) != 0) {
		Log::Error(__func__, "The authentication reply does not match, the device did not authenticate");
		enterApplication();
		return false;
	}
	lastData.clear();

	return true;
}

bool REDCommunication::getVersion() {
	if(!sendCommand(uint8_t(Command::GetVersion), std::chrono::seconds(1), false)) {
		Log::Error(__func__, "The bootloader did not answer the version request");
		enterApplication();
		return false;
	}

	if(lastData.front().data.size() < VersionResponse::EncodedSize) {
		Log::Error(__func__, "The bootloader version reply is too short");
		enterApplication();
		return false;
	}

	currentVersion = VersionResponse(lastData.front());
	lastData.clear();

	if(doAuthChecks) {
		// Check that the serial number the bootloader reports is correct
		uint8_t correctSerial[sizeof(VersionResponse::serial_num)] = {};
		uint32_t devSerial = device.getSerialNumber();
		std::memcpy(correctSerial, &devSerial, sizeof(devSerial));
		if(std::memcmp(correctSerial, currentVersion.serial_num, sizeof(correctSerial))) {
			Log::Error(__func__, "The serial number reported by the bootloader does not match the device");
			enterApplication();
			return false;
		}
	}

	blockSize = currentVersion.block_size;
	device.setBootloaderVersion({currentVersion.bl_major, currentVersion.bl_minor});
	return true;
}

bool REDCommunication::flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) {
	const auto& srcVer = fw.getVersion();

	std::vector<uint8_t> bin;
	if(!fw.getFW(targetChip.getIEFName(), bin)) {
		return false;
	}

	IEF ief;
	if(!ief.load(std::move(bin))) {
		return false;
	}

	// The supplied IEF is not for this product
	if(ief.getVersion().prodId != static_cast<uint8_t>(device.getProductID())) {
		Log::Error(__func__, "IEF product ID mismatch for " + targetChip.getName());
		return false;
	}

	// The supplied IEF is for this product, but not for this chip
	if(ief.getVersion().chipId != static_cast<uint8_t>(targetChip.getID())) {
		Log::Error(__func__, "IEF chip ID mismatch for " + targetChip.getName());
		return false;
	}

	Log::Debug(__func__, "IEF version " + std::to_string(ief.getVersion().appMajor) + "." + std::to_string(ief.getVersion().appMinor)
		+ ", " + std::to_string(ief.getSectors().size()) + " sectors");

	if(doAuthChecks && device.supportsSwVersionValidate()) {
		// Tell the bootloader what version of Vehicle Spy 3 this firmware is from.
		// We check to make sure it approves, but also, some devices will not accept
		// new firmware if we have not sent this.
		// Sometimes new hardware revisions can not accept firmware which is too old.
		if(!sendCommand(uint8_t(Command::ValidateSWVersion), {srcVer.major, srcVer.minor, srcVer.release, srcVer.build})) {
			Log::Error(__func__, "SW version validation failed for " + targetChip.getName());
			return false;
		}
	}

	handler(icsneo::Device::OpenStatusType::Progress,
		"Preparing to update " + targetChip.getName() + "...", std::nullopt);
	Log::Debug(__func__, "Erasing " + targetChip.getName());
	if(!sendCommand(uint8_t(Command::Erase), std::chrono::seconds(45))) {
		Log::Error(__func__, "Erase failed for " + targetChip.getName());
		return false;
	}

	const auto& sectorPairs = ief.getSectors();
	size_t sectorsFlashed = 0;
	for(const auto& [id, sector] : sectorPairs) {
		Log::Trace(__func__, "Flashing sector " + std::to_string(id));
		handler(icsneo::Device::OpenStatusType::Progress,
			"Updating " + targetChip.getName() + "...",
			double(sectorsFlashed++) / sectorPairs.size());

		if(sector.size() > blockSize) {
			Log::Error(__func__, "Sector " + std::to_string(id) + " too large");
			return false;
		}

		// Sectors smaller than the block size are padded out with zeros
		std::vector<uint8_t> padded(sector.begin(), sector.end());
		padded.resize(blockSize, 0);

		std::vector<uint8_t> encrypted(padded.size());
		if(encryptFirmwareWithDeviceKey)
			icsbl_encrypt_v1(device.getSerialNumber(), padded.data(), padded.size(), encrypted.data());
		else
			encrypted = std::move(padded);

		encrypted.insert(encrypted.begin(), {
			0x00, 0x00, 0x00, // Padding bytes
			// Address
			uint8_t((id >> 0x00) & 0xFF),
			uint8_t((id >> 0x08) & 0xFF),
			uint8_t((id >> 0x10) & 0xFF),
			uint8_t((id >> 0x18) & 0xFF),
		});

		// Unencrypted firmware is used for FIRE2 Zynq (and VNET) which also uses the top bit of the ID
		// to denote whether the sector is flashed to chip A or chip B. That is indicated
		// with a 1 (for chip B) or a 0 as the first byte of the flash packet payload.
		if(!encryptFirmwareWithDeviceKey)
			encrypted.insert(encrypted.begin(), uint8_t(id & (1u << 31) ? 1 : 0));

		if(!sendCommand(uint8_t(Command::Flash), encrypted)) {
			Log::Error(__func__, "Flash failed at sector " + std::to_string(id) + " for " + targetChip.getName());
			return false;
		}
	}

	// Tell the application that we flashed 100%
	handler(icsneo::Device::OpenStatusType::Progress,
		"Updating " + targetChip.getName() + "...",
		1.0);

	handler(icsneo::Device::OpenStatusType::Progress, "Finalizing update...", std::nullopt);
	if(!sendCommand(uint8_t(Command::ValidateFirmware))) {
		Log::Error(__func__, "Firmware validation failed for " + targetChip.getName());
		return false;
	}
	return true;
}

REDCommunication::VersionResponse::VersionResponse(const REDPacketizer::RxFrame& r) {
	auto i = r.data.begin();
	bl_major = *i++;
	bl_minor = *i++;
	app_major = *i++;
	app_minor = *i++;
	hardware_rev_major = *i++;
	hardware_rev_minor = *i++;
	flags = *i++;
	block_size = *i++;
	block_size |= uint16_t(*i++) << 8;
	man_day = *i++;
	man_month = *i++;
	man_year = *i++;
	man_year |= uint16_t(*i++) << 8;
	for(size_t x = 0; x != sizeof(serial_num); ++x)
		serial_num[x] = *i++;
	prod_id = *i++;
	last_error_code = *i++;
	pad_byte = *i++;
}

} // namespace Bootloader

} // namespace icsneo
