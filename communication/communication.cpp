#include "icsneo/communication/communication.h"
#include <chrono>
#include <iostream>
#include <queue>
#include <iomanip>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include "icsneo/communication/command.h"
#include "icsneo/communication/decoder.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/message/serialnumbermessage.h"
#include "icsneo/communication/message/filter/main51messagefilter.h"
#include "icsneo/communication/message/readsettingsmessage.h"

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
	closing = true;
	if(readTaskThread.joinable())
		readTaskThread.join();
	closing = false;
}

bool Communication::close() {
	if(!isOpen)
		return false;

	isOpen = false;
	joinThreads();

	return impl->close();
}

bool Communication::sendPacket(std::vector<uint8_t>& bytes) {
	// This is here so that other communication types (like multichannel) can override it
	return rawWrite(bytes);
}

bool Communication::sendCommand(Command cmd, std::vector<uint8_t> arguments) {
	std::vector<uint8_t> packet;
	if(!encoder->encode(packet, cmd, arguments))
		return false;

	return sendPacket(packet);
}

bool Communication::getSettingsSync(std::vector<uint8_t>& data, std::chrono::milliseconds timeout) {
	sendCommand(Command::ReadSettings, { 0, 0, 0, 1 /* Get Global Settings */, 0, 1 /* Subversion 1 */ });
	std::shared_ptr<Message> msg = waitForMessageSync(MessageFilter(Network::NetID::ReadSettings), timeout);
	if(!msg)
		return false;

	std::shared_ptr<ReadSettingsMessage> gsmsg = std::dynamic_pointer_cast<ReadSettingsMessage>(msg);
	if(!gsmsg)
		return false;

	if(gsmsg->response != ReadSettingsMessage::Response::OK)
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
					std::shared_ptr<Message> msg;
					if(!decoder->decode(msg, packet)) {
						err(APIError::Unknown); // TODO Use specific error
						continue;
					}
					
					for(auto& cb : messageCallbacks) {
						if(!closing) { // We might have closed while reading or processing
							cb.second.callIfMatch(msg);
						}
					}
				}
			}
		}
	}
}
