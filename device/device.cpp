#include "icsneo/device/device.h"
#include "icsneo/communication/message/callback/messagecallback.h"
#include "icsneo/api/eventmanager.h"
#include "icsneo/communication/command.h"
#include "icsneo/device/extensions/deviceextension.h"
#include "icsneo/platform/optional.h"
#include "icsneo/disk/fat.h"
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
	messagePollingCallbackID = com->addMessageCallback(MessageCallback([this](std::shared_ptr<Message> message) {
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
	internalHandlerCallbackID = com->addMessageCallback(MessageCallback(filter, [this](std::shared_ptr<Message> message) {
		handleInternalMessage(message);
	}));

	heartbeatThread = std::thread([this]() {
		EventManager::GetInstance().downgradeErrorsOnCurrentThread();

		MessageFilter filter;
		filter.includeInternalInAny = true;
		std::atomic<bool> receivedMessage{false};
		auto messageReceivedCallbackID = com->addMessageCallback(MessageCallback(filter, [&receivedMessage](std::shared_ptr<Message> message) {
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

optional<uint64_t> Device::readLogicalDisk(uint64_t pos, uint8_t* into, uint64_t amount, std::chrono::milliseconds timeout) {
	if(!into || timeout <= std::chrono::milliseconds(0)) {
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return nullopt;
	}

	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return nullopt;
	}

	return diskReadDriver->readLogicalDisk(*com, report, pos, into, amount, timeout);
}

optional<uint64_t> Device::writeLogicalDisk(uint64_t pos, const uint8_t* from, uint64_t amount, std::chrono::milliseconds timeout) {
	if(!from || timeout <= std::chrono::milliseconds(0)) {
		report(APIEvent::Type::RequiredParameterNull, APIEvent::Severity::Error);
		return nullopt;
	}

	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return nullopt;
	}

	return diskWriteDriver->writeLogicalDisk(*com, report, *diskReadDriver, pos, from, amount, timeout);
}

optional<bool> Device::isLogicalDiskConnected() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return nullopt;
	}

	const auto info = com->getLogicalDiskInfoSync();
	if (!info) {
		report(APIEvent::Type::Timeout, APIEvent::Severity::Error);
		return nullopt;
	}

	return info->connected;
}

optional<uint64_t> Device::getLogicalDiskSize() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return nullopt;
	}

	const auto info = com->getLogicalDiskInfoSync();
	if (!info) {
		report(APIEvent::Type::Timeout, APIEvent::Severity::Error);
		return nullopt;
	}

	return info->getReportedSize();
}

optional<uint64_t> Device::getVSAOffsetInLogicalDisk() {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return nullopt;
	}

	if (diskReadDriver->getAccess() == Disk::Access::VSA || diskReadDriver->getAccess() == Disk::Access::None)
		return 0ull;
	
	return Disk::FindVSAInFAT([this](uint64_t pos, uint8_t *into, uint64_t amount) {
		return readLogicalDisk(pos, into, amount);
	});
}

optional<bool> Device::getDigitalIO(IO type, size_t number /* = 1 */) {
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
	return nullopt;
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

optional<double> Device::getAnalogIO(IO type, size_t number /* = 1 */) {
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
	return nullopt;
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

optional<EthPhyMessage> Device::sendEthPhyMsg(const EthPhyMessage& message, std::chrono::milliseconds timeout) {
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return nullopt;
	}
	if(!getEthPhyRegControlSupported()) {
		report(APIEvent::Type::EthPhyRegisterControlNotAvailable, APIEvent::Severity::Error);
		return nullopt;
	}
	if(!isOnline()) {
		report(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error);
		return nullopt;
	}

	std::vector<uint8_t> bytes;
	HardwareEthernetPhyRegisterPacket::EncodeFromMessage(message, bytes, report);
	std::shared_ptr<Message> response = com->waitForMessageSync(
		[this, bytes](){ return com->sendCommand(Command::PHYControlRegisters, bytes); },
		std::make_shared<MessageFilter>(Network::NetID::EthPHYControl), timeout);

	if(!response) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return nullopt;
	}
	auto retMsg = std::static_pointer_cast<EthPhyMessage>(response);
	if(!retMsg) {
		return nullopt;
	}
	return make_optional<EthPhyMessage>(*retMsg);
}