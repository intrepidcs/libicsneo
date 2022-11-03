#include "icsneo/device/device.h"
#include "icsneo/communication/message/callback/messagecallback.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/communication/command.h"
#include "icsneo/device/extensions/deviceextension.h"
#include "icsneo/disk/fat.h"
#include "icsneo/communication/packet/wivicommandpacket.h"
#include "icsneo/communication/message/wiviresponsemessage.h"
#include "icsneo/communication/message/scriptstatusmessage.h"
#include "icsneo/communication/message/extendedresponsemessage.h"
#include <string.h>
#include <iostream>
#include <sstream>
#include <chrono>

using namespace icsneo;

static const uint8_t fromBase36Table[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 };

static const char toBase36Table[36] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E',
	'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

static const uint32_t toBase36Powers[7] = { 1, 36, 1296, 46656, 1679616, 60466176, 2176782336 };

#define MIN_BASE36_SERIAL (16796160)
#define MAX_SERIAL (2176782335)

std::string Device::SerialNumToString(uint32_t serial) {
	if(serial == 0 || serial > MAX_SERIAL)
		return "0";

	std::stringstream ss;
	if(serial >= MIN_BASE36_SERIAL) {
		for (auto i = 5; i >= 0; i--) {
			ss << toBase36Table[serial / toBase36Powers[i]];
			serial = serial % toBase36Powers[i];
		}
	} else {
		ss << serial;
	}
	return ss.str();
}

uint32_t Device::SerialStringToNum(const std::string& serial) {
	if(Device::SerialStringIsNumeric(serial)) {
		try {
			return std::stoi(serial);
		} catch(...) {
			return 0;
		}
	}

	if(serial.length() != 6)
		return 0; // Non-numeric serial numbers should be 6 characters

	uint32_t ret = 0;
	for (auto i = 0; i < 6; i++) {
		ret *= 36;
		ret += fromBase36Table[(unsigned char)serial[i]];
	}
	return ret;
}

bool Device::SerialStringIsNumeric(const std::string& serial) {
	if(serial.length() == 0)
		return false;

	if(serial.length() == 1)
		return isdigit(serial[0]);

	// Check the first two characters, at least one should be a character if we need to do a base36 conversion
	return isdigit(serial[0]) && isdigit(serial[1]);
}

Device::~Device() {
	if(isMessagePollingEnabled())
		disableMessagePolling();
	if(isOpen())
		close();
}

uint16_t Device::getTimestampResolution() const {
	return com->decoder->timestampResolution;
}

std::string Device::describe() const {
	std::stringstream ss;
	ss << getProductName() << ' ' << getSerial();
	return ss.str();
}

bool Device::enableMessagePolling() {
	if(isMessagePollingEnabled()) {// We are already polling
		report(APIEvent::Type::DeviceCurrentlyPolling, APIEvent::Severity::Error);
		return false;
	}
	messagePollingCallbackID = com->addMessageCallback(std::make_shared<MessageCallback>([this](std::shared_ptr<Message> message) {
		pollingContainer.enqueue(message);
		enforcePollingMessageLimit();
	}));
	return true;
}

bool Device::disableMessagePolling() {
	if(!isMessagePollingEnabled()) {
		report(APIEvent::Type::DeviceNotCurrentlyPolling, APIEvent::Severity::Error);
		return false; // Not currently polling
	}
	auto ret = com->removeMessageCallback(messagePollingCallbackID);
	getMessages(); // Flush any messages still in the container
	messagePollingCallbackID = 0;
	return ret;
}

// Returns a pair of {vector, bool}, where the vector contains shared_ptrs to the returned msgs and the bool is whether or not the call was successful.
std::pair<std::vector<std::shared_ptr<Message>>, bool> Device::getMessages() {
	std::vector<std::shared_ptr<Message>> ret;
	bool retBool = getMessages(ret);
	return std::make_pair(ret, retBool);
}

bool Device::getMessages(std::vector<std::shared_ptr<Message>>& container, size_t limit, std::chrono::milliseconds timeout) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	if(!isOnline()) {
		report(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error);
		return false;
	}

	if(!isMessagePollingEnabled()) {
		report(APIEvent::Type::DeviceNotCurrentlyPolling, APIEvent::Severity::Error);
		return false;
	}

	// A limit of zero indicates no limit
	if(limit == 0)
		limit = (size_t)-1;

	if(limit > (pollingContainer.size_approx() + 4))
		limit = (pollingContainer.size_approx() + 4);

	if(container.size() < limit)
		container.resize(limit);

	size_t actuallyRead;
	if(timeout != std::chrono::milliseconds(0))
		actuallyRead = pollingContainer.wait_dequeue_bulk_timed(container.data(), limit, timeout);
	else
		actuallyRead = pollingContainer.try_dequeue_bulk(container.data(), limit);

	if(container.size() > actuallyRead)
		container.resize(actuallyRead);

	return true;
}

void Device::enforcePollingMessageLimit() {
	while(pollingContainer.size_approx() > pollingMessageLimit) {
		std::shared_ptr<Message> throwAway;
		pollingContainer.try_dequeue(throwAway);
		report(APIEvent::Type::PollingMessageOverflow, APIEvent::Severity::EventWarning);
	}
}

