#include "icsneo/device/extensions/bootloader/api.h"
#include "icsneo/device/extensions/bootloader/communication/radcommunication.h"
#include "icsneo/device/extensions/bootloader/communication/radmultichipcommunication.h"
#include "icsneo/device/extensions/bootloader/communication/radperiphcommunication.h"
#include "icsneo/device/extensions/bootloader/communication/redcore.h"
#include "icsneo/device/extensions/bootloader/communication/red.h"
#include "icsneo/device/extensions/bootloader/communication/application.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"
#include "icsneo/communication/message/resetstatusmessage.h"
#include "icsneo/api/lifetime.h"

#include <algorithm>
#include <chrono>
#include <set>
#include <thread>
#include <unordered_set>

using namespace icsneo;
using namespace icsneo::Bootloader;

static std::unique_ptr<Bootloader::Communication> getCommunication(
	icsneo::Device& device, const FlashPhase& phase, const ChipInfo& chip
) {
	switch(phase.comm) {
		case BootloaderCommunication::Application:
			return std::make_unique<Application>(device, chip, phase.authenticate, phase.encrypt);
		case BootloaderCommunication::RED:
			return std::make_unique<REDCommunication>(device, chip, phase.authenticate, phase.encrypt);
		case BootloaderCommunication::REDCore:
			return std::make_unique<REDCoreCommunication>(device, chip);
		case BootloaderCommunication::RAD:
			return std::make_unique<RADCommunication>(device, chip);
		case BootloaderCommunication::RADMultiChip:
			return std::make_unique<RADMultiChipCommunication>(device, chip, phase.authenticate, phase.slot);
		case BootloaderCommunication::RADGalaxy2Peripheral:
			return std::make_unique<RADPeriphCommunication>(device, chip);
		case BootloaderCommunication::Invalid:
			break;
	}
	return nullptr;
}

static bool findInfo(icsneo::Device& device, ChipID chip, ChipInfo& info) {
	const auto& allInfo = device.getChipInfo();
	auto iter = std::find_if(allInfo.begin(), allInfo.end(), [chip](const auto& candidate) { return candidate.id == chip; });
	if(iter == allInfo.end()) {
		return false;
	}
	info = *iter;
	return true;
}

static const FlashPhase* findCorrespondingFlashPhase(const BootloaderPipeline& pipeline, ChipID chip) {
	for(const auto& phase : pipeline.phases) {
		if(phase->getType() == BootloaderPhase::Type::Flash) {
			const auto* flashPhase = static_cast<const FlashPhase*>(phase.get());
			if(flashPhase->chip == chip) {
				return flashPhase;
			}
		}
	}
	return nullptr;
}

static std::string chipString(ChipID chip) {
	return "chip id: " + std::to_string(static_cast<int>(chip));
}

bool Bootloader::requiresUpdate(icsneo::Device& device, const FWRepo& fw) {
	const auto versions = device.getChipVersions();
	for(const auto& info : device.getChipInfo()) {
		if(!info.defaultEnabled || !info.iefName) {
			continue;
		}
		auto iter = std::find_if(versions.begin(), versions.end(), [&info](const auto& ver) { return ver.id == info.id; });
		if(iter == versions.end()) {
			continue;
		}
		auto ver = fw.getChipVersion(info.iefName);
		if(ver && *ver != iter->makeAppVersion()) {
			return true;
		}
	}
	return false;
}

/**
 * Open the communication, run the action, then close it
 */
template<typename Action>
static bool withCommunication(Bootloader::Communication* com, const ChipInfo& info, const char* phaseName, Action action) {
	if(!com) {
		Log::Error(phaseName, "no bootloader communication for " + chipString(info.id));
		return false;
	}
	if(!com->open()) {
		Log::Error(phaseName, "failed to open communication for " + chipString(info.id));
		return false;
	}
	if(!action(*com)) {
		return false;
	}
	if(!com->close()) {
		Log::Error(phaseName, "failed to close communication for " + chipString(info.id));
		return false;
	}
	return true;
}

