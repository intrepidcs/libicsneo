#include "communication/include/communication.h"
#include <chrono>
#include <iostream>
#include <queue>
#include <iomanip>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include "communication/include/command.h"
#include "communication/include/decoder.h"
#include "communication/include/packetizer.h"
#include "communication/message/include/serialnumbermessage.h"
#include "communication/message/filter/include/main51messagefilter.h"

using namespace icsneo;

int Communication::messageCallbackIDCounter = 1;

bool Communication::open() {
	if(isOpen)
		return true;

	spawnThreads();
	isOpen = true;
	return impl->open();
}

void Communication::spawnThreads() {
	readTaskThread = std::thread(&Communication::readTask, this);
}

void Communication::joinThreads() {
	if(readTaskThread.joinable())
		readTaskThread.join();
}

bool Communication::close() {
	if(!isOpen)
		return false;

	isOpen = false;
	closing = true;
	joinThreads();

	return impl->close();
}

bool Communication::sendPacket(std::vector<uint8_t>& bytes) {
	// std::cout << "\nWriting " << bytes.size() << " bytes\n" << std::hex;
	// for(size_t i = 0; i < bytes.size(); i++)
	// 	std::cout << std::setw(2) << std::setfill('0') << (int)bytes[i] << (i % 16 == 15 ? '\n' : ' ');
	// std::cout << '\n' << std::endl << std::dec;
	return rawWrite(bytes);
}

bool Communication::sendCommand(Command cmd, std::vector<uint8_t> arguments) {
	auto msg = std::make_shared<Message>();
	msg->network = Network::NetID::Main51;
	msg->data = std::move(arguments);
	msg->data.insert(msg->data.begin(), (uint8_t)cmd);
	auto packet = encoder->encode(msg);
	return sendPacket(packet);
}

bool Communication::getSettingsSync(std::vector<uint8_t>& data, std::chrono::milliseconds timeout) {
	sendCommand(Command::GetSettings);
	std::shared_ptr<Message> msg = waitForMessageSync(MessageFilter(Network::NetID::RED_READ_BAUD_SETTINGS), timeout);
	if(!msg)
		return false;
	
	data = std::move(msg->data);
	return true;
}

std::shared_ptr<SerialNumberMessage> Communication::getSerialNumberSync(std::chrono::milliseconds timeout) {
	sendCommand(Command::RequestSerialNumber);
	std::shared_ptr<Message> msg = waitForMessageSync(std::make_shared<Main51MessageFilter>(Command::RequestSerialNumber), timeout);
	if(!msg) // Did not receive a message
		return std::shared_ptr<SerialNumberMessage>();

	auto m51 = std::dynamic_pointer_cast<Main51Message>(msg);
	if(!m51) // Could not upcast for some reason
		return std::shared_ptr<SerialNumberMessage>();
	
	return std::dynamic_pointer_cast<SerialNumberMessage>(m51);
}

int Communication::addMessageCallback(const MessageCallback& cb) {
	messageCallbacks.insert(std::make_pair(messageCallbackIDCounter, cb));
	return messageCallbackIDCounter++;
}

bool Communication::removeMessageCallback(int id) {
	try {
		messageCallbacks.erase(id);
		return true;
	} catch(...) {
		return false;
	}
}

std::shared_ptr<Message> Communication::waitForMessageSync(std::shared_ptr<MessageFilter> f, std::chrono::milliseconds timeout) {
	std::mutex m;
	std::condition_variable cv;
	std::shared_ptr<Message> returnedMessage;
	int cb = addMessageCallback(MessageCallback([&m, &returnedMessage, &cv](std::shared_ptr<Message> message) {
		{
			std::lock_guard<std::mutex> lk(m);
			returnedMessage = message;
		}
		cv.notify_one();
	}, f));

	// We have now added the callback, wait for it to return from the other thread
	std::unique_lock<std::mutex> lk(m);
	cv.wait_for(lk, timeout, [&returnedMessage]{ return !!returnedMessage; }); // `!!shared_ptr` checks if the ptr has a value

	// We don't actually check that we got a message, because either way we want to remove the callback (since it should only happen once)
	removeMessageCallback(cb);

	// Then we either will return the message we got or we will return the empty shared_ptr, caller responsible for checking
	return returnedMessage;
}

void Communication::readTask() {
	std::vector<uint8_t> readBytes;

	while(!closing) {
		readBytes.clear();
		if(impl->readWait(readBytes)) {
			if(packetizer->input(readBytes)) {
				for(auto& packet : packetizer->output()) {
					auto msg = decoder->decodePacket(packet);
					//std::cout << "Got packet for " << msg->network << ", calling " << messageCallbacks.size() << " callbacks" << std::endl;
					for(auto& cb : messageCallbacks) { // We might have closed while reading or processing
						if(!closing) {
							cb.second.callIfMatch(msg);
						}
					}
				}
			}
		}
	}
}