bool Device::open(OpenFlags flags, OpenStatusHandler handler) {
	if(!com) {
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
		return false;
	}

	if(!com->open()) {
		return false;
	}

	APIEvent::Type attemptErr = attemptToBeginCommunication();
	if(attemptErr != APIEvent::Type::NoErrorFound) {
		// We could not communicate with the device, let's see if an extension can
		bool tryAgain = false;
		forEachExtension([&tryAgain, &flags, &handler](const std::shared_ptr<DeviceExtension>& ext) -> bool {
			if(ext->onDeviceCommunicationDead(flags, handler))
				tryAgain = true;
			return true;
		});
		if(!tryAgain) {
			com->close();
			report(attemptErr, APIEvent::Severity::Error);
			return false; // Extensions couldn't save us
		}
		attemptErr = attemptToBeginCommunication();
		if(attemptErr != APIEvent::Type::NoErrorFound) {
			com->close();
			report(attemptErr, APIEvent::Severity::Error);
			return false;
		}
	}

	bool block = false;
	forEachExtension([&block, &flags, &handler](const std::shared_ptr<DeviceExtension>& ext) {
		if(ext->onDeviceOpen(flags, handler))
			return true;
		block = true;
		return false;
	});
	if(block) // Extensions say no
		return false;

	if(!settings->disabled) {
		// Since we will not fail the open if a settings read fails,
		// downgrade any errors to warnings. Otherwise the error will
		// go unnoticed in the opening thread's getLastError buffer.
		const bool downgrading = EventManager::GetInstance().isDowngradingErrorsOnCurrentThread();
		if(!downgrading)
			EventManager::GetInstance().downgradeErrorsOnCurrentThread();
		settings->refresh();
		if(!downgrading)
			EventManager::GetInstance().cancelErrorDowngradingOnCurrentThread();
	}

	MessageFilter filter;
	filter.includeInternalInAny = true;
	internalHandlerCallbackID = com->addMessageCallback(std::make_shared<MessageCallback>(filter, [this](std::shared_ptr<Message> message) {
		handleInternalMessage(message);
	}));

	heartbeatThread = std::thread([this]() {
		EventManager::GetInstance().downgradeErrorsOnCurrentThread();

		MessageFilter filter;
		filter.includeInternalInAny = true;
		std::atomic<bool> receivedMessage{false};
		auto messageReceivedCallbackID = com->addMessageCallback(std::make_shared<MessageCallback>(filter, [&receivedMessage](std::shared_ptr<Message> message) {
			receivedMessage = true;
		}));

		// Give the device time to get situated
		auto i = 150;
		while(!stopHeartbeatThread && i != 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			i--;
		}

		while(!stopHeartbeatThread) {
			// Wait for 110ms for a possible heartbeat
			std::this_thread::sleep_for(std::chrono::milliseconds(110));
			if(receivedMessage) {
				receivedMessage = false;
			} else {
				// Some communication, such as the bootloader and extractor interfaces, must
				// redirect the input stream from the device as it will no longer be in the
				// packet format we expect here. As a result, status updates will not reach
				// us here and suppressDisconnects() must be used. We don't want to request
				// a status and then redirect the stream, as we'll then be polluting an
				// otherwise quiet stream. This lock makes sure suppressDisconnects() will
				// block until we've either gotten our status update or disconnected from
				// the device.
				std::lock_guard<std::mutex> lk(heartbeatMutex);
				if(heartbeatSuppressed())
					continue;

				// No heartbeat received, request a status
				com->sendCommand(Command::RequestStatusUpdate);
				// The response should come back quickly if the com is quiet
				std::this_thread::sleep_for(std::chrono::milliseconds(50));
				// Check if we got a message, and if not, if settings are being applied
				if(receivedMessage) {
					receivedMessage = false;
				} else {
					if(!stopHeartbeatThread && !isDisconnected())
						report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
					break;
				}
			}
		}

		com->removeMessageCallback(messageReceivedCallbackID);
	});

	return true;
}

APIEvent::Type Device::attemptToBeginCommunication() {
	versions.clear();

	if(!afterCommunicationOpen()) {
		// Very unlikely, at the time of writing this only fails if rawWrite does.
		// If you're looking for this error, you're probably looking for if(!serial) below.
		// "Communication could not be established with the device. Perhaps it is not powered with 12 volts?"
		return getCommunicationNotEstablishedError();
	}

	auto serial = com->getSerialNumberSync();
	int i = 0;
	while(!serial) {
		serial = com->getSerialNumberSync();
		if(i++ > 5)
			break;
	}
	if(!serial) // "Communication could not be established with the device. Perhaps it is not powered with 12 volts?"
		return getCommunicationNotEstablishedError();

	std::string currentSerial = getNeoDevice().serial;
	if(currentSerial != serial->deviceSerial)
		return APIEvent::Type::IncorrectSerialNumber;

	auto maybeVersions = com->getVersionsSync();
	if(!maybeVersions)
		return getCommunicationNotEstablishedError();
	else
		versions = std::move(*maybeVersions);

	return APIEvent::Type::NoErrorFound;
}

bool Device::close() {
	if(!com) {
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
		return false;
	}

	stopHeartbeatThread = true;

	if(isOnline())
		goOffline();

	if(internalHandlerCallbackID)
		com->removeMessageCallback(internalHandlerCallbackID);

	internalHandlerCallbackID = 0;

	if(heartbeatThread.joinable())
		heartbeatThread.join();
	stopHeartbeatThread = false;

	forEachExtension([](const std::shared_ptr<DeviceExtension>& ext) { ext->onDeviceClose(); return true; });
	return com->close();
}

bool Device::goOnline() {
	if(!com->sendCommand(Command::EnableNetworkCommunication, true))
		return false;

	auto startTime = std::chrono::system_clock::now();

	ledState = LEDState::Online;

	updateLEDState();

	std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Network::NetID::Reset_Status);
	filter->includeInternalInAny = true;

	// Wait until communication is enabled or 5 seconds, whichever comes first
	while((std::chrono::system_clock::now() - startTime) < std::chrono::seconds(5)) {
		if(latestResetStatus && latestResetStatus->comEnabled)
			break;

		bool failOut = false;
		com->waitForMessageSync([this, &failOut]() {
			if(!com->sendCommand(Command::RequestStatusUpdate)) {
				failOut = true;
				return false;
			}
			return true;
		}, filter, std::chrono::milliseconds(100));
		if(failOut)
			return false;
	}

	online = true;

	forEachExtension([](const std::shared_ptr<DeviceExtension>& ext) { ext->onGoOnline(); return true; });
	return true;
}

