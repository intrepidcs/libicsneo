#include "icsneo/device/extensions/bootloader/communication/radmultichipcommunication.h"
#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"
#include "icsneo/device/extensions/bootloader/parser/ief.h"
#include "icsneo/device/extensions/bootloader/api.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <algorithm>
#include <thread>

using namespace icsneo::Bootloader;

RADMultiChipCommunication::RADMultiChipCommunication(icsneo::Device& dev, const ChipInfo& chipToFlash, bool auth, uint8_t slot)
	: RADCommunication(dev, chipToFlash), doAuthChecks(auth), slot(slot) {}

bool RADMultiChipCommunication::authenticate() {
	if(!doAuthChecks) {
		Log::Trace(__func__, "auth checks disabled, skipping");
		return true;
	}
	Log::Debug(__func__, "starting authentication");
	if(!standardComm->authenticate()) {
		Log::Warning(__func__, "standardComm auth failed, retrying with sanity check");
		device.com->pauseReads();
		if(!sanityCheck()) {
			if(!sanityCheck()) {
				device.com->resumeReads();
				Log::Error(__func__, "sanity check failed after retries");
				return false;
			}
		}

		if(!RADCommunication::authenticate()) {
			device.com->resumeReads();
			Log::Error(__func__, "RADCommunication auth failed");
			return false;
		}

		device.com->resumeReads();
	}
	Log::Debug(__func__, "authentication successful");
	return true;
}

bool RADMultiChipCommunication::flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) {
	Log::Info(__func__, "starting flash for " + targetChip.getName());
	auto chipSrcVer = fw.getChipVersion(targetChip.getIEFName());
	if(!chipSrcVer) {
		Log::Error(__func__, "no chip version found for " + targetChip.getIEFName());
		return false;
	}

	if(!authenticate()) {
		Log::Error(__func__, "authentication failed for " + targetChip.getName());
		return false;
	}

	if(!standardComm->validateFirmware(*chipSrcVer)) {
		Log::Error(__func__, "firmware validation failed for " + targetChip.getName());
		handler(icsneo::Device::OpenStatusType::Failed, "Failed to validate firmware", std::nullopt);
		return false;
	}

	if(targetChip.getFirmwareType() == FirmwareType::Zip) {
		std::vector<uint8_t> chipSoftware;
		if(!loadZips(fw, chipSoftware)) {
			Log::Error(__func__, "loadZips failed for " + targetChip.getName());
			return false;
		}

		if(!standardComm->sendSoftware(chipSoftware, handler)) {
			return false;
		}
	} else {
		std::vector<uint8_t> bin;
		if(!fw.getFW(targetChip.getIEFName(), bin)) {
			return false;
		}

		IEF ief;
		if(!ief.load(std::move(bin))) {
			return false;
		}

		// The supplied IEF is not for this product
		if(ief.getVersion().prodId != static_cast<uint8_t>(device.getProductID()) && !isSFPIEF(ief)) {
			Log::Error(__func__, "IEF product ID mismatch for " + targetChip.getName());
			return false;
		}

		// The supplied IEF is for this product, but not for this chip
		if(ief.getVersion().chipId != static_cast<uint8_t>(targetChip.getID())) {
			Log::Error(__func__, "IEF chip ID mismatch for " + targetChip.getName());
			return false;
		}

		if(!standardComm->sendSoftware(ief.getSectors(), ief.getBlockSize(), handler)) {
			return false;
		}
	}
	Log::Debug(__func__, "calling finalize");
	if(!standardComm->finalize()) {
		Log::Error(__func__, "finalize failed");
		return false;
	}
	Log::Debug(__func__, "finalize successful, proceeding to reboot");
	handler(icsneo::Device::OpenStatusType::Progress, "Rebooting device...", std::nullopt);
	return true;
}

bool RADMultiChipCommunication::enterApplication() {
	Log::Info(__func__, "starting");
	if(!standardComm->enterApplication()) {
		Log::Error(__func__, "reboot failed");
		return false;
	}
	Log::Debug(__func__, "reboot sent, waiting 5000ms");
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
	return true;
}

bool RADMultiChipCommunication::open() {
	const auto& components = device.getComponentVersions();
	auto component = std::find_if(components.begin(), components.end(), [this](const auto& candidate) {
		if(slot != UINT8_MAX && slot != candidate.expansionSlot) {
			return false;
		}
		return targetChip.getID() == (ChipID)candidate.identifier;
	});
	if(component == components.end()) {
		Log::Error(__func__, "no component found for " + targetChip.getName());
		return false;
	}
	standardComm = std::make_unique<Application>(device, targetChip.info);
	standardComm->setExpansionSlot(component->expansionSlot);
	// Pause the read thread so it doesn't consume any responses
	device.com->pauseReads();
	const bool res = initialize();
	device.com->resumeReads();
	return res;
}

bool RADMultiChipCommunication::close() {
	return true;
}
