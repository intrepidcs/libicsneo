#include "icsneo/device/extensions/bootloader/communication/application.h"
#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"
#include "icsneo/device/extensions/bootloader/api.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include "icsneo/communication/message/main51message.h"

#include <algorithm>
#include <condition_variable>
#include <cstring>
#include <limits>
#include <mutex>
#include <optional>
#include <thread>

#include <icsbl/icsbl.h>

namespace icsneo {

namespace Bootloader {

bool Application::enterApplication() {
	// Always reboot
	Log::Info(__func__, "sending reboot command");
	bool ok = device.com->sendCommand(icsneo::ExtendedCommand::Reboot);
	if(!ok) {
		Log::Error(__func__, "reboot command failed");
	}
	return ok;
}

bool Application::finalize() {
	Log::Debug(__func__, "sending finalize command");
	bool ok = sendCommand(Update::Finalize, [](double) {});
	if(!ok) {
		Log::Error(__func__, "finalize command failed");
	}
	return ok;
}

bool Application::flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) {
	Log::Info(__func__, "starting flash for " + targetChip.getName());
	const auto& srcVer = fw.getVersion();

	std::vector<uint8_t> bin;
	if(!fw.getFW(targetChip.getIEFName(), bin)) {
		Log::Error(__func__, "failed to get firmware for " + targetChip.getIEFName());
		return false;
	}

	IEF ief;
	if(!ief.load(std::move(bin))) {
		Log::Error(__func__, "failed to load IEF for " + targetChip.getName());
		return false;
	}

	auto currentString = "Preparing to update " + targetChip.getName() + "...";
	handler(icsneo::Device::OpenStatusType::Progress, currentString, std::nullopt);

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

	if(doAuthChecks) {
		if(!authenticate()) {
			Log::Error(__func__, "authentication failed for " + targetChip.getName());
			return false;
		}

		// Tell the application software what version of Vehicle Spy 3 this firmware is from.
		// We check to make sure it approves, but also, the device will not accept new firmware
		// if we have not sent this.
		// Sometimes new hardware revisions can not accept firmware which is too old.
		if(!sendCommand(Update::CheckVersion, [](double) {}, 0, {srcVer.major, srcVer.minor, srcVer.release, srcVer.build})) {
			Log::Error(__func__, "version check failed for " + targetChip.getName());
			return false;
		}
	}

	bool ok = sendSoftware(ief.getSectors(), ief.getBlockSize(), handler);
	if(!ok) {
		Log::Error(__func__, "sendSoftware failed for " + targetChip.getName());
	}
	return ok;
}

bool Application::authenticate() {
	icsbl_seed_v1 seed = {};
	if(!icsbl_generate_seed_v1(device.getSerialNumber(), &seed)) {
		Log::Error(__func__, "libicsbl could not generate an authentication challenge");
		return false;
	}

	// Send the challenge (seed.a), the device replies with seed.b and one trailing byte
	std::vector<uint8_t> cmdArgs(sizeof(seed.a));
	writeLE32(cmdArgs.data(), seed.a[0]);
	writeLE32(cmdArgs.data() + 4, seed.a[1]);

	auto filter = std::make_shared<icsneo::MessageFilter>(icsneo::Message::Type::Main51);
	auto msg = device.com->waitForMessageSync([&] {
		return device.com->sendCommand(icsneo::Command::ValidateDevice, cmdArgs);
	}, filter, std::chrono::milliseconds(200));

	if(!msg) {
		Log::Error(__func__, "No response from the device");
		return false;
	}

	auto main51Msg = std::static_pointer_cast<icsneo::Main51Message>(msg);

	if(main51Msg->data.size() != sizeof(seed.b) + 1) {
		Log::Error(__func__, "Unexpected authentication reply size from the device");
		return false;
	}

	const bool authenticated = readLE32(main51Msg->data.data()) == seed.b[0] && readLE32(main51Msg->data.data() + 4) == seed.b[1];
	if(!authenticated) {
		Log::Error(__func__, "The authentication reply does not match, the device did not authenticate");
	}
	return authenticated;
}

bool Application::validateFirmware(const DeviceAppVersion& chipSrcVer) {
	return sendCommand(Update::CheckComponentVersion, [](double) {}, 0, {chipSrcVer.major, chipSrcVer.minor});
}

bool Application::sendSoftware(const std::vector<uint8_t>& bin, const icsneo::Device::OpenStatusHandler& handler) {
	Log::Debug(__func__, "starting for " + targetChip.getName() + " (" + std::to_string(bin.size()) + " bytes)");
	auto currentString = "Preparing to update " + targetChip.getName() + "...";
	handler(icsneo::Device::OpenStatusType::Progress, currentString, std::nullopt);
	if(bin.empty() || bin.size() > std::numeric_limits<uint32_t>::max()) {
		Log::Error(__func__, "invalid firmware size for " + targetChip.getName());
		return false;
	}
	const uint32_t totalSize = static_cast<uint32_t>(bin.size());

	Log::Debug(__func__, "erasing " + targetChip.getName());
	if(!sendCommand(Update::Erase, 0, totalSize, [&](double progress) {
		handler(icsneo::Device::OpenStatusType::Progress, currentString, progress);
	}, std::chrono::milliseconds(45000))) {
		Log::Error(__func__, "erase failed for " + targetChip.getName());
		return false;
	}

	currentString = "Updating " + targetChip.getName() + "...";
	handler(icsneo::Device::OpenStatusType::Progress, currentString, std::nullopt);

	const uint32_t stepSize = std::min<uint32_t>(totalSize, 2048);
	uint32_t bytesTransferred = 0;
	while(bytesTransferred < totalSize) {
		const uint32_t bytesToSend = std::min(stepSize, totalSize - bytesTransferred);
		if(!sendCommand(Update::Write, [](double) {}, bytesTransferred, { bin.begin() + bytesTransferred, bin.begin() + bytesTransferred + bytesToSend})) {
			Log::Error(__func__, "write failed at offset " + std::to_string(bytesTransferred) + " for " + targetChip.getName());
			return false;
		}

		bytesTransferred += bytesToSend;
		handler(icsneo::Device::OpenStatusType::Progress, currentString, double(bytesTransferred) / totalSize);
	}
	Log::Debug(__func__, "write complete, validating " + targetChip.getName());
	if(!validateComponent()) {
		Log::Error(__func__, "validation failed for " + targetChip.getName());
		return false;
	}
	Log::Debug(__func__, "completed successfully for " + targetChip.getName());
	return true;
}

bool Application::sendSoftware(const std::vector<IEF::Sector>& sectorPairs, uint32_t blockSize, const icsneo::Device::OpenStatusHandler& handler) {
	if(sectorPairs.empty()) {
		Log::Error(__func__, "no sectors to flash for " + targetChip.getName());
		return false;
	}

	Log::Debug(__func__, "starting for " + targetChip.getName() + " (" + std::to_string(sectorPairs.size()) + " sectors)");
	auto currentString = "Preparing to update " + targetChip.getName() + "...";
	handler(icsneo::Device::OpenStatusType::Progress, currentString, std::nullopt);
	uint64_t totalSize = 0;
	for(const auto& sector : sectorPairs) {
		totalSize += sector.second.size();
	}
	if(totalSize > std::numeric_limits<uint32_t>::max()) {
		Log::Error(__func__, "total size too large for " + targetChip.getName());
		return false;
	}

	if(totalSize == 0) {
		Log::Error(__func__, "total size is 0 for " + targetChip.getName());
		return false;
	}

	Log::Debug(__func__, "erasing " + targetChip.getName() + " (" + std::to_string(totalSize) + " bytes)");
	if(!sendCommand(Update::Erase, 0, static_cast<uint32_t>(totalSize), [&](double progress) {
		handler(icsneo::Device::OpenStatusType::Progress, currentString, progress);
	}, std::chrono::milliseconds(45000))) {
		Log::Error(__func__, "erase failed for " + targetChip.getName());
		return false;
	}

	if(isSFPChip(targetChip)) {
		// SFPs are not ready to write until 4 seconds after finishing erasing (for some reason)
		Log::Trace(__func__, "SFP chip detected, waiting 4000ms after erase");
		std::this_thread::sleep_for(std::chrono::milliseconds(4000));
	}

	currentString = "Updating " + targetChip.getName() + "...";
	handler(icsneo::Device::OpenStatusType::Progress, currentString, std::nullopt);

	uint32_t bytesTransferred = 0;
	for(const auto& pair : sectorPairs) {
		const auto& rawData = pair.second;
		const uint32_t offset = pair.first * blockSize;
		std::vector<uint8_t> processedData(rawData.begin(), rawData.end());
		if(encrypted) {
			if(rawData.size() % 8 != 0) {
				Log::Error(__func__, "raw data size not aligned");
				return false;
			}
			icsbl_encrypt_v1(device.getSerialNumber(), rawData.data(), rawData.size(), processedData.data());
		}

		if(!sendCommand(Update::Write, [](double) {}, offset, processedData)) {
			Log::Error(__func__, "write failed at offset " + std::to_string(offset) + " for " + targetChip.getName());
			return false;
		}

		bytesTransferred += static_cast<uint32_t>(rawData.size());
		handler(icsneo::Device::OpenStatusType::Progress, currentString, double(bytesTransferred) / double(totalSize));
	}
	Log::Debug(__func__, "write complete, validating " + targetChip.getName());
	bool ok = validateComponent();
	if(!ok) {
		Log::Error(__func__, "validation failed for " + targetChip.getName());
	}
	else {
		Log::Debug(__func__, "completed successfully for " + targetChip.getName());
	}
	return ok;
}

bool Application::validateComponent() {
	Log::Debug(__func__, "validating " + targetChip.getName());
	bool ok = sendCommand(Update::ValidateComponent, [](double) {}, 0, {}, std::chrono::milliseconds(45000));
	if(!ok) {
		Log::Error(__func__, "validation failed for " + targetChip.getName());
	}
	return ok;
}

/**
 * Send one SoftwareUpdate sub-command and wait for it to complete.
 *
 * The device answers the request with OK when it is already done, or OperationPending when it will
 * finish later. For a pending command we poll Update::GetProgress (except for Write and
 * ValidateComponent, which do not report progress) until the device sends a final OK or the timeout
 * passes. Progress replies are OperationPending with a SoftwareUpdateCommandHeader whose
 * commandSizeOrProgress holds the progress so far, or a GenericReturn with no progress.
 */
bool Application::sendCommand(Update command, uint32_t offset, uint32_t length, const StatusHandler& handler, std::chrono::milliseconds timeout, std::vector<uint8_t> args) {
	bool finishedResponse = false;
	std::condition_variable finishedResponseCV;
	std::mutex finishedResponseMutex;

	std::optional<uint32_t> progress;
	std::condition_variable progressCV;
	std::mutex progressMutex;

	bool firstResponse = true;
	// Register the callback before sending the command, so that if it goes from pending to finished quickly we will still catch it
	const auto callback = std::make_shared<icsneo::MessageCallback>(std::make_shared<icsneo::MessageFilter>(icsneo::Message::Type::ExtendedResponse), [&](std::shared_ptr<icsneo::Message> msg) {
		if(firstResponse) {
			firstResponse = false;
			return true; // let waitForMessageSync handle the first message
		}
		auto extMsg = std::static_pointer_cast<ExtendedResponseMessage>(msg);
		if(extMsg->command != ExtendedCommand::SoftwareUpdate) {
			Log::Error(__func__, "Got an extended response for a command other than SoftwareUpdate");
			return false;
		}
		if(extMsg->response == ExtendedResponse::OK) {
			{
				std::scoped_lock lk(finishedResponseMutex);
				finishedResponse = true;
			}
			finishedResponseCV.notify_one();
		} else if(extMsg->response == ExtendedResponse::OperationPending) {
			if(extMsg->data.size() < sizeof(ExtendedResponseMessage::ResponseHeader)) {
				Log::Error(__func__, "The pending response is too short to contain a header");
				return false;
			}
			ExtendedResponseMessage::ResponseHeader respHeader;
			std::memcpy(&respHeader, extMsg->data.data(), sizeof(respHeader));
			if(respHeader.command == ExtendedCommand::SoftwareUpdate) {
				if(respHeader.length != sizeof(SoftwareUpdateCommandHeader) ||
					extMsg->data.size() < sizeof(ExtendedResponseMessage::ResponseHeader) + sizeof(SoftwareUpdateCommandHeader)) {
					Log::Error(__func__, "The progress response does not contain a SoftwareUpdate header");
					return false;
				}
				SoftwareUpdateCommandHeader resp;
				std::memcpy(&resp, extMsg->data.data() + sizeof(ExtendedResponseMessage::ResponseHeader), sizeof(resp));
				{
					std::scoped_lock lk(progressMutex);
					progress.emplace(resp.commandSizeOrProgress);
				}
				progressCV.notify_one();
			} else if(respHeader.command == ExtendedCommand::GenericReturn) {
				// No progress information, the final OK comes later
				{
					std::scoped_lock lk(progressMutex);
					progress.emplace(0);
				}
				progressCV.notify_one();
			} else {
				Log::Error(__func__, "The pending response is for an unexpected command");
				return false;
			}
		} else {
			Log::Error(__func__, "The device reported an error for the SoftwareUpdate command");
			return false;
		}
		return true;
	});
	const auto finishedCallbackHandle = device.com->addMessageCallback(callback);
	Lifetime statusCallbackCleanup([&] {
		device.com->removeMessageCallback(finishedCallbackHandle);
	});

	SoftwareUpdateCommandHeader header = {};
	header.componentIdentifier = static_cast<uint32_t>(targetChip.getID());
	header.commandType = command;
	header.offset = offset;
	header.commandSizeOrProgress = length;
	header.expansionSlot = targetChip.expansionSlot;
	const auto headerBytes = reinterpret_cast<const uint8_t*>(&header);
	args.insert(args.begin(), headerBytes, headerBytes + sizeof(header));

	{
		auto msg = device.com->waitForMessageSync([&] {
			return device.com->sendCommand(ExtendedCommand::SoftwareUpdate, args);
		}, std::make_shared<icsneo::MessageFilter>(icsneo::Message::Type::ExtendedResponse), timeout);

		if(!msg) {
			Log::Error(__func__, "No response to the SoftwareUpdate command");
			return false;
		}

		auto extMsg = std::static_pointer_cast<ExtendedResponseMessage>(msg);
		if(extMsg->command != ExtendedCommand::SoftwareUpdate) {
			Log::Error(__func__, "Got an extended response for a command other than SoftwareUpdate");
			return false;
		}
		if(extMsg->response == ExtendedResponse::OK) {
			return true;
		}
		if(extMsg->response != ExtendedResponse::OperationPending) {
			Log::Error(__func__, "The device rejected the SoftwareUpdate command");
			return false;
		}
	}

	SoftwareUpdateCommandHeader progressHeader = {};
	progressHeader.componentIdentifier = static_cast<uint32_t>(targetChip.getID());
	progressHeader.commandType = Update::GetProgress;
	progressHeader.expansionSlot = targetChip.expansionSlot;
	const auto progressHeaderBytes = reinterpret_cast<const uint8_t*>(&progressHeader);
	const std::vector<uint8_t> progressArgs(progressHeaderBytes, progressHeaderBytes + sizeof(progressHeader));
	const auto deadline = std::chrono::steady_clock::now() + timeout;
	// Write and ValidateComponent do not report progress, just wait for the final OK
	if(command != Update::Write && command != Update::ValidateComponent) {
		while(std::chrono::steady_clock::now() < deadline) {
			{
				std::unique_lock lk(progressMutex);
				progress.reset();
				if(!device.com->sendCommand(ExtendedCommand::SoftwareUpdate, progressArgs)) {
					Log::Error(__func__, "Failed to send progress request to the device");
					return false;
				}
				if(!progressCV.wait_for(lk, std::chrono::milliseconds(5000), [&]{ return progress.has_value(); })) {
					Log::Error(__func__, "No progress response from device");
					return false;
				}
				handler(*progress / double(length));
			}
			{
				std::scoped_lock lk(finishedResponseMutex);
				if(finishedResponse) {
					break;
				}
			}
		}
	}

	std::unique_lock lk(finishedResponseMutex);
	return finishedResponseCV.wait_until(lk, deadline, [&]{ return finishedResponse; });
}

} // namespace Bootloader

} // namespace icsneo