bool Device::goOffline() {
	forEachExtension([](const std::shared_ptr<DeviceExtension>& ext) { ext->onGoOffline(); return true; });

	if(isDisconnected()) {
		online = false;
		return true;
	}

	if(!com->sendCommand(Command::EnableNetworkCommunication, false))
		return false;

	auto startTime = std::chrono::system_clock::now();

	ledState = (latestResetStatus && latestResetStatus->cmRunning) ? LEDState::CoreMiniRunning : LEDState::Offline;

	updateLEDState();

	std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Network::NetID::Reset_Status);
	filter->includeInternalInAny = true;

	// Wait until communication is disabled or 5 seconds, whichever comes first
	while((std::chrono::system_clock::now() - startTime) < std::chrono::seconds(5)) {
		if(latestResetStatus && !latestResetStatus->comEnabled)
			break;

		if(!com->sendCommand(Command::RequestStatusUpdate))
			return false;

		com->waitForMessageSync(filter, std::chrono::milliseconds(100));
	}

	online = false;

	return true;
}

int8_t Device::prepareScriptLoad() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	static std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Network::NetID::CoreMiniPreLoad);

	if(!com->sendCommand(Command::CoreMiniPreload))
		return false;

	int8_t retVal = 0;
	while(retVal == 0)
	{
		auto generic = com->waitForMessageSync(filter, std::chrono::milliseconds(1000));

		if(!generic) {
			report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
			return false;
		}

		const auto resp = std::static_pointer_cast<RawMessage>(generic);
		retVal = (int8_t)resp->data[0];
	}

	return retVal;
}

bool Device::startScript()
{
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	uint8_t LocationSdCard = 1; //Only support starting a coremini in an SDCard
	auto generic = com->sendCommand(Command::LoadCoreMini, LocationSdCard);

	if(!generic)
	{
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	return true;
}

bool Device::stopScript()
{
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	auto generic = com->sendCommand(Command::ClearCoreMini);

	if(!generic)
	{
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	return true;
}

bool Device::clearScript()
{
	if(!stopScript())
		return false;

	std::vector<uint8_t> clearData(512, 0xCD);
	uint64_t ScriptLocation = 0; //We only support a coremini in an SDCard, which is at the very beginning of the card
	auto written = writeLogicalDisk(ScriptLocation, clearData.data(), clearData.size());

	return written > 0;
}

bool Device::transmit(std::shared_ptr<Frame> frame) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	if(!isOnline()) {
		report(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error);
		return false;
	}

	if(!isSupportedTXNetwork(frame->network)) {
		report(APIEvent::Type::UnsupportedTXNetwork, APIEvent::Severity::Error);
		return false;
	}

	bool extensionHookedTransmit = false;
	bool transmitStatusFromExtension = false;
	forEachExtension([&](const std::shared_ptr<DeviceExtension>& ext) {
		if(!ext->transmitHook(frame, transmitStatusFromExtension))
			extensionHookedTransmit = true;
		return !extensionHookedTransmit; // false breaks out of the loop early
	});
	if(extensionHookedTransmit)
		return transmitStatusFromExtension;

	std::vector<uint8_t> packet;
	if(!com->encoder->encode(*com->packetizer, packet, frame))
		return false;

	return com->sendPacket(packet);
}

bool Device::transmit(std::vector<std::shared_ptr<Frame>> frames) {
	for(auto& frame : frames) {
		if(!transmit(frame))
			return false;
	}
	return true;
}

void Device::setWriteBlocks(bool blocks) {
	com->setWriteBlocks(blocks);
}

size_t Device::getNetworkCountByType(Network::Type type) const {
	size_t count = 0;
	for(const auto& net : getSupportedRXNetworks())
		if(net.getType() == type)
			count++;
	return count;
}

// Indexed starting at one
Network Device::getNetworkByNumber(Network::Type type, size_t index) const {
	size_t count = 0;
	for(const auto& net : getSupportedRXNetworks()) {
		if(net.getType() == type) {
			count++;
			if(count == index)
				return net;
		}
	}
	return Network::NetID::Invalid;
}

std::optional<uint64_t> Device::readLogicalDisk(uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) {
	if(!into || timeout <= std::chrono::milliseconds(0)) {
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return std::nullopt;
	}

	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	std::lock_guard<std::mutex> lk(diskLock);

	if(diskReadDriver->getAccess() == Disk::Access::EntireCard && diskWriteDriver->getAccess() == Disk::Access::VSA) {
		// We have mismatched drivers, we need to add an offset to the diskReadDriver
		const auto offset = Disk::FindVSAInFAT([this, &timeout](uint64_t pos, uint8_t *into, uint64_t amount) {
			const auto start = std::chrono::steady_clock::now();
			auto ret = diskReadDriver->readLogicalDisk(*com, report, pos, into, amount, timeout);
			timeout -= std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
			return ret;
		});
		if(!offset.has_value())
			return std::nullopt;
		diskReadDriver->setVSAOffset(*offset);
	}

	// This is needed for certain read drivers which take over the communication stream
	const auto lifetime = suppressDisconnects();

	return diskReadDriver->readLogicalDisk(*com, report, pos, into, amount, timeout);
}

std::optional<uint64_t> Device::writeLogicalDisk(uint64_t pos, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout) {
	if(!from || timeout <= std::chrono::milliseconds(0)) {
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return std::nullopt;
	}

	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	std::lock_guard<std::mutex> lk(diskLock);
	return diskWriteDriver->writeLogicalDisk(*com, report, *diskReadDriver, pos, from, amount, timeout);
}

std::optional<bool> Device::isLogicalDiskConnected() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	// This doesn't *really* make sense here but because the disk read redirects the parser until it is done, we'll lock this
	// just to avoid the timeout.
	std::lock_guard<std::mutex> lg(diskLock);
	const auto info = com->getLogicalDiskInfoSync();
	if (!info) {
		report(APIEvent::Type::Timeout, APIEvent::Severity::Error);
		return std::nullopt;
	}

	return info->connected;
}