static bool runPipeline(
	icsneo::Device& device,
	const BootloaderPipeline& pipeline,
	const FWRepo& fw,
	const Logger& logger,
	bool alreadyInBootloader,
	bool forceFlash
) {
	icsneo::Lifetime suppressHeartbeats = device.suppressDisconnects();
	const auto versions = device.getChipVersions();
	std::unordered_set<icsneo::ChipID> chipsFlashed;

	bool updateAll = false;
	if(auto iter = pipeline.settings.find(BootloaderSetting::UpdateAll); iter != pipeline.settings.end()) {
		updateAll = std::get<bool>(iter->second);
	}

	for(const auto& phase : pipeline.phases) {
		switch(phase->getType()) {
			case BootloaderPhase::Type::EnterBootloader: {
				if(alreadyInBootloader) {
					Log::Trace("EnterBootloaderPhase", "device already in bootloader, skipping");
					break;
				}
				Log::Info("EnterBootloaderPhase", "entering bootloader");
				if(!enterBootloader(device)) {
					Log::Error("EnterBootloaderPhase", "failed to enter bootloader");
					return false;
				}
				break;
			}
			case BootloaderPhase::Type::Flash: {
				const auto& flashPhase = static_cast<const FlashPhase&>(*phase);
				Log::Info("FlashPhase", "started for " + chipString(flashPhase.chip));

				ChipInfo info;
				if(!findInfo(device, flashPhase.chip, info)) {
					Log::Error("FlashPhase", "failed to find chip info for " + chipString(flashPhase.chip));
					return false;
				}

				const auto sourceVer = info.iefName ? fw.getChipVersion(info.iefName) : std::nullopt;
				if(!sourceVer) {
					Log::Trace("FlashPhase", "skipped, no source version for " + chipString(flashPhase.chip));
					break;
				}
				if(!updateAll && !forceFlash) {
					auto iter = std::find_if(versions.begin(), versions.end(), [&info](const auto& ver) { return ver.id == info.id; });
					if(iter == versions.end()) {
						Log::Trace("FlashPhase", "skipped, chip not reported by the device, " + chipString(flashPhase.chip));
						break;
					}
					if(iter->makeAppVersion() == *sourceVer) {
						Log::Trace("FlashPhase", "skipped, chip already at target version, " + chipString(flashPhase.chip));
						break;
					}
				}

				auto com = getCommunication(device, flashPhase, info);
				if(!withCommunication(com.get(), info, "FlashPhase", [&](Bootloader::Communication& opened) {
					if(!opened.flash(fw, logger)) {
						Log::Error("FlashPhase", "failed to flash " + chipString(flashPhase.chip));
						return false;
					}
					return true;
				})) {
					return false;
				}

				chipsFlashed.insert(info.id);
				Log::Info("FlashPhase", "completed successfully for " + chipString(flashPhase.chip));
				break;
			}
			case BootloaderPhase::Type::Finalize: {
				const auto& finalizePhase = static_cast<const FinalizePhase&>(*phase);
				if(!chipsFlashed.count(finalizePhase.chip)) {
					Log::Trace("FinalizePhase", "skipped, chip not flashed, " + chipString(finalizePhase.chip));
					break;
				}
				Log::Info("FinalizePhase", "started for " + chipString(finalizePhase.chip));
				const FlashPhase* flashPhase = findCorrespondingFlashPhase(pipeline, finalizePhase.chip);
				ChipInfo info;
				if(!flashPhase || !findInfo(device, finalizePhase.chip, info)) {
					// Should never happen (developer error)
					Log::Error("FinalizePhase", "no flash phase or chip info for " + chipString(finalizePhase.chip));
					return false;
				}

				auto com = getCommunication(device, *flashPhase, info);
				if(com && !com->hasFinalizationStep()) {
					Log::Trace("FinalizePhase", "skipped, no finalization step for " + chipString(finalizePhase.chip));
					break;
				}

				if(!withCommunication(com.get(), info, "FinalizePhase", [&](Bootloader::Communication& opened) {
					if(!opened.finalize()) {
						Log::Error("FinalizePhase", "failed to finalize " + chipString(finalizePhase.chip));
						return false;
					}
					return true;
				})) {
					return false;
				}
				Log::Info("FinalizePhase", "completed successfully for " + chipString(finalizePhase.chip));
				break;
			}
			case BootloaderPhase::Type::Reconnect: {
				Log::Info("ReconnectPhase", "reconnecting to device");
				if(!device.reconnect(std::chrono::milliseconds(60000))) {
					Log::Error("ReconnectPhase", "failed to reconnect to device");
					return false;
				}
				break;
			}
			case BootloaderPhase::Type::Wait: {
				const auto& waitPhase = static_cast<const WaitPhase&>(*phase);
				Log::Trace("WaitPhase", "waiting " + std::to_string(waitPhase.timeout.count()) + "ms");
				std::this_thread::sleep_for(waitPhase.timeout);
				break;
			}
			case BootloaderPhase::Type::EnterApplication: {
				const auto& enterAppPhase = static_cast<const EnterApplicationPhase&>(*phase);
				if(!chipsFlashed.count(enterAppPhase.chip)) {
					Log::Trace("EnterApplicationPhase", "skipped, chip not flashed, " + chipString(enterAppPhase.chip));
					break;
				}
				Log::Info("EnterApplicationPhase", "started for " + chipString(enterAppPhase.chip));
				const FlashPhase* flashPhase = findCorrespondingFlashPhase(pipeline, enterAppPhase.chip);
				ChipInfo info;
				if(!flashPhase || !findInfo(device, enterAppPhase.chip, info)) {
					// Should never happen (developer error)
					Log::Error("EnterApplicationPhase", "no flash phase or chip info for " + chipString(enterAppPhase.chip));
					return false;
				}

				auto com = getCommunication(device, *flashPhase, info);
				if(!withCommunication(com.get(), info, "EnterApplicationPhase", [&](Bootloader::Communication& opened) {
					if(!opened.enterApplication()) {
						Log::Error("EnterApplicationPhase", "failed to enter application for " + chipString(enterAppPhase.chip));
						return false;
					}
					return true;
				})) {
					return false;
				}
				Log::Info("EnterApplicationPhase", "completed successfully for " + chipString(enterAppPhase.chip));
				break;
			}
		}
	}
	return exitBootloader(device, logger, !chipsFlashed.empty());
}

