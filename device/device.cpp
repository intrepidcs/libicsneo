#include "include/device.h"
#include "communication/include/messagecallback.h"
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

bool Device::getMessages(std::vector<std::shared_ptr<Message>>& container, size_t limit) {
	// A limit of zero indicates no limit
	auto oglimit = limit;
	if(limit == 0)
		limit = (size_t)-1;

	if(limit > (pollingContainer.size_approx() + 4))
		limit = (pollingContainer.size_approx() + 4);

	if(container.capacity() < limit)
		container.resize(limit);

	size_t actuallyRead = pollingContainer.try_dequeue_bulk(container.data(), limit);

	container.resize(actuallyRead);

	return actuallyRead <= oglimit;
}

void Device::enforcePollingMessageLimit() {
	while(pollingContainer.size_approx() > pollingMessageLimit) {
		std::shared_ptr<Message> throwAway;
		pollingContainer.try_dequeue(throwAway);
		// TODO Flag an error for the user!
	}
}

bool Device::open() {
	if(!com)
		return false;

	return com->open();
}

bool Device::close() {
	if(!com)
		return false;

	return com->close();
}

bool Device::goOnline() {
	if(!com->sendCommand(Communication::Command::EnableNetworkCommunication, true))
		return false;

	if(!com->sendCommand(Communication::Command::RequestSerialNumber))
		return false;

	com->addMessageCallback(CANMessageCallback([](std::shared_ptr<Message> message) {
		std::shared_ptr<CANMessage> canMessage = std::static_pointer_cast<CANMessage>(message);
		std::cout << "CAN 0x" << std::hex << canMessage->arbid << std::dec << " [" << canMessage->data.size() << "] " << std::hex;
		for(const auto& b : canMessage->data)
			std::cout << (int)b << ' ';
		std::cout << std::dec << std::endl;
	}));

	return online = true;
}

bool Device::goOffline() {
	return com->sendCommand(Communication::Command::EnableNetworkCommunication, false);
}