std::optional<uint64_t> Device::getLogicalDiskSize() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	// This doesn't *really* make sense here but because the disk read redirects the parser until it is done, we'll lock this
	// just to avoid the timeout.
	std::lock_guard<std::mutex> lg(diskLock);
	const auto info = com->getLogicalDiskInfoSync();
	if (!info) {
		report(APIEvent::Type::Timeout, APIEvent::Severity::Error);
		return std::nullopt;
	}

	return info->getReportedSize();
}

std::optional<uint64_t> Device::getVSAOffsetInLogicalDisk() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	std::lock_guard<std::mutex> lk(diskLock);

	if (diskReadDriver->getAccess() == Disk::Access::VSA || diskReadDriver->getAccess() == Disk::Access::None)
		return 0ull;
	
	auto offset = Disk::FindVSAInFAT([this](uint64_t pos, uint8_t *into, uint64_t amount) {
		return diskReadDriver->readLogicalDisk(*com, report, pos, into, amount);
	});
	if(!offset.has_value())
		return std::nullopt;

	if(diskReadDriver->getAccess() == Disk::Access::EntireCard && diskWriteDriver->getAccess() == Disk::Access::VSA) {
		// We have mismatched drivers, we need to add an offset to the diskReadDriver
		diskReadDriver->setVSAOffset(*offset);
		return 0ull;
	}
	return *offset;
}

