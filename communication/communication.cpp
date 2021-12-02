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
#include "icsneo/communication/message/versionmessage.h"

using namespace icsneo;

int Communication::messageCallbackIDCounter = 1;

Communication::~Communication() {
	if(redirectingRead)
		clearRedirectRead();
	close();
}

bool Communication::open() {
	if(isOpen()) {
		report(APIEvent::Type::DeviceCurrentlyOpen, APIEvent::Severity::Error);
		return false;
	}

	if(!driver->open())
		return false;
	spawnThreads();
	return true;
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
	joinThreads();

	if(!isOpen() && !isDisconnected()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	return driver->close();
}

bool Communication::isOpen() {
	return driver->isOpen();
}

bool Communication::isDisconnected() {
	return driver->isDisconnected();
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

bool Communication::sendCommand(ExtendedCommand cmd, std::vector<uint8_t> arguments) {
	const auto size = arguments.size();
	if (size > std::numeric_limits<uint16_t>::max())
		return false;

	arguments.insert(arguments.begin(), {
		uint8_t(uint16_t(cmd) & 0xff),
		uint8_t((uint16_t(cmd) >> 8) & 0xff),
		uint8_t(size & 0xff),
		uint8_t((size >> 8) & 0xff)
	});

	return sendCommand(Command::Extended, arguments);
}

bool Communication::redirectRead(std::function<void(std::vector<uint8_t>&&)> redirectTo) {
	if(redirectingRead)
		return false;
	redirectionFn = redirectTo;
	redirectingRead = true;
	return true;
}

void Communication::clearRedirectRead() {
	if(!redirectingRead)
		return;
	// The mutex is required to clear the redirection, but not to set it
	std::lock_guard<std::mutex> lk(redirectingReadMutex);
	redirectingRead = false;
	redirectionFn = std::function<void(std::vector<uint8_t>&&)>();
}

bool Communication::getSettingsSync(std::vector<uint8_t>& data, std::chrono::milliseconds timeout) {
	static const std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Network::NetID::ReadSettings);
	std::shared_ptr<Message> msg = waitForMessageSync([this]() {
		return sendCommand(Command::ReadSettings, { 0, 0, 0, 1 /* Get Global Settings */, 0, 1 /* Subversion 1 */ });
	}, filter, timeout);
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

	data = std::move(gsmsg->data);
	return true;
}

std::shared_ptr<SerialNumberMessage> Communication::getSerialNumberSync(std::chrono::milliseconds timeout) {
	static const std::shared_ptr<MessageFilter> filter = std::make_shared<Main51MessageFilter>(Command::RequestSerialNumber);
	std::shared_ptr<Message> msg = waitForMessageSync([this]() {
		return sendCommand(Command::RequestSerialNumber);
	}, filter, timeout);
	if(!msg) // Did not receive a message
	{
		std::cout << "didn't get a message" << std::endl;
		return std::shared_ptr<SerialNumberMessage>();
	}

	auto m51 = std::dynamic_pointer_cast<Main51Message>(msg);
	if(!m51) // Could not upcast for some reason
	{
		std::cout << "could not upcast" << std::endl;
		return std::shared_ptr<SerialNumberMessage>();
	}

	auto ret = std::dynamic_pointer_cast<SerialNumberMessage>(m51);
	std::cout << "returning " << ret.get() << std::endl;
	return ret;
}

optional< std::vector< optional<DeviceAppVersion> > > Communication::getVersionsSync(std::chrono::milliseconds timeout) {
	static const std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::DeviceVersion);
	std::vector< optional<DeviceAppVersion> > ret;

	std::shared_ptr<Message> msg = waitForMessageSync([this]() {
		return sendCommand(Command::GetMainVersion);
	}, filter, timeout);
	if(!msg) // Did not receive a message
		return nullopt;

	auto ver = std::dynamic_pointer_cast<VersionMessage>(msg);
	if(!ver) // Could not upcast for some reason
		return nullopt;

	if(ver->ForChip != VersionMessage::MainChip || ver->Versions.size() != 1)
		return nullopt;

	ret.push_back(ver->Versions.front());

	msg = waitForMessageSync([this]() {
		return sendCommand(Command::GetSecondaryVersions);
	}, filter, timeout);
	if(msg) { // This one is allowed to fail
		ver = std::dynamic_pointer_cast<VersionMessage>(msg);
		if(ver && ver->ForChip != VersionMessage::MainChip)
			ret.insert(ret.end(), ver->Versions.begin(), ver->Versions.end());
	}

	return ret;
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

std::shared_ptr<Message> Communication::waitForMessageSync(std::function<bool(void)> onceWaitingDo,
	const std::shared_ptr<MessageFilter>& f, std::chrono::milliseconds timeout) {
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
			handleInput(*packetizer, readBytes);
		}
	}
}

void Communication::handleInput(Packetizer& p, std::vector<uint8_t>& readBytes) {
	if(redirectingRead) {
		// redirectingRead is an atomic so it can be set without acquiring a mutex
		// However, we do not clear it without the mutex. The idea is that if another
		// thread calls clearRedirectRead(), it will block until the redirectionFn
		// finishes, and after that the redirectionFn will not be called again.
		std::unique_lock<std::mutex> lk(redirectingReadMutex);
		// So after we acquire the mutex, we need to check the atomic again, and
		// if it has become cleared, we *can not* run the redirectionFn.
		if(redirectingRead) {
			redirectionFn(std::move(readBytes));
		} else {
			// The redirectionFn got cleared while we were acquiring the lock
			lk.unlock(); // We don't need the lock anymore
			handleInput(p, readBytes); // and we might as well process this input ourselves
		}
	} else {
		if(p.input(readBytes)) {
			for(const auto& packet : p.output()) {
				std::shared_ptr<Message> msg;
				if(!decoder->decode(msg, packet))
					continue;

				dispatchMessage(msg);
			}
		}
	}
}
