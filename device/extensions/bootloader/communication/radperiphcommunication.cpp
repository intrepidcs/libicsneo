#include "icsneo/device/extensions/bootloader/communication/radperiphcommunication.h"
#include "icsneo/device/extensions/bootloader/parser/fwrepo.h"
#include "icsneo/device/extensions/bootloader/platform/logging.h"

#include <cstddef>
#include <thread>

#include <icsbl/icsbl.h>

using namespace icsneo::Bootloader;

RADPeriphCommunication::RADPeriphCommunication(icsneo::Device& dev, const ChipInfo& chipToFlash)
	: Communication(dev), targetChip(chipToFlash) {}

RADPeriphCommunication::~RADPeriphCommunication() {
	if(device.com->readsArePaused()) {
		device.com->resumeReads();
	}
}

bool RADPeriphCommunication::sendRequestNoResponse(Request req, std::vector<uint8_t>&& args) {
	return sendRequestNoResponse(req, targetChip.getID(), std::move(args));
}

bool RADPeriphCommunication::sendRequestNoResponse(Request req, ChipID id, std::vector<uint8_t>&& args) {
	args.insert(args.begin(), {
		(uint8_t)id,
		(uint8_t)req,
	});
	return device.com->sendCommand(icsneo::Command::PeripheralBootloader, args);
}

RADPeriphCommunication::Response RADPeriphCommunication::sendRequest(Request req, std::vector<uint8_t>&& args, std::chrono::milliseconds timeout) {
	return sendRequest(req, targetChip.getID(), std::move(args), timeout);
}

RADPeriphCommunication::Response RADPeriphCommunication::sendRequest(Request req, ChipID id, std::vector<uint8_t>&& args, std::chrono::milliseconds timeout) {
	// Total reply sizes, including the header bytes
	size_t expectedResponseLength = 0;
	bool checkBlAccepted = false;
	switch(req) {
		case Request::Mode:
			expectedResponseLength = 6;
			break;
		case Request::Version:
			expectedResponseLength = 24;
			checkBlAccepted = true;
			break;
		case Request::Authenticate:
			expectedResponseLength = 13;
			checkBlAccepted = true;
			break;
		case Request::Echo:
			expectedResponseLength = 3;
			break;
		case Request::Erase:
		case Request::FirmwareValidate:
		case Request::FlashSector:
			expectedResponseLength = 3;
			checkBlAccepted = true;
			break;
		default:
			expectedResponseLength = 3;
			break;
	}

	auto& rb = device.com->driver->getReadBuffer();
	rb.clear();
	if(!sendRequestNoResponse(req, id, std::move(args))) {
		Log::Error(__func__, "Failed to send request " + std::to_string(static_cast<uint8_t>(req)));
		return {};
	}

	auto result = device.com->driver->waitForRx(expectedResponseLength, timeout);
	if(!result) {
		Log::Error(__func__, "No response to request " + std::to_string(static_cast<uint8_t>(req)));
		return {};
	}

	Response resp;
	resp.data.resize(expectedResponseLength, 0);
	rb.read(resp.data.data(), 0, expectedResponseLength);
	resp.success = resp.data[ReplySuccessOffset] == ReplySuccess;
	if(!resp.success) {
		Log::Error(__func__, "The peripheral reported failure for request " + std::to_string(static_cast<uint8_t>(req)));
		return resp;
	}
	if(checkBlAccepted && resp.data[ReplyAcceptedOffset] != ReplyAccepted) {
		Log::Error(__func__, "The bootloader rejected request " + std::to_string(static_cast<uint8_t>(req)));
		return {};
	}

	return resp;
}

RADPeriphCommunication::Mode RADPeriphCommunication::getMode() {
	auto resp = sendRequest(Request::Mode, {}, std::chrono::milliseconds(2000));

	if(!resp) {
		return Mode::NoAnswer;
	}
	static constexpr size_t ModeOffset = 4;
	return (Mode)resp.data[ModeOffset];
}

bool RADPeriphCommunication::sendPeriphTest() {
	// Echo is sent to chip id 0, the reply carries the bitwise inverse of the test byte
	const uint8_t testByte = 0x00;
	auto resp = sendRequest(Request::Echo, (ChipID)0x00, {testByte}, std::chrono::milliseconds(2000));

	if(!resp) {
		return false;
	}

	if(resp.data[2] != static_cast<uint8_t>(~testByte)) {
		Log::Error(__func__, "The peripheral echo reply did not match");
		return false;
	}

	return true;
}

bool RADPeriphCommunication::readSerialNumber() {
	auto resp = sendRequest(Request::Version, {0x00}, std::chrono::milliseconds(2000));
	if(!resp) {
		return false;
	}

	// The reply payload is the start of a BootloaderVersionResponse
	static constexpr size_t SerialOffset = ReplyHeaderSize + offsetof(BootloaderVersionResponse, serialNum);
	serialNumber = readLE32(resp.data.data() + SerialOffset);
	return true;
}