std::optional<bool> Device::getDigitalIO(IO type, size_t number /* = 1 */) {
	if(number == 0) { // Start counting from 1
		report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
		return false;
	}

	std::lock_guard<std::mutex> lk(ioMutex);
	switch(type) {
	case IO::EthernetActivation:
		if(getEthernetActivationLineCount() < number)
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		if(!ethActivationStatus.has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return ethActivationStatus;
	case IO::USBHostPower:
		if(getUSBHostPowerCount() < number)
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		if(!usbHostPowerStatus.has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return usbHostPowerStatus;
	case IO::BackupPowerEnabled:
		if(!getBackupPowerSupported())
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		if(!backupPowerEnabled.has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return backupPowerEnabled;
	case IO::BackupPowerGood:
		if(!getBackupPowerSupported())
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		if(!backupPowerGood.has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return backupPowerGood;
	case IO::Misc: {
		bool found = false;
		for(const auto& misc : getMiscIO()) {
			if(misc.number == number) {
				found = misc.supportsDigitalIn;
				break;
			}
		}
		if(!found)
			break; // ParameterOutOfRange

		if(number > miscDigital.size())
			break; // ParameterOutOfRange

		if(!miscDigital[number - 1].has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return miscDigital[number - 1];
	}
	case IO::EMisc: {
		bool found = false;
		for(const auto& misc : getEMiscIO()) {
			if(misc.number == number) {
				found = misc.supportsDigitalIn;
				break;
			}
		}
		if(!found)
			break; // ParameterOutOfRange

		if(number > miscDigital.size())
			break; // ParameterOutOfRange

		// If there is ever a device with overlapping misc IOs and emisc IOs,
		// you will need to make a new member variable for the emisc IOs.
		if(!miscDigital[number - 1].has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return miscDigital[number - 1];
	}
	};

	report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
	return std::nullopt;
}

bool Device::setDigitalIO(IO type, size_t number, bool value) {
	if(number == 0) { // Start counting from 1
		report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
		return false;
	}

	std::lock_guard<std::mutex> lk(ioMutex);
	switch(type) {
	case IO::EthernetActivation:
		if(getEthernetActivationLineCount() < number)
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		ethActivationStatus = value;

		return com->sendCommand(Command::MiscControl, {
			uint8_t(1), uint8_t(value ? 1 : 0), // enetActivateSet, enetActivateValue
			uint8_t(0), uint8_t(0), // usbHostPowerSet, usbHostPowerValue
			uint8_t(0), uint8_t(0) // backupPowerSet, backupPowerValue
		});
	case IO::USBHostPower:
		if(getUSBHostPowerCount() < number)
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		usbHostPowerStatus = value;

		return com->sendCommand(Command::MiscControl, {
			uint8_t(0), uint8_t(0), // enetActivateSet, enetActivateValue
			uint8_t(1), uint8_t(value ? 1 : 0), // usbHostPowerSet, usbHostPowerValue
			uint8_t(0), uint8_t(0) // backupPowerSet, backupPowerValue
		});
	case IO::BackupPowerEnabled:
		if(!getBackupPowerSupported())
			break; // ParameterOutOfRange
		assert(number == 1); // If you implement a device with more, you'll need to modify the accessor

		backupPowerEnabled = value;

		return com->sendCommand(Command::MiscControl, {
			uint8_t(0), uint8_t(0), // enetActivateSet, enetActivateValue
			uint8_t(0), uint8_t(value ? 1 : 0), // usbHostPowerSet, usbHostPowerValue (set to work around firmware bug)
			uint8_t(1), uint8_t(value ? 1 : 0) // backupPowerSet, backupPowerValue
		});
	case IO::BackupPowerGood:
		break; // Read-only, return ParameterOutOfRange
	case IO::Misc:
	case IO::EMisc:
		break; // Read-only for the moment, return ParameterOutOfRange
	};

	report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
	return false;
}

std::optional<double> Device::getAnalogIO(IO type, size_t number /* = 1 */) {
	if(number == 0) { // Start counting from 1
		report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
		return false;
	}

	std::lock_guard<std::mutex> lk(ioMutex);
	switch(type) {
	case IO::EthernetActivation:
	case IO::USBHostPower:
	case IO::BackupPowerEnabled:
	case IO::BackupPowerGood:
		break;
	case IO::Misc: {
		bool found = false;
		for(const auto& misc : getMiscIO()) {
			if(misc.number == number) {
				found = misc.supportsAnalogIn;
				break;
			}
		}
		if(!found)
			break; // ParameterOutOfRange

		if(number > miscAnalog.size())
			break; // ParameterOutOfRange

		if(!miscAnalog[number - 1].has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return miscAnalog[number - 1];
	}
	case IO::EMisc: {
		bool found = false;
		for(const auto& misc : getEMiscIO()) {
			if(misc.number == number) {
				found = misc.supportsAnalogIn;
				break;
			}
		}
		if(!found)
			break; // ParameterOutOfRange

		if(number > miscAnalog.size())
			break; // ParameterOutOfRange

		// If there is ever a device with overlapping misc IOs and emisc IOs,
		// you will need to make a new member variable for the emisc IOs.
		if(!miscAnalog[number - 1].has_value())
			report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);

		return miscAnalog[number - 1];
	}
	};

	report(APIEvent::Type::ParameterOutOfRange, APIEvent::Severity::Error);
	return std::nullopt;
}

void Device::wiviThreadBody() {
	std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::WiVICommandResponse);
	std::unique_lock<std::mutex> lk(wiviMutex);
	// Disk access commands can 
	std::unique_lock<std::mutex> dl(diskLock);
	dl.unlock();
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	bool first = true;
	while(!stopWiVIThread) {
		if(first) // Skip the first wait
			first = false;
		else
			stopWiVIcv.wait_for(lk, std::chrono::seconds(3));

		// Use the command GetAll to get a WiVI::Info structure from the device
		dl.lock();
		const auto generic = com->waitForMessageSync([this]() {
			return com->sendCommand(Command::WiVICommand, WiVI::CommandPacket::GetAll::Encode());
		}, filter);
		dl.unlock();

		if(!generic || generic->type != Message::Type::WiVICommandResponse) {
			report(APIEvent::Type::WiVIStackRefreshFailed, APIEvent::Severity::Error);
			continue;
		}

		const auto resp = std::static_pointer_cast<WiVI::ResponseMessage>(generic);
		if(!resp->success || !resp->info.has_value()) {
			report(APIEvent::Type::WiVIStackRefreshFailed, APIEvent::Severity::Error);
			continue;
		}
		
		// Now we know we have a WiVI::Info structure

		// Don't process captures unless there is a callback attached,
		// we don't want to clear any while nobody's listening.
		bool processCaptures = false;
		for(const auto& cb : newCaptureCallbacks) {
			if(cb) {
				processCaptures = true;
				break;
			}
		}

		if(processCaptures) {
			std::vector<uint8_t> clearMasks;
			size_t i = 0;
			for(const auto& capture : resp->info->captures) {
				i++;

				if(capture.flags.uploadOverflow)
					report(APIEvent::Type::WiVIUploadStackOverflow, APIEvent::Severity::Error);

				const auto MaxUploads = sizeof(capture.uploadStack) / sizeof(capture.uploadStack[0]);
				auto uploadCount = capture.flags.uploadStackSize + 1u;
				if(uploadCount > MaxUploads) {
					report(APIEvent::Type::WiVIStackRefreshFailed, APIEvent::Severity::Error);
					uploadCount = MaxUploads;
				}

				for(size_t j = 0; j < uploadCount; j++) {
					const auto& upload = capture.uploadStack[j];
					if(!upload.flags.pending)
						continue; // Not complete yet, don't notify

					// Schedule this upload to be cleared from the firmware's stack
					if(clearMasks.size() != resp->info->captures.size())
						clearMasks.resize(resp->info->captures.size());
					clearMasks[i] |= (1 << j);

					// Notify the client
					for(const auto& cb : newCaptureCallbacks) {
						if(cb) {
							lk.unlock();
							try {
								cb(upload.startSector, upload.endSector);
							} catch(...) {
								report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
							}
							lk.lock();
						}
					}
				}
			}

			if(!clearMasks.empty()) {
				const auto clearMasksGenericResp = com->waitForMessageSync([this, &clearMasks]() {
					return com->sendCommand(Command::WiVICommand, WiVI::CommandPacket::ClearUploads::Encode(clearMasks));
				}, filter);

				if(!clearMasksGenericResp
					|| clearMasksGenericResp->type != Message::Type::WiVICommandResponse
					|| !std::static_pointer_cast<WiVI::ResponseMessage>(clearMasksGenericResp)->success)
					report(APIEvent::Type::WiVIStackRefreshFailed, APIEvent::Severity::Error);
			}
		}

		// Process sleep requests
		if(resp->info->sleepRequest & 1 /* sleep requested by VSSAL */) {
			// Notify any callers we haven't notified yet
			for(auto& cb : sleepRequestedCallbacks) {
				if(!cb.second && cb.first) {
					cb.second = true;
					lk.unlock();
					try {
						cb.first(resp->info->connectionTimeoutMinutes);
					} catch(...) {
						report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
					}
					lk.lock();
				}
			}
		} else {
			// If the sleepRequest becomes 1 again we will notify again
			for(auto& cb : sleepRequestedCallbacks)
				cb.second = false;
		}
	}
}

void Device::stopWiVIThreadIfNecessary(std::unique_lock<std::mutex> lk) {
	// The callbacks will be empty std::functions if they are removed
	for(const auto& cb : newCaptureCallbacks) {
		if(cb)
			return; // We still need the WiVI Thread
	}

	for(const auto& cb : sleepRequestedCallbacks) {
		if(cb.first)
			return; // We still need the WiVI Thread
	}

	stopWiVIThread = true;
	lk.unlock();
	stopWiVIcv.notify_all();
	wiviThread.join();
	wiviThread = std::thread();
}

Lifetime Device::addNewCaptureCallback(NewCaptureCallback cb) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return {};
	}

	if(!supportsWiVI()) {
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::Error);
		return {};
	}

	std::lock_guard<std::mutex> lk(wiviMutex);
	if(!wiviThread.joinable()) {
		// Start the thread
		stopWiVIThread = false;
		wiviThread = std::thread([this]() { wiviThreadBody(); });
	}

	size_t idx = 0;
	for(; idx < newCaptureCallbacks.size(); idx++) {
		if(!newCaptureCallbacks[idx]) // Empty space (previously erased callback)
			break;
	}

	if(idx == newCaptureCallbacks.size()) // Create a new space
		newCaptureCallbacks.push_back(std::move(cb));
	else
		newCaptureCallbacks[idx] = std::move(cb);

	// Cleanup function to remove this capture callback
	return Lifetime([this, idx]() {
		// TODO: Hold a weak ptr to the `this` instead of relying on the user to keep `this` valid
		std::unique_lock<std::mutex> lk2(wiviMutex);
		newCaptureCallbacks[idx] = NewCaptureCallback();
		stopWiVIThreadIfNecessary(std::move(lk2));
	});
}

Lifetime Device::addSleepRequestedCallback(SleepRequestedCallback cb) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return {};
	}

	if(!supportsWiVI()) {
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::Error);
		return {};
	}

	std::lock_guard<std::mutex> lk(wiviMutex);
	if(!wiviThread.joinable()) {
		// Start the thread
		stopWiVIThread = false;
		wiviThread = std::thread([this]() { wiviThreadBody(); });
	}

	size_t idx = 0;
	for(; idx < sleepRequestedCallbacks.size(); idx++) {
		if(!sleepRequestedCallbacks[idx].first) // Empty space (previously erased callback)
			break;
	}

	if(idx == sleepRequestedCallbacks.size()) // Create a new space
		sleepRequestedCallbacks.emplace_back(std::move(cb), false);
	else
		sleepRequestedCallbacks[idx] = { std::move(cb), false };

	// Cleanup function to remove this sleep requested callback
	return Lifetime([this, idx]() {
		// TODO: Hold a weak ptr to the `this` instead of relying on the user to keep `this` valid
		std::unique_lock<std::mutex> lk2(wiviMutex);
		sleepRequestedCallbacks[idx].first = SleepRequestedCallback();
		stopWiVIThreadIfNecessary(std::move(lk2));
	});
}

