#include "icsneo/device/extensions/bootloader/communication/radcommunication.h"
#include "icsneo/device/extensions/bootloader/communication/bootloaderversion.h"
#include "icsneo/device/extensions/bootloader/parser/fwzip.h"
#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <algorithm>
#include <cstring>
#include <thread>

#include <icsbl/icsbl.h>

using namespace icsneo::Bootloader;

RADCommunication::RADCommunication(icsneo::Device& dev, const ChipInfo& info)
	: Communication(dev), targetChip(info) {}

bool RADCommunication::supportsFlashBuffer() const {
	switch(targetChip.getID()) {
		case icsneo::ChipID::RADGigastar_USBZ_ZYNQ:
		case icsneo::ChipID::RADGigastar_USBZ_Z7007S_ZYNQ:
		case icsneo::ChipID::RADGigastar_USBZ_Z7010_ZYNQ: {
			return true;
		}
		default:
			return false;
	}
}

RADCommunication::Request RADCommunication::makeRequest(Request::Type type, size_t payloadSize, size_t expectedResponseLen) {
	const size_t headerSize = supportsZynqBinaryFlasher ? ZynqFlasherHeaderSize : 0;
	const size_t packetSize = headerSize + 1 + payloadSize; // Header + command + payload

	// This shouldn't cause any re-allocations since we already reserved the MaxMessageLength in initialize()
	txBuffer.resize(packetSize, 0);

	if(supportsZynqBinaryFlasher) {
		std::copy(std::begin(ZynqFlasherSync), std::end(ZynqFlasherSync), txBuffer.begin());

		// The length covers the command byte and the payload
		const uint16_t payloadPlusCmd = static_cast<uint16_t>(payloadSize) + 1;
		txBuffer[4] = payloadPlusCmd & 0xFFu;
		txBuffer[5] = (payloadPlusCmd >> 8) & 0xFFu;
	}

	txBuffer[headerSize] = static_cast<uint8_t>(type);

	Request newRequest;
	newRequest.type = type;
	newRequest.expectedResponseLen = expectedResponseLen;
	if(payloadSize > 0) {
		newRequest.payload = &txBuffer[headerSize + 1]; // First byte right after the command
	}

	return newRequest;
}

bool RADCommunication::sendRequestNoResponse() {
	/**
	 * Send our current packet, the payload was written to it if the user
	 * of this function wrote to request.payload. The remaining details were
	 * written when makeRequest() was called
	*/
	return device.com->sendPacket(txBuffer);
}

RADCommunication::Response& RADCommunication::sendRequest(const Request& request, std::chrono::milliseconds timeout) {
	static constexpr size_t ResponseHeaderSize = 2;
	const size_t rxSize = ResponseHeaderSize + request.expectedResponseLen;
	/**
	 * Clear previous response
	*/
	lastResponse = Response();

	if(!sendRequestNoResponse()) {
		Log::Error(__func__, "Failed to send request " + std::to_string(static_cast<uint8_t>(request.type)));
		return lastResponse;
	}

	auto& rb = device.com->driver->getReadBuffer();
	rb.clear();
	auto result = device.com->driver->waitForRx(rxSize, timeout);

	if(!result) {
		Log::Error(__func__, "No response to request " + std::to_string(static_cast<uint8_t>(request.type)));
		return lastResponse;
	}

	rb.read(rxBuffer.data(), 0, rxSize);
	rb.clear();

	lastResponse.done = true;

	/**
	 * Second byte is success/failure byte
	*/
	lastResponse.type = static_cast<Response::Type>(rxBuffer[1]);

	/**
	 * The rest is the payload, if we requested any
	*/
	if(request.expectedResponseLen > 0) {
		lastResponse.payload = &rxBuffer[2];
		lastResponse.size = request.expectedResponseLen;
	}

	return lastResponse;
}

bool RADCommunication::enterApplication() {
	Log::Info(__func__, "sending EnterApplication request");
	makeRequest(Request::Type::EnterApplication);
	bool ok = sendRequestNoResponse();
	if(!ok) {
		Log::Error(__func__, "EnterApplication request failed");
	}
	return ok;
}

bool RADCommunication::close() {
	device.com->resumeReads();
	return true;
}