bool RADPeriphCommunication::authenticate() {
	icsbl_seed_v1 seed = {};
	if(!icsbl_generate_seed_v1(serialNumber, &seed)) {
		Log::Error(__func__, "libicsbl could not generate an authentication challenge");
		return false;
	}

	std::vector<uint8_t> args(BootloaderArgsOffset + sizeof(seed.a), 0);
	writeLE32(args.data() + BootloaderArgsOffset, seed.a[0]);
	writeLE32(args.data() + BootloaderArgsOffset + 4, seed.a[1]);

	auto resp = sendRequest(Request::Authenticate, std::move(args), std::chrono::milliseconds(3000));
	if(!resp) {
		return false;
	}

	// seed.b follows the header and one more byte
	static constexpr size_t ReplyOffset = ReplyHeaderSize + 1;
	return readLE32(resp.data.data() + ReplyOffset) == seed.b[0] &&
		readLE32(resp.data.data() + ReplyOffset + 4) == seed.b[1];
}

bool RADPeriphCommunication::validateFirmware() {
	return sendRequest(Request::FirmwareValidate, {0x00u}, std::chrono::milliseconds(2000));
}

bool RADPeriphCommunication::flashSector(const IEF::Sector& sector) {
	if(sector.second.size() < FlashSectorFirstChunkSize) {
		Log::Error(__func__, "Invalid sector length");
		return false;
	}

	// The first request is the 32-bit little-endian address followed by the first chunk, encrypted
	std::vector<uint8_t> args(BootloaderArgsOffset + sizeof(uint32_t) + FlashSectorFirstChunkSize, 0);
	writeLE32(args.data() + BootloaderArgsOffset, sector.first);
	icsbl_encrypt_v1(serialNumber, sector.second.begin(), FlashSectorFirstChunkSize, args.data() + BootloaderArgsOffset + sizeof(uint32_t));
	if(!sendRequestNoResponse(Request::FlashSector, std::move(args))) {
		return false;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(400));

	// The second request is the rest of the sector, encrypted
	const size_t bytesLeft = sector.second.size() - FlashSectorFirstChunkSize;
	args = std::vector<uint8_t>(bytesLeft, 0);
	icsbl_encrypt_v1(serialNumber, sector.second.begin() + FlashSectorFirstChunkSize, bytesLeft, args.data());
	return sendRequest(Request::FlashSector, std::move(args), std::chrono::milliseconds(3000));
}

bool RADPeriphCommunication::flash(const FWRepo& fw, const icsneo::Device::OpenStatusHandler& handler) {
	// Validate the firmware before touching the device so a bad file can not leave it erased
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

	const auto& sectors = ief.getSectors();
	uint32_t totalSize = 0;
	for(const auto& sector : sectors) {
		if(sector.second.size() < FlashSectorFirstChunkSize) {
			Log::Error(__func__, "Invalid sector length in IEF for " + targetChip.getName());
			return false;
		}
		totalSize += static_cast<uint32_t>(sector.second.size());
	}

	if(!readSerialNumber()) {
		return false;
	}

	if(!authenticate()) {
		return false;
	}

	if(!sendRequest(Request::Erase)) {
		Log::Error(__func__, "Failed to erase firmware");
		return false;
	}

	handler(
		icsneo::Device::OpenStatusType::Progress,
		"Flashing " + targetChip.getName() + "...",
		std::nullopt
	);

	uint32_t sizeFlashed = 0;

	for(const auto& sector : sectors) {
		if(!flashSector(sector)) {
			return false;
		}

		sizeFlashed += static_cast<uint32_t>(sector.second.size());
		handler(
			icsneo::Device::OpenStatusType::Progress,
			"Flashing " + targetChip.getName() + "...",
			(double)sizeFlashed / (double)totalSize
		);
	}

	return validateFirmware();
}

bool RADPeriphCommunication::waitForConnect(std::chrono::milliseconds timeout) {
	const auto deadline = std::chrono::steady_clock::now() + timeout;
	while(std::chrono::steady_clock::now() < deadline) {
		if(sendPeriphTest()) {
			return true;
		}
	}

	return false;
}

bool RADPeriphCommunication::open() {
	device.com->pauseReads();

	int attempts = 3;
	do {
		mode = getMode();
		if(mode == Mode::NoAnswer) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			mode = getMode();
			if(mode == Mode::NoAnswer) {
				return false;
			}
		}

		if(mode == Mode::MainApplication) {
			if(!sendRequestNoResponse(Request::SwitchToBootloader)) {
				return false;
			}
			if(!device.reconnect(std::chrono::seconds(30))) {
				return false;
			}
			if(!waitForConnect(std::chrono::seconds(30))) {
				return false;
			}
		} else if(mode == Mode::Bootloader) {
			return true;
		}
	} while(attempts--);

	return false;
}

bool RADPeriphCommunication::close() {
	if(mode != Mode::Bootloader) {
		device.com->resumeReads();
		return true;
	}
	if(!sendRequestNoResponse(Request::SwitchToApp)) {
		return false;
	}

	std::this_thread::sleep_for(std::chrono::seconds(5)); // measured at 2s, 5s for buffer, yes this is needed

	if(!device.reconnect(std::chrono::seconds(30))) {
		return false;
	}

	if(!waitForConnect()) {
		return false;
	}

	mode = getMode();
	if(mode == Mode::MainApplication) {
		device.com->resumeReads();
		return true;
	}
	return false;
}

bool RADPeriphCommunication::enterApplication() {
	return true;
}

bool RADPeriphCommunication::finalize() {
	return true;
}

bool RADPeriphCommunication::hasFinalizationStep() const {
	return false;
}
