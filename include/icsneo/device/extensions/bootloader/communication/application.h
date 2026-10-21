#ifndef __BOOTLOADER_APPLICATION_H_
#define __BOOTLOADER_APPLICATION_H_

#ifdef __cplusplus

#include "icsneo/device/extensions/bootloader/communication/communication.h"
#include "icsneo/device/extensions/bootloader/parser/ief.h"

#include <chrono>
#include <cstdint>
#include <functional>
#include <vector>

namespace icsneo {

namespace Bootloader {

/**
 * The software update protocol used with the application
 * on RED devices, does not require entering the bootloader.
 */
class Application : public Communication {
public:
	Application(icsneo::Device& dev, const ChipInfo& chip, bool doAuthChecks = false, bool encrypt = false)
		: Communication(dev), targetChip(chip), doAuthChecks(doAuthChecks), encrypted(encrypt) {}

	void setExpansionSlot(uint8_t slot) { targetChip.expansionSlot = slot; }

	bool enterApplication() override;
	bool hasFinalizationStep() const override {
		return true;
	}
	bool finalize() override;
	bool flash(const FWRepo& repo, const icsneo::Device::OpenStatusHandler& handler) override;

	bool open() override {
		return true;
	}

	bool close() override {
		return true;
	}

	bool authenticate();
	bool sendSoftware(const std::vector<uint8_t>& bin, const icsneo::Device::OpenStatusHandler& handler);
	bool sendSoftware(const std::vector<IEF::Sector>& sectors, uint32_t blockSize, const icsneo::Device::OpenStatusHandler& handler);
	bool validateFirmware(const DeviceAppVersion& chipSrcVer);
	bool validateComponent();

private:
	// Sub-commands of ExtendedCommand::SoftwareUpdate
	enum class Update : uint8_t {
		Write = 0,
		Erase = 1,
		GetProgress = 2,
		CheckVersion = 5,
		ValidateComponent = 6,
		Finalize = 7,
		CheckComponentVersion = 9
	};

	// Prefixes the arguments of every ExtendedCommand::SoftwareUpdate request, and is echoed back with progress
#pragma pack(push, 2)
	struct SoftwareUpdateCommandHeader {
		uint32_t componentIdentifier; // unique id for the software component - analogous to what used to be "chip id"
		Update commandType;
		uint8_t expansionSlot;
		uint32_t offset;
		uint32_t commandSizeOrProgress; // size
	};
#pragma pack(pop)

	Chip targetChip;
	const bool doAuthChecks;
	const bool encrypted;

	using StatusHandler = std::function<void(double progress)>;
	bool sendCommand(Update command, const StatusHandler& handler, uint32_t offset = 0,
		const std::vector<uint8_t>& args = {}, std::chrono::milliseconds timeout = std::chrono::milliseconds(3000)) {
		return sendCommand(command, offset, static_cast<uint32_t>(args.size()), handler, timeout, args);
	}
	bool sendCommand(Update command, uint32_t offset, uint32_t length, const StatusHandler& handler,
		std::chrono::milliseconds timeout = std::chrono::milliseconds(3000), std::vector<uint8_t> args = {});
};

} // namespace Bootloader

} // namespace icsneo

#endif // __cplusplus

#endif // __BOOTLOADER_APPLICATION_H_