bool RADCommunication::open() {
	device.com->pauseReads();

	if(!initialize()) {
		close();
		return false;
	}

	if(!sanityCheck()) {
		// Note: RAD-Gigastar on low power USB flash fails here for some reason, try another sanity check
		if(!sanityCheck()) {
			close();
			return false;
		}
	}

	if(!getBootloaderVersion()) {
		// Try again, just like the sanity checks
		if(!getBootloaderVersion()) {
			close();
			return false;
		}
	}

	if(!sanityCheck()) {
		// Note: RAD-Gigastar on low power USB flash fails here for some reason, try another sanity check
		if(!sanityCheck()) {
			close();
			return false;
		}
	}

	if(!authenticate()) {
		close();
		return false;
	}

	return true;
}

bool RADCommunication::initialize() {
	auto& rb = device.com->driver->getReadBuffer();

	rxBuffer = std::vector<uint8_t>(MaxMessageLength, 0);
	txBuffer.reserve(MaxMessageLength);

	// An EnterBootloader command on Main51, as a framed short packet (0xAA, then length << 4 | netid)
	// with a trailing 0xAF, followed by the same command without the 0xAA framing
	const uint8_t lengthAndNetID = (1 << 4) | static_cast<uint8_t>(icsneo::Network::NetID::Main51);
	std::vector<uint8_t> pkt = {
		0xAA,
		lengthAndNetID,
		static_cast<uint8_t>(icsneo::Command::EnterBootloader),
		0xAF
	};

	rb.clear();
	if(!device.com->sendPacket(pkt)) {
		Log::Error(__func__, "Failed to send the framed enter bootloader command");
		return false;
	}

	pkt = {
		lengthAndNetID,
		static_cast<uint8_t>(icsneo::Command::EnterBootloader)
	};

	if(!device.com->sendPacket(pkt)) {
		Log::Error(__func__, "Failed to send the unframed enter bootloader command");
		return false;
	}

	// The Zynq binary flasher answers with the command, Success, then ZynqFlasherSync
	static constexpr size_t ZynqFlasherReplySize = 2 + sizeof(ZynqFlasherSync);
	if(!device.com->driver->waitForRx(ZynqFlasherReplySize, std::chrono::milliseconds(1000))) {
		Log::Error(__func__, "No response to the enter bootloader command");
		return false;
	}

	const size_t rxLen = rb.size();
	if(rxLen == ZynqFlasherReplySize) {
		rb.read(rxBuffer.data(), 0, rxLen);
	}
	rb.clear();
	supportsZynqBinaryFlasher =
		rxLen == ZynqFlasherReplySize &&
		rxBuffer[0] == static_cast<uint8_t>(icsneo::Command::EnterBootloader) &&
		rxBuffer[1] == static_cast<uint8_t>(Response::Type::Success) &&
		std::equal(std::begin(ZynqFlasherSync), std::end(ZynqFlasherSync), rxBuffer.begin() + 2);

	return true;
}

bool RADCommunication::finalize() {
	return true;
}

bool RADCommunication::hasFinalizationStep() const {
	return false;
}

bool RADCommunication::sanityCheck() {
	if(!supportsZynqBinaryFlasher) {
		return true;
	}

	// Two numbered test patterns the bootloader must accept, the 8 byte replies are not checked
	constexpr uint8_t PatternA[8] = {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0xAA, 0x55, 0x55};
	constexpr uint8_t PatternB[8] = {0x12, 0x21, 0x34, 0x43, 0x56, 0x65, 0x78, 0x87};

	auto request = makeRequest(Request::Type::SanityCheck, 1 + sizeof(PatternA), sizeof(PatternA));

	request.payload[0] = 0x01u;
	std::copy(std::begin(PatternA), std::end(PatternA), request.payload + 1);

	if(!sendRequest(request, std::chrono::milliseconds(1000))) {
		return false;
	}

	request.payload[0] = 0x02u;
	std::copy(std::begin(PatternB), std::end(PatternB), request.payload + 1);

	return sendRequest(request, std::chrono::milliseconds(1000));
}

bool RADCommunication::getBootloaderVersion() {
	auto request = makeRequest(Request::Type::BootloaderVersion, 0, sizeof(BootloaderVersionResponse));
	auto response = sendRequest(request, std::chrono::milliseconds(1000));

	if(!response) {
		return false;
	}

	if(response.size < sizeof(BootloaderVersionResponse)) {
		return false;
	}

	BootloaderVersionResponse version;
	std::memcpy(&version, response.payload, sizeof(version));
	targetChip.info.id = static_cast<ChipID>(version.zynqId);
	return true;
}

bool RADCommunication::validateChipVersion(const icsneo::DeviceAppVersion& chipSrcVer) {
	if(!supportsZynqBinaryFlasher) {
		return true;
	}

	auto request = makeRequest(Request::Type::ValidateVersion, 2, 2);

	request.payload[0] = chipSrcVer.major;
	request.payload[1] = chipSrcVer.minor;

	return sendRequest(request, std::chrono::milliseconds(1000));
}