std::optional<bool> Device::isSleepRequested() const {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}

	if(!supportsWiVI()) {
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::Error);
		return std::nullopt;
	}

	static std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::WiVICommandResponse);
	// Hold this lock so the WiVI stack doesn't issue a WiVICommand at the same time as us
	std::lock_guard<std::mutex> lk(wiviMutex);
	const auto generic = com->waitForMessageSync([this]() {
		// VSSAL sets bit0 to indicate that it's waiting to sleep, then
		// it waits for Wireless neoVI to acknowledge by clearing it.
		// If we set bit1 at the same time we clear bit0, remote wakeup
		// will be suppressed (assuming the device supported it in the
		// first place)
		return com->sendCommand(Command::WiVICommand, WiVI::CommandPacket::GetSignal::Encode(WiVI::SignalType::SleepRequest));
	}, filter);

	if(!generic || generic->type != Message::Type::WiVICommandResponse) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}

	const auto resp = std::static_pointer_cast<WiVI::ResponseMessage>(generic);
	if(!resp->success || !resp->value.has_value()) {
		report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);
		return std::nullopt;
	}

	return *resp->value;
}

bool Device::allowSleep(bool remoteWakeup) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	if(!supportsWiVI()) {
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::Error);
		return false;
	}

	static std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::WiVICommandResponse);
	// Hold this lock so the WiVI stack doesn't issue a WiVICommand at the same time as us
	std::lock_guard<std::mutex> lk(wiviMutex);
	const auto generic = com->waitForMessageSync([this, remoteWakeup]() {
		// VSSAL sets bit0 to indicate that it's waiting to sleep, then
		// it waits for Wireless neoVI to acknowledge by clearing it.
		// If we set bit1 at the same time we clear bit0, remote wakeup
		// will be suppressed (assuming the device supported it in the
		// first place)
		return com->sendCommand(Command::WiVICommand, WiVI::CommandPacket::SetSignal::Encode(
			WiVI::SignalType::SleepRequest, remoteWakeup ? 0 : 2
		));
	}, filter);

	if(!generic || generic->type != Message::Type::WiVICommandResponse) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return false;
	}

	const auto resp = std::static_pointer_cast<WiVI::ResponseMessage>(generic);
	if(!resp->success) {
		report(APIEvent::Type::ValueNotYetPresent, APIEvent::Severity::Error);
		return false;
	}

	return true;
}

