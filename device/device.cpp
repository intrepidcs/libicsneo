#include "icsneo/device/device.h"
#include "icsneo/communication/message/callback/messagecallback.h"
#include "icsneo/communication/command.h"
#include <string.h>
#include <iostream>
#include <sstream>

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

uint16_t Device::getTimestampResolution() const {
	return com->decoder->timestampResolution;
}

std::string Device::describe() const {
	std::stringstream ss;
	ss << getType() << ' ' << getSerial();
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
	// not open
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	// not online
	if(!isOnline()) {
		report(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error);
		return false;
	}

	// not currently polling, throw error
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

bool Device::open() {
	if(!com) {
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
		return false;
	}

	if(!com->open()) {
		return false;
	}

	auto serial = com->getSerialNumberSync();
	int i = 0;
	while(!serial) {
		serial = com->getSerialNumberSync();
		if(i++ > 5)
			break;
	}
	if(!serial) {
		report(APIEvent::Type::NoSerialNumber, APIEvent::Severity::Error); // Communication could not be established with the device. Perhaps it is not powered with 12 volts?
		com->close();
		return false;
	}
	
	std::string currentSerial = getNeoDevice().serial;
	if(currentSerial != serial->deviceSerial) {
		report(APIEvent::Type::IncorrectSerialNumber, APIEvent::Severity::Error);
		com->close();
		return false;
	}
	
	if(!settings->disabled)
		settings->refresh();

	internalHandlerCallbackID = com->addMessageCallback(MessageCallback(MessageFilter(Network::Type::Internal), [this](std::shared_ptr<Message> message) {
		handleInternalMessage(message);
	}));

	return true;
}

bool Device::close() {
	if(!com) {
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
		return false;
	}

	if(internalHandlerCallbackID)
		com->removeMessageCallback(internalHandlerCallbackID);

	internalHandlerCallbackID = 0;

	if(isOnline())
		goOffline();

	return com->close();
}

bool Device::goOnline() {
	if(!com->sendCommand(Command::EnableNetworkCommunication, true))
		return false;

	ledState = LEDState::Online;

	online = true;
	updateLEDState();

	return true;
}

bool Device::goOffline() {
	if(!com->sendCommand(Command::EnableNetworkCommunication, false))
		return false;

	ledState = (latestResetStatus && latestResetStatus->cmRunning) ? LEDState::CoreMiniRunning : LEDState::Offline;
	
	updateLEDState();
	online = false;

	return true;
}

bool Device::transmit(std::shared_ptr<Message> message) {
	// not open
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	// not online
	if(!isOnline()) {
		report(APIEvent::Type::DeviceCurrentlyOffline, APIEvent::Severity::Error);
		return false;
	}

	if(!isSupportedTXNetwork(message->network)) {
		report(APIEvent::Type::UnsupportedTXNetwork, APIEvent::Severity::Error);
		return false;
	}

	std::vector<uint8_t> packet;
	if(!com->encoder->encode(packet, message))
		return false;
	
	return com->sendPacket(packet);
}

bool Device::transmit(std::vector<std::shared_ptr<Message>> messages) {
	for(auto& message : messages) {
		if(!transmit(message))
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

void Device::handleInternalMessage(std::shared_ptr<Message> message) {
	switch(message->network.getNetID()) {
		case Network::NetID::Reset_Status:
			latestResetStatus = std::dynamic_pointer_cast<ResetStatusMessage>(message);
			break;
		default:
			break; //std::cout << "HandleInternalMessage got a message from " << message->network << " and it was unhandled!" << std::endl;
	}
}

void Device::updateLEDState() {
	std::vector<uint8_t> args {(uint8_t) ledState};
	com->sendCommand(Command::UpdateLEDState, args);
}