bool RADCommunication::authenticate() {
	icsbl_seed_v1 seed = {};
	if(!icsbl_generate_seed_v1(device.getSerialNumber(), &seed)) {
		Log::Error(__func__, "libicsbl could not generate an authentication challenge");
		return false;
	}

	auto request = makeRequest(Request::Type::Authenticate, sizeof(seed.a), sizeof(seed.b));

	writeLE32(&request.payload[0], seed.a[0]);
	writeLE32(&request.payload[4], seed.a[1]);

	auto response = sendRequest(request, std::chrono::milliseconds(2000));

	if(!response) {
		return false;
	}

	return seed.b[0] == readLE32(&response.payload[0]) && seed.b[1] == readLE32(&response.payload[4]);
}

bool RADCommunication::sendSoftware(const std::vector<uint8_t>& chipSoftware) {
	Log::Debug(__func__, "starting for " + targetChip.getName() + " (" + std::to_string(chipSoftware.size()) + " bytes)");
	static constexpr size_t MaxEthernetPayload = 1487;

	if(supportsZynqBinaryFlasher && chipSoftware.size() < sizeof(uint32_t)) {
		Log::Error(__func__, "firmware too short to contain a CRC for " + targetChip.getName());
		return false;
	}

	Log::Trace(__func__, "sending BeginTransfer for " + targetChip.getName());
	auto request = makeRequest(Request::Type::BeginTransfer, 0, 0);
	auto response = sendRequest(request, std::chrono::milliseconds(10000));

	if(!response) {
		Log::Error(__func__, "BeginTransfer failed for " + targetChip.getName());
		return false;
	}

	const size_t headerSize = 1 + (supportsZynqBinaryFlasher ? ZynqFlasherHeaderSize : 0); // Command byte plus the optional Zynq flasher header
	const size_t maxPayloadSize = MaxEthernetPayload - headerSize;

	size_t remainingBytes = chipSoftware.size();
	size_t currentlyReading = 0;

	logger(
		icsneo::Device::OpenStatusType::Progress,
		"Flashing " + targetChip.getName() + "...",
		std::nullopt
	);

	while(remainingBytes > 0) {
		const double progress = static_cast<double>(currentlyReading) / static_cast<double>(chipSoftware.size());

		logger(
			icsneo::Device::OpenStatusType::Progress,
			"Flashing " + targetChip.getName() + "...",
			progress
		);

		size_t numToSend = std::min<size_t>(maxPayloadSize, remainingBytes);
		request = makeRequest(Request::Type::FWPayload, numToSend, supportsZynqBinaryFlasher ? 2 : 0);

		std::copy(
			chipSoftware.begin() + currentlyReading,
			chipSoftware.begin() + currentlyReading + numToSend,
			request.payload
		);

		if(supportsFlashBuffer()) {
			if(!sendRequest(request, std::chrono::milliseconds(2000))) {
				return false;
			}
		} else if(!sendRequestNoResponse()) {
			return false;
		}

		currentlyReading += numToSend;
		remainingBytes -= numToSend;
		if(device.com->driver->isEthernet()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	Log::Trace(__func__, "sending EndTransfer for " + targetChip.getName());
	request = makeRequest(Request::Type::EndTransfer, 0, 0);
	response = sendRequest(request, std::chrono::milliseconds(10000));

	if(!response) {
		Log::Error(__func__, "EndTransfer failed for " + targetChip.getName());
		return false;
	}

	request = makeRequest(Request::Type::DownloadedBinInfo, 0, 4);
	response = sendRequest(request, std::chrono::milliseconds(6000));

	if(!response) {
		Log::Error(__func__, "DownloadedBinInfo failed for " + targetChip.getName());
		return false;
	}

	const uint32_t numDownloaded = readLE32(response.payload);

	if(numDownloaded != static_cast<uint32_t>(chipSoftware.size())) {
		Log::Error(__func__, "downloaded size mismatch for " + targetChip.getName() + ": expected " + std::to_string(chipSoftware.size()) + ", got " + std::to_string(numDownloaded));
		return false;
	}

	if(supportsZynqBinaryFlasher) {
		request = makeRequest(Request::Type::VerifyCRC, sizeof(uint32_t), 2);

		/**
		 * Copy CRC from end of the binary
		*/
		std::copy(chipSoftware.end() - sizeof(uint32_t), chipSoftware.end(), request.payload);

		response = sendRequest(request, std::chrono::milliseconds(2000));

		if(!response) {
			return false;
		}

		// The first response byte is 1 when the CRC matched
		if(response.payload[0] != 0x01) {
			Log::Error(__func__, "CRC mismatch for " + targetChip.getName());
			return false;
		}
	}

	logger(
		icsneo::Device::OpenStatusType::Progress,
		"Flashing " + targetChip.getName() + "...",
		1.0
	);

	Log::Debug(__func__, "completed successfully for " + targetChip.getName());
	return true;
}

bool RADCommunication::getProgrammingStatus(size_t sizeToValidate) {
	Log::Debug(__func__, "starting for " + targetChip.getName());
	auto request = makeRequest(Request::Type::ProgramFlash, 0, supportsZynqBinaryFlasher ? 2 : 0);

	if(!sendRequest(request, std::chrono::milliseconds(2000))) {
		Log::Error(__func__, "ProgramFlash failed for " + targetChip.getName());
		return false;
	}

	request = makeRequest(Request::Type::ProgrammingStatus, 0, 4);

	uint32_t status = 0;
	const auto beginTime = std::chrono::steady_clock::now();

	logger(
		icsneo::Device::OpenStatusType::Progress,
		"Programming " + targetChip.getName() + "...",
		std::nullopt
	);

	while(status < static_cast<uint32_t>(sizeToValidate)) {
		const double progress = static_cast<double>(status) / static_cast<double>(sizeToValidate);

		logger(
			icsneo::Device::OpenStatusType::Progress,
			"Programming " + targetChip.getName() + "...",
			progress
		);

		const auto& response = sendRequest(request, std::chrono::milliseconds(60000));
		if(!response) {
			return false;
		}
		status = readLE32(response.payload);

		if(std::chrono::steady_clock::now() - beginTime >= std::chrono::milliseconds(180000)) {
			return false;
		}
	}

	logger(
		icsneo::Device::OpenStatusType::Progress,
		"Programming " + targetChip.getName() + "...",
		1.0
	);

	Log::Debug(__func__, "completed successfully for " + targetChip.getName());
	return true;
}

bool RADCommunication::loadZips(const FWRepo& fw, std::vector<uint8_t>& chipSoftware) {
	if(const auto& segmentNames = targetChip.getMultiIEFName(); !segmentNames.empty()) {
		std::vector<std::unique_ptr<FWZip>> zips;
		zips.reserve(segmentNames.size());

		size_t totalSize = 0;
		for(const auto& name : segmentNames) {
			std::vector<uint8_t> segment;
			if(!fw.getFW(name, segment)) {
				return false;
			}

			zips.push_back(std::make_unique<FWZip>(std::move(segment)));
			auto& zip = zips.back();
			if(!zip->size()) {
				return false;
			}
			totalSize += *zip->size();
		}
		chipSoftware.resize(totalSize, 0);
		size_t offset = 0;
		for(auto& zip : zips) {
			if(!zip->readAll(chipSoftware.data() + offset)) {
				return false;
			}
			offset += *(zip->size());
		}
		return true;
	} else if(!targetChip.getIEFName().empty()) {
		std::vector<uint8_t> bin;
		if(!fw.getFW(targetChip.getIEFName(), bin)) {
			return false;
		}
		FWZip fwZip(std::move(bin));
		if(!fwZip.readAll(chipSoftware)) {
			return false;
		}
		return true;
	}
	// Should never happen
	return false;
}

bool RADCommunication::flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) {
	Log::Info(__func__, "starting flash for " + targetChip.getName());
	auto chipSrcVer = fw.getChipVersion(targetChip.getIEFName());
	if(!chipSrcVer) {
		Log::Error(__func__, "no chip version found for " + targetChip.getIEFName());
		return false;
	}

	std::vector<uint8_t> chipSoftware;
	if(!loadZips(fw, chipSoftware)) {
		Log::Error(__func__, "loadZips failed for " + targetChip.getName());
		return false;
	}

	logger = handler;
	if(!validateChipVersion(*chipSrcVer)) {
		Log::Error(__func__, "validateChipVersion failed for " + targetChip.getName());
		return false;
	}

	if(!sendSoftware(chipSoftware)) {
		Log::Error(__func__, "sendSoftware failed for " + targetChip.getName());
		return false;
	}

	if(!supportsFlashBuffer()) {
		if(!getProgrammingStatus(chipSoftware.size())) {
			Log::Error(__func__, "getProgrammingStatus failed for " + targetChip.getName());
			return false;
		}
	}

	Log::Info(__func__, "completed successfully for " + targetChip.getName());
	return true;
}