bool Bootloader::update(
	icsneo::Device& device,
	const FWRepo& fw,
	Logger logger,
	bool alreadyInBootloader,
	bool forceFlash
) {
	const auto pipeline = device.getBootloader();
	if(!pipeline) {
		device.getEventHandler()(APIEvent::Type::FirmwareUpdateNotSupported, APIEvent::Severity::Error);
		return false;
	}

	if(!runPipeline(device, pipeline, fw, logger, alreadyInBootloader, forceFlash)) {
		device.getEventHandler()(APIEvent::Type::FirmwareUpdateFailed, APIEvent::Severity::Error);
		return false;
	}
	return true;
}

bool Bootloader::enterBootloader(icsneo::Device& device) {
	if(!device.com->sendCommand(icsneo::Command::EnterBootloader)) {
		return false;
	}
	return device.reconnect(std::chrono::milliseconds(30000));
}

bool Bootloader::exitBootloader(icsneo::Device& device, Logger logger, bool updatedChips) {
	if(!updatedChips) {
		return true;
	}
	logger(icsneo::Device::OpenStatusType::Progress, "Connecting to the device...", std::nullopt);
	return verifyConnection(device, logger);
}

/**
 * Toggle network communication and wait up to 5 seconds for a reset status confirming the change.
 * Like icsneo::Device::goOnline(), a timeout is not an error.
 */
static bool setCommunicationEnabled(icsneo::Device& device, const Logger& logger, bool enable, double progressStart) {
	static constexpr std::chrono::seconds Timeout(5);
	const auto filter = std::make_shared<MessageFilter>(Message::Type::ResetStatus);

	if(!device.com->sendCommand(Command::EnableNetworkCommunication, enable)) {
		return false;
	}
	device.com->sendCommand(Command::UpdateLEDState, std::vector<uint8_t>({uint8_t(enable ? 0x10 : 0x04)}));

	const auto startTime = std::chrono::steady_clock::now();
	auto elapsedTime = std::chrono::duration<double>(0);
	while(elapsedTime < Timeout) {
		bool failOut = false;
		auto msg = device.com->waitForMessageSync([&device, &failOut]() {
			if(!device.com->sendCommand(Command::RequestStatusUpdate)) {
				failOut = true;
				return false;
			}
			return true;
		}, filter, std::chrono::milliseconds(100));

		if(failOut) {
			return false;
		}

		if(msg && std::static_pointer_cast<icsneo::ResetStatusMessage>(msg)->comEnabled == enable) {
			break;
		}

		// Each call covers half of the overall progress
		logger(icsneo::Device::OpenStatusType::Progress, "Connecting to the device...", progressStart + (elapsedTime / Timeout) / 2.0);
		elapsedTime = std::chrono::steady_clock::now() - startTime;
	}
	logger(icsneo::Device::OpenStatusType::Progress, "Connecting to the device...", progressStart + 0.5);
	return true;
}

bool Bootloader::verifyConnection(icsneo::Device& device, Logger logger) {
	// Firmware needs to confirm the update by verifying that it can enable/disable the connection on the device
	return setCommunicationEnabled(device, logger, true, 0.0) && setCommunicationEnabled(device, logger, false, 0.5);
}

bool Bootloader::isSFPChip(const icsneo::Bootloader::Chip& chip) {
	static const std::set<icsneo::ChipID> sfpChipIDs = {
		icsneo::ChipID::SFPModule_88q2112_MCHIP,
		icsneo::ChipID::SFPModule_88q2221_MCHIP,
		icsneo::ChipID::SFPModule_88q3244_MCHIP,
		icsneo::ChipID::SFPModule_ent11100_MCHIP,
		icsneo::ChipID::SFPModule_lan8670_MCHIP,
	};
	return sfpChipIDs.find(chip.getID()) != sfpChipIDs.end();
}

bool Bootloader::isSFPIEF(const icsneo::Bootloader::IEF& ief) {
	static const std::set<icsneo::Device::ProductID> sfpProductIDs = {
		icsneo::Device::ProductID::SFPModule_88q2112,
		icsneo::Device::ProductID::SFPModule_88q2221m,
		icsneo::Device::ProductID::SFPModule_88q4364,
		icsneo::Device::ProductID::SFPModule_ent11100,
		icsneo::Device::ProductID::SFPModule_lan8670,
	};
	return sfpProductIDs.find(static_cast<icsneo::Device::ProductID>(ief.getVersion().prodId)) != sfpProductIDs.end();
}