void Device::scriptStatusThreadBody()
{
	std::unique_lock<std::mutex> lk(scriptStatusMutex);

	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	bool first = true;
	while(!stopScriptStatusThread)
	{
		if(first) // Skip the first wait
			first = false;
		else
			stopScriptStatusCv.wait_for(lk, std::chrono::seconds(10));

		const auto resp = getScriptStatus();

		if (!resp)
			continue;

		//If value changed/was inserted, notify callback
		if(updateScriptStatusValue(ScriptStatus::CoreMiniRunning, resp->isCoreminiRunning))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CoreMiniRunning, resp->isCoreminiRunning);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::SectorOverflow, resp->sectorOverflows))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::SectorOverflow, resp->sectorOverflows);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::RemainingSectors, resp->numRemainingSectorBuffers))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::RemainingSectors, resp->numRemainingSectorBuffers);
			lk.lock();
		}

		bool logging = false;
		if(updateScriptStatusValue(ScriptStatus::LastSector, resp->lastSector))
		{
			logging = true;
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::LastSector, resp->lastSector);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::Logging, logging))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::Logging, logging);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::ReadBinSize, resp->readBinSize))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::ReadBinSize, resp->readBinSize);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::MinSector, resp->minSector))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::MinSector, resp->minSector);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::MaxSector, resp->maxSector))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::MaxSector, resp->maxSector);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::CurrentSector, resp->currentSector))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CurrentSector, resp->currentSector);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::CoreMiniCreateTime, resp->coreminiCreateTime))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CoreMiniCreateTime, resp->coreminiCreateTime);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::FileChecksum, resp->fileChecksum))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::FileChecksum, resp->fileChecksum);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::CoreMiniVersion, resp->coreminiVersion))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CoreMiniVersion, resp->coreminiVersion);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::CoreMiniHeaderSize, resp->coreminiHeaderSize))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::CoreMiniHeaderSize, resp->coreminiHeaderSize);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::DiagnosticErrorCode, resp->diagnosticErrorCode))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::DiagnosticErrorCode, resp->diagnosticErrorCode);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::DiagnosticErrorCodeCount, resp->diagnosticErrorCodeCount))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::DiagnosticErrorCodeCount, resp->diagnosticErrorCodeCount);
			lk.lock();
		}

		if(updateScriptStatusValue(ScriptStatus::MaxCoreMiniSize, resp->maxCoreminiSizeKB))
		{
			lk.unlock();
			notifyScriptStatusCallback(ScriptStatus::MaxCoreMiniSize, resp->maxCoreminiSizeKB);
			lk.lock();
		}
	}
}

std::shared_ptr<ScriptStatusMessage> Device::getScriptStatus() const
{
	static std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::ScriptStatus);

	std::lock_guard<std::mutex> lg(diskLock);
	const auto generic = com->waitForMessageSync([this]() {
		return com->sendCommand(Command::ScriptStatus);
	}, filter, std::chrono::milliseconds(3000));

	if(!generic || generic->type != Message::Type::ScriptStatus) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return nullptr;
	}

	return std::static_pointer_cast<ScriptStatusMessage>(generic);
}

bool Device::updateScriptStatusValue(ScriptStatus key, uint64_t value)
{
	auto pair = scriptStatusValues.find(key);
	if(pair != scriptStatusValues.end())
	{
		if(pair->second != value)
		{
			//Value changed
			scriptStatusValues[key] = value;
			return true;
		}
		//Value didn't change
		return false;
	}

	//Value was inserted
	scriptStatusValues.insert(std::make_pair(key, value));
	return true;
}

void Device::notifyScriptStatusCallback(ScriptStatus key, uint64_t value)
{
	auto callbackList = scriptStatusCallbacks.find(key);
	if(callbackList != scriptStatusCallbacks.end())
	{
		for(const auto& callback : callbackList->second)
		{
			if(callback) {
				try {
					callback(value);
				} catch(...) {
					report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
				}
			}
		}
	}
}

Lifetime Device::addScriptStatusCallback(ScriptStatus key, ScriptStatusCallback cb)
{
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return {};
	}

	std::lock_guard<std::mutex> lk(scriptStatusMutex);
	if(!scriptStatusThread.joinable()) {
		// Start the thread
		stopScriptStatusThread = false;
		scriptStatusThread = std::thread([this]() { scriptStatusThreadBody(); });
	}

	size_t idx = 0;
	std::vector<ScriptStatusCallback> callbackList;
	auto callbackPair = scriptStatusCallbacks.find(key);
	if(callbackPair != scriptStatusCallbacks.end())
		callbackList = callbackPair->second;

	if(idx == callbackList.size())
		callbackList.push_back(std::move(cb));
	else callbackList[idx] = std::move(cb);

	scriptStatusCallbacks.insert_or_assign(key, callbackList);

	return Lifetime([this, key, idx](){
		std::unique_lock<std::mutex> lk2(scriptStatusMutex);
		auto callbackList = scriptStatusCallbacks.find(key);
		if(callbackList != scriptStatusCallbacks.end())
			callbackList->second[idx] = ScriptStatusCallback();
		stopScriptStatusThreadIfNecessary(std::move(lk2));
	});
}

void Device::stopScriptStatusThreadIfNecessary(std::unique_lock<std::mutex> lk)
{
	for(const auto& callbackList : scriptStatusCallbacks)
	{
		for(const auto& callback : callbackList.second)
		{
			if(callback)
				return;
		}
	}

	stopScriptStatusThread = true;
	lk.unlock();
	stopScriptStatusCv.notify_all();
	scriptStatusThread.join();
	scriptStatusThread = std::thread();
}

Lifetime Device::suppressDisconnects() {
	std::lock_guard<std::mutex> lk(heartbeatMutex);
	heartbeatSuppressedByUser++;
	return Lifetime([this] { std::lock_guard<std::mutex> lk2(heartbeatMutex); heartbeatSuppressedByUser--; });
}

void Device::addExtension(std::shared_ptr<DeviceExtension>&& extension) {
	std::lock_guard<std::mutex> lk(extensionsLock);
	extensions.push_back(extension);
}

void Device::forEachExtension(std::function<bool(const std::shared_ptr<DeviceExtension>&)> fn) {
	std::vector<std::shared_ptr<DeviceExtension>> extensionsCopy;

	{
		std::lock_guard<std::mutex> lk(extensionsLock);
		extensionsCopy = extensions;
	}

	for(const auto& ext : extensionsCopy) {
		if(!fn(ext))
			break;
	}
}

