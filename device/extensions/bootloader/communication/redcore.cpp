#include "icsneo/device/extensions/bootloader/communication/redcore.h"
#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <algorithm>
#include <limits>

namespace icsneo {

namespace Bootloader {

bool REDCoreCommunication::flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) {
	constexpr const size_t MaxSectorSize = 0x600;

	std::vector<uint8_t> bin;
	if(!fw.getFW(targetChip.getIEFName(), bin)) {
		return false;
	}

	if(bin.size() > std::numeric_limits<uint32_t>::max()) {
		Log::Error(__func__, "Firmware too large for " + targetChip.getName());
		return false;
	}

	const std::string message = "Updating " + targetChip.getName() + "...";

	// First send the firmware
	const uint32_t fileSize = uint32_t(bin.size());
	size_t sectorsFlashed = 0;
	const size_t sectorCount = (fileSize / MaxSectorSize) + (fileSize % MaxSectorSize ? 1 : 0);
	std::vector<uint8_t> sector;
	for(size_t i = 0; i < fileSize; i += MaxSectorSize) {
		Log::Trace(__func__, "Sending sector " + std::to_string(sectorsFlashed));
		handler(icsneo::Device::OpenStatusType::Progress, message, double(sectorsFlashed++) / sectorCount);

		sector.assign(bin.begin() + i, bin.begin() + i + std::min<size_t>(MaxSectorSize, fileSize - i));
		if(!sendCommand(uint8_t(Command::SendFirmware), sector)) {
			Log::Error(__func__, "Failed to send sector " + std::to_string(sectorsFlashed - 1) + " for " + targetChip.getName());
			return false;
		}
	}

	// Then flash it
	Log::Debug(__func__, "Starting core flash for " + targetChip.getName());
	handler(icsneo::Device::OpenStatusType::Progress, message, std::nullopt);

	// There is a long timeout on the StartFirmwareFlash because the bootloader may block here,
	// potentially for the entire duration of the flash
	if(!sendCommand(uint8_t(Command::StartFirmwareFlash), {
			uint8_t(fileSize & 0xFF), uint8_t((fileSize >> 8) & 0xFF),
			uint8_t((fileSize >> 16) & 0xFF), uint8_t((fileSize >> 24) & 0xFF),
		}, std::chrono::seconds(60))) {
		Log::Error(__func__, "Failed to start core flash for " + targetChip.getName());
		return false;
	}

	// Watch the responses to know when it's done
	const auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(60);
	uint32_t flashed = 0;
	while(flashed < fileSize && std::chrono::steady_clock::now() < endTime) {
		if(!sendCommandRaw(uint8_t(Command::GetFlashProgress))) {
			Log::Error(__func__, "Failed to request flash progress for " + targetChip.getName());
			return false;
		}

		if(!waitForRx(std::chrono::milliseconds(3000))) {
			Log::Trace(__func__, "No flash progress packet within 3 seconds");
			continue;
		}

		const uint32_t oldFlashed = flashed;
		for(const auto& packet : lastData) {
			if(packet.resp != REDPacketizer::Response::Accepted)
				return false;
			if(packet.data.size() != 4) {
				Log::Trace(__func__, "Got a flash progress packet of length " + std::to_string(packet.data.size()));
				continue;
			}
			const uint32_t newFlashed = readLE32(packet.data.data());
			if(newFlashed >= flashed)
				flashed = newFlashed;
			else
				Log::Trace(__func__, "Got a flash progress packet which went backwards " + std::to_string(newFlashed) + " " + std::to_string(flashed));

			// After sending us the packet that confirms the flash, the flash position is cleared and will send us zeros.
			// We don't need any of these other packets, in that case.
			if(flashed >= fileSize)
				break;
		}
		lastData.clear();

		Log::Trace(__func__, "Flashed " + std::to_string(flashed) + " of " + std::to_string(fileSize));

		if(flashed != oldFlashed)
			handler(icsneo::Device::OpenStatusType::Progress, message, std::min(double(flashed) / fileSize, 1.0));
	}

	return flashed == fileSize;
}

bool REDCoreCommunication::sendCommandRaw(uint8_t cmd, std::vector<uint8_t> args) {
	if(args.size() > std::numeric_limits<uint16_t>::max())
		return false;
	const uint16_t size = static_cast<uint16_t>(args.size());
	args.insert(args.begin(), { CoreFlasherMagic, uint8_t(size & 0xFF), uint8_t((size >> 8) & 0xFF), cmd });
	return REDCommunicationBase::sendCommandRaw(CoreFlasherCommand, std::move(args));
}

} // namespace Bootloader

} // namespace icsneo
