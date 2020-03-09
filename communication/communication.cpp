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
	if(isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}
	
	spawnThreads();
	return driver->open();
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
	if(!isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	joinThreads();

	return driver->close();
}

bool Communication::isOpen() {
	return driver->isOpen();
}

bool Communication::sendPacket(std::vector<uint8_t>& bytes) {
	// This is here so that other communication types (like multichannel) can override it
	return rawWrite(bytes);
}

bool Communication::sendCommand(Command cmd, std::vector<uint8_t> arguments) {
	std::vector<uint8_t> packet;
	if(!encoder->encode(*packetizer, packet, cmd, arguments))
		return false;

	return sendPacket(packet);
}

bool Communication::getSettingsSync(std::vector<uint8_t>& data, std::chrono::milliseconds timeout) {
	std::shared_ptr<Message> msg = waitForMessageSync([this]() {
		return sendCommand(Command::ReadSettings, { 0, 0, 0, 1 /* Get Global Settings */, 0, 1 /* Subversion 1 */ });
	}, MessageFilter(Network::NetID::ReadSettings), timeout);
	if(!msg)
		return false;

	std::shared_ptr<ReadSettingsMessage> gsmsg = std::dynamic_pointer_cast<ReadSettingsMessage>(msg);
	if(!gsmsg) {
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
		return false;
	}

	if(gsmsg->response != ReadSettingsMessage::Response::OK) {
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
		return false;
	}

	data = std::move(msg->data);
	return true;
}

std::shared_ptr<SerialNumberMessage> Communication::getSerialNumberSync(std::chrono::milliseconds timeout) {
	sendCommand(Command::RequestSerialNumber);
	std::shared_ptr<Message> msg = waitForMessageSync([this]() {
		return sendCommand(Command::RequestSerialNumber);
	}, Main51MessageFilter(Command::RequestSerialNumber), timeout);
	if(!msg) // Did not receive a message
		return std::shared_ptr<SerialNumberMessage>();

	auto m51 = std::dynamic_pointer_cast<Main51Message>(msg);
	if(!m51) // Could not upcast for some reason
		return std::shared_ptr<SerialNumberMessage>();
	
	return std::dynamic_pointer_cast<SerialNumberMessage>(m51);
}

int Communication::addMessageCallback(const MessageCallback& cb) {
	std::lock_guard<std::mutex> lk(messageCallbacksLock);
	messageCallbacks.insert(std::make_pair(messageCallbackIDCounter, cb));
	return messageCallbackIDCounter++;
}

bool Communication::removeMessageCallback(int id) {
	std::lock_guard<std::mutex> lk(messageCallbacksLock);
	try {
		messageCallbacks.erase(id);
		return true;
	} catch(...) {
		report(APIEvent::Type::Unknown, APIEvent::Severity::Error);
		return false;
	}
}

std::shared_ptr<Message> Communication::waitForMessageSync(std::function<bool(void)> onceWaitingDo, MessageFilter f, std::chrono::milliseconds timeout) {
	std::mutex m;
	std::condition_variable cv;
	std::shared_ptr<Message> returnedMessage;

	std::unique_lock<std::mutex> lk(m); // Don't let the callback fire until we're waiting for it
	int cb = addMessageCallback(MessageCallback([&m, &returnedMessage, &cv](std::shared_ptr<Message> message) {
		{
			std::lock_guard<std::mutex> lk(m);
			returnedMessage = message;
		}
		cv.notify_all();
	}, f));

	// We have now added the callback, do whatever the caller wanted to do
	bool fail = !onceWaitingDo();
	if(!fail)
		cv.wait_for(lk, timeout, [&returnedMessage] { return !!returnedMessage; }); // `!!shared_ptr` checks if the ptr has a value
	lk.unlock(); // Ensure callbacks can complete even if we didn't wait for them

	// We don't actually check that we got a message, because either way we want to remove the callback (since it should only happen once)
	removeMessageCallback(cb);
	// We are now guaranteed that no more callbacks will happen

	if(fail) // The caller's function failed, so don't return a message
		returnedMessage.reset();

	// Then we either will return the message we got or we will return the empty shared_ptr, caller responsible for checking
	return returnedMessage;
}

void Communication::dispatchMessage(const std::shared_ptr<Message>& msg) {
	std::lock_guard<std::mutex> lk(messageCallbacksLock);

	// We want callbacks to be able to access errors
	const bool downgrade = EventManager::GetInstance().isDowngradingErrorsOnCurrentThread();
	if(downgrade)
		EventManager::GetInstance().cancelErrorDowngradingOnCurrentThread();
	for(auto& cb : messageCallbacks) {
		if(!closing) { // We might have closed while reading or processing
			cb.second.callIfMatch(msg);
		}
	}
	if(downgrade)
		EventManager::GetInstance().downgradeErrorsOnCurrentThread();
}

void Communication::readTask() {
	std::vector<uint8_t> readBytes;

	EventManager::GetInstance().downgradeErrorsOnCurrentThread();
	
	while(!closing) {
		readBytes.clear();
		if(driver->readWait(readBytes)) {
			if(packetizer->input(readBytes)) {
				for(const auto& packet : packetizer->output()) {
					std::shared_ptr<Message> msg;
					if(!decoder->decode(msg, packet))
						continue;

					dispatchMessage(msg);
				}
			}
		}
	}
}