void Device::handleInternalMessage(std::shared_ptr<Message> message) {
	switch(message->type) {
		case Message::Type::ResetStatus:
			latestResetStatus = std::static_pointer_cast<ResetStatusMessage>(message);
			break;
		case Message::Type::RawMessage: {
			auto rawMessage = std::static_pointer_cast<RawMessage>(message);
			switch(rawMessage->network.getNetID()) {
				case Network::NetID::Device: {
					// Device is not guaranteed to be a CANMessage, it might be a RawMessage
					// if it couldn't be decoded to a CANMessage. We only care about the
					// CANMessage decoding right now.
					auto canmsg = std::dynamic_pointer_cast<CANMessage>(message);
					if(canmsg)
						handleNeoVIMessage(std::move(canmsg));
					break;
				}
				case Network::NetID::DeviceStatus:
					// Device Status format is unique per device, so the devices need to decode it themselves
					handleDeviceStatus(rawMessage);
					break;
				default:
					break; //std::cout << "HandleInternalMessage got a message from " << message->network << " and it was unhandled!" << std::endl;
			}
			break;
		}
		default: break;
	}
	forEachExtension([&](const std::shared_ptr<DeviceExtension>& ext) {
		ext->handleMessage(message);
		return true; // false breaks out early
	});
}

void Device::handleNeoVIMessage(std::shared_ptr<CANMessage> message) {
	switch(message->arbid) {
		case 0x103: { // Report Message (neoVI FIRE 2)
			if(message->data.size() < 34) {
				report(APIEvent::Type::PacketDecodingError, APIEvent::Severity::EventWarning);
				return;
			}

			uint16_t emisc[2];
			memcpy(emisc, message->data.data() + 24, sizeof(emisc));
			std::lock_guard<std::mutex> lk(ioMutex);
			miscAnalog[0] = (message->data[24] | (uint16_t(message->data[25]) << 8)) * 0.01015511; // In volts now
			miscAnalog[1] = (message->data[26] | (uint16_t(message->data[27]) << 8)) * 0.01015511;
			miscDigital[0] = message->data[28] & 0x01;
			miscDigital[1] = message->data[29] & 0x01;
			miscDigital[4] = message->data[30] & 0x01;
			miscDigital[5] = message->data[31] & 0x01;
		}
	}
}

bool Device::firmwareUpdateSupported() {
	bool ret = false;
	forEachExtension([&ret](const std::shared_ptr<DeviceExtension>& ext) {
		if(ext->providesFirmware()) {
			ret = true;
			return false;
		}
		return true; // false breaks out early
	});
	return ret;
}

APIEvent::Type Device::getCommunicationNotEstablishedError() {
	if(firmwareUpdateSupported()) {
		if(requiresVehiclePower())
			return APIEvent::Type::NoSerialNumberFW12V;
		else
			return APIEvent::Type::NoSerialNumberFW;
	} else {
		if(requiresVehiclePower())
			return APIEvent::Type::NoSerialNumber12V;
		else
			return APIEvent::Type::NoSerialNumber;
	}
}

void Device::updateLEDState() {
	std::vector<uint8_t> args {(uint8_t) ledState};
	com->sendCommand(Command::UpdateLEDState, args);
}

std::optional<EthPhyMessage> Device::sendEthPhyMsg(const EthPhyMessage& message, std::chrono::milliseconds timeout) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return std::nullopt;
	}
	if(!getEthPhyRegControlSupported()) {
		report(APIEvent::Type::EthPhyRegisterControlNotAvailable, APIEvent::Severity::Error);
		return std::nullopt;
	}
	if(!isOnline()) {
		report(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error);
		return std::nullopt;
	}

	std::vector<uint8_t> bytes;
	HardwareEthernetPhyRegisterPacket::EncodeFromMessage(message, bytes, report);
	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, bytes](){ return com->sendCommand(Command::PHYControlRegisters, bytes); },
		std::make_shared<MessageFilter>(Network::NetID::EthPHYControl), timeout);

	if(!response) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}
	auto retMsg = std::static_pointer_cast<EthPhyMessage>(response);
	if(!retMsg) {
		return std::nullopt;
	}
	return std::make_optional<EthPhyMessage>(*retMsg);
}

std::optional<bool> Device::SetCollectionUploaded(uint32_t collectionEntryByteAddress)
{
	if (!supportsWiVI())
	{
		report(APIEvent::Type::WiVINotSupported, APIEvent::Severity::EventWarning);
		return std::nullopt;
	}

	auto timeout = std::chrono::milliseconds(2500);
	std::vector<uint8_t> args(
		{(uint8_t)(collectionEntryByteAddress & 0xFF),
		 (uint8_t)((collectionEntryByteAddress >> 8) & 0xFF),
		 (uint8_t)((collectionEntryByteAddress >> 16) & 0xFF),
		 (uint8_t)((collectionEntryByteAddress >> 24) & 0xFF)});

	std::lock_guard<std::mutex> lg(diskLock);
	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, args](){ return com->sendCommand(ExtendedCommand::SetUploadedFlag, args); },
		std::make_shared<MessageFilter>(Message::Type::ExtendedResponse), timeout);
	if (!response)
	{
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}
	auto retMsg = std::static_pointer_cast<ExtendedResponseMessage>(response);
	if (!retMsg)
	{
		// TODO fix this error
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::make_optional<bool>(false);
	}
	bool success = retMsg->response == ExtendedResponse::OK;
	if (!success)
	{
		// TODO fix this error
		report(APIEvent::Type::Unknown, APIEvent::Severity::EventWarning);
	}
	// Valid device with a properly formed respose, return success
	return std::make_optional<bool>(success);
}
