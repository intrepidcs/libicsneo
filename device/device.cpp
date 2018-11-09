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

std::string Device::describe() const {
	std::stringstream ss;
	ss << getType() << ' ' << getSerial();
	return ss.str();
}

void Device::enableMessagePolling() {
	if(messagePollingCallbackID != 0) // We are already polling
		return;

	messagePollingCallbackID = com->addMessageCallback(MessageCallback([this](std::shared_ptr<Message> message) {
		pollingContainer.enqueue(message);
		enforcePollingMessageLimit();
	}));
}

bool Device::disableMessagePolling() {
	if(messagePollingCallbackID == 0)
		return true; // Not currently polling
		
	auto ret = com->removeMessageCallback(messagePollingCallbackID);
	getMessages(); // Flush any messages still in the container
	messagePollingCallbackID = 0;
	return ret;
}

std::vector<std::shared_ptr<Message>> Device::getMessages() {
	std::vector<std::shared_ptr<Message>> ret;
	getMessages(ret);
	return ret;
}

bool Device::getMessages(std::vector<std::shared_ptr<Message>>& container, size_t limit, std::chrono::milliseconds timeout) {
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
		err(APIError::PollingMessageOverflow);
	}
}

bool Device::open() {
	if(!com) {
		err(APIError::Unknown);
		return false;
	}

	if(!com->open())
		return false;

	auto serial = com->getSerialNumberSync();
	int i = 0;
	while(!serial) {
		serial = com->getSerialNumberSync();
		if(i++ > 5)
			break;
	}
	if(!serial) {
		err(APIError::NoSerialNumber);
		return false;
	}
	
	std::string currentSerial = getNeoDevice().serial;
	if(currentSerial != serial->deviceSerial) {
		err(APIError::IncorrectSerialNumber);
		return false;
	}
	
	bool settingsNecessary = !settings->disabled;
	if(settingsNecessary) {
		settings->refresh();
		if(!settings->ok())
			return false;
	}

	internalHandlerCallbackID = com->addMessageCallback(MessageCallback(MessageFilter(Network::Type::Internal), [this](std::shared_ptr<Message> message) {
		handleInternalMessage(message);
	}));

	return true;
}

bool Device::close() {
	if(!com) {
		err(APIError::Unknown);
		return false;
	}

	if(internalHandlerCallbackID)
		com->removeMessageCallback(internalHandlerCallbackID);

	return com->close();
}

bool Device::goOnline() {
	if(!com->sendCommand(Command::EnableNetworkCommunication, true))
		return false;

	ledState = LEDState::Online;
	updateLEDState();

	online = true;
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
	/* NetID::Device is a super old command type.
	 * It has a leading 0x00 byte, a byte for command, and a byte for an argument.
	 * In this case, command 0x06 is SetLEDState.
	 * This old command type is not really used anywhere else.
	 */
	auto msg = std::make_shared<Message>();
	msg->network = Network::NetID::Device;
	msg->data = {0x00, 0x06, uint8_t(ledState)};
	transmit(msg);
}