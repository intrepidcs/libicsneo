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
#include "icsneo/communication/message/componentversionsmessage.h"
#include "icsneo/communication/message/filter/extendedresponsefilter.h"
#include "icsneo/communication/message/clientidmessage.h"
#include "icsneo/communication/icspb.h"

#include <commands/generic/v1/client_id.pb.h>
#include <commands/network/v1/mutex.pb.h>

using namespace icsneo;

int Communication::messageCallbackIDCounter = 1;

Communication::~Communication() {
	if(isOpen())
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
	closing = false;
	readTaskThread = std::thread(&Communication::readTask, this);
}

void Communication::joinThreads() {
	if(pauseReadTask) {
		resumeReads();
	}

	closing = true;
	if(readTaskThread.joinable())
		readTaskThread.join();
}

bool Communication::close() {
	if(!isOpen() && !isDisconnected()) {
		report(APIEvent::Type::DeviceCurrentlyClosed, APIEvent::Severity::Error);
		return false;
	}

	joinThreads();

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

	if(gsmsg->response == ReadSettingsMessage::Response::OKDefaultsUsed) {
		report(APIEvent::Type::SettingsDefaultsUsed, APIEvent::Severity::EventInfo);
	} else if(gsmsg->response != ReadSettingsMessage::Response::OK) {
		report(APIEvent::Type::SettingsReadError, APIEvent::Severity::Error);
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
		return std::shared_ptr<SerialNumberMessage>();

	auto m51 = std::dynamic_pointer_cast<Main51Message>(msg);
	if(!m51) // Could not upcast for some reason
		return std::shared_ptr<SerialNumberMessage>();
	return std::dynamic_pointer_cast<SerialNumberMessage>(m51);
}

std::optional< std::vector< std::optional<DeviceAppVersion> > > Communication::getVersionsSync(std::chrono::milliseconds timeout) {
	static const std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::DeviceVersion);
	std::vector< std::optional<DeviceAppVersion> > ret;

	std::shared_ptr<Message> msg = waitForMessageSync([this]() {
		return sendCommand(Command::GetMainVersion);
	}, filter, timeout);
	if(!msg) // Did not receive a message
		return std::nullopt;

	auto ver = std::dynamic_pointer_cast<VersionMessage>(msg);
	if(!ver) // Could not upcast for some reason
		return std::nullopt;

	if(ver->ForChip != VersionMessage::MainChip || ver->Versions.size() != 1)
		return std::nullopt;

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

std::shared_ptr<LogicalDiskInfoMessage> Communication::getLogicalDiskInfoSync(std::chrono::milliseconds timeout) {
	static const std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::LogicalDiskInfo);

	std::shared_ptr<Message> msg = waitForMessageSync([this]() {
		return sendCommand(Command::GetLogicalDiskInfo);
	}, filter, timeout);
	if(!msg) // Did not receive a message
		return {};

	return std::dynamic_pointer_cast<LogicalDiskInfoMessage>(msg);
}

int Communication::addMessageCallback(const std::shared_ptr<MessageCallback>& cb) {
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
	std::mutex cvMutex;
	std::condition_variable cv;
	std::shared_ptr<Message> returnedMessage;

	std::unique_lock<std::mutex> fnLk(syncMessageMutex); // Only allow for one sync message at a time
	std::unique_lock<std::mutex> cvLk(cvMutex); // Don't let the callback fire until we're waiting for it
	int cb = addMessageCallback(std::make_shared<MessageCallback>([&cvMutex, &returnedMessage, &cv](std::shared_ptr<Message> message) {
		{
			std::lock_guard<std::mutex> lk(cvMutex);
			returnedMessage = message;
		}
		cv.notify_all();
	}, f));

	// We have now added the callback, do whatever the caller wanted to do
	bool fail = !onceWaitingDo();
	if(!fail)
		cv.wait_for(cvLk, timeout, [&returnedMessage] { return !!returnedMessage; }); // `!!shared_ptr` checks if the ptr has a value
	cvLk.unlock(); // Ensure callbacks can complete even if we didn't wait for them

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
			cb.second->callIfMatch(msg);
		}
	}
	if(downgrade)
		EventManager::GetInstance().downgradeErrorsOnCurrentThread();
}

void Communication::pauseReads() {
	std::unique_lock<std::mutex> lk(pauseReadTaskMutex);
	pauseReadTask = true;
}

void Communication::resumeReads() {
	std::unique_lock<std::mutex> lk(pauseReadTaskMutex);
	if(!pauseReadTask) {
		return;
	}
	pauseReadTask = false; 
	lk.unlock();

	pauseReadTaskCv.notify_one();
}

bool Communication::readsArePaused() {
	std::unique_lock<std::mutex> lk(pauseReadTaskMutex);
	return pauseReadTask;
}

void Communication::readTask() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	while(!closing) {
		if(pauseReadTask) {
			std::unique_lock<std::mutex> lk(pauseReadTaskMutex);
			pauseReadTaskCv.wait(lk, [this]() { return !pauseReadTask; });
		}
		if(driver->waitForRx(readTaskWakeLimit, readTaskWakeTimeout)) {
			if(pauseReadTask) {
				/**
				 * Reads could have paused while the driver was not available
				*/
				continue;
			}
			handleInput(*packetizer);
		}
	}
}

void Communication::handleInput(Packetizer& p) {
	if(p.input(driver->getReadBuffer())) {
		for(const auto& packet : p.output()) {
			std::shared_ptr<Message> msg;
			if(!decoder->decode(msg, packet))
				continue;

			dispatchMessage(msg);
		}
	}
}

std::optional< std::vector<ComponentVersion> > Communication::getComponentVersionsSync(std::chrono::milliseconds timeout) {
	static const std::shared_ptr<MessageFilter> filter = std::make_shared<MessageFilter>(Message::Type::ComponentVersions);
	std::shared_ptr<Message> msg = waitForMessageSync([this]() {
		return sendCommand(ExtendedCommand::GetComponentVersions, {});
	}, filter, timeout);
	if(!msg) // Did not receive a message
		return std::nullopt;

	auto ver = std::dynamic_pointer_cast<ComponentVersionsMessage>(msg);
	if(!ver) // Could not upcast for some reason
		return std::nullopt;

	return std::make_optional< std::vector<ComponentVersion> >(std::move(ver->versions));
}

std::optional<uint32_t> Communication::getClientIDSync() {	
	constexpr auto timeout = std::chrono::milliseconds(250);
	commands::generic::v1::ClientId msg;
	msg.Clear();

	std::vector<uint8_t> payload = protoapi::getPayload(protoapi::Command::GET, msg);

	std::shared_ptr<Message> response = waitForMessageSync(
		[this, payload](){
			return sendCommand(ExtendedCommand::ProtobufAPI, payload);
		},
		std::make_shared<MessageFilter>(Message::Type::ClientId),
		timeout
	);

	if(!response) {
		report(APIEvent::Type::NoDeviceResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}

	auto clientIdMessage = std::dynamic_pointer_cast<ClientIdMessage>(response);
	if(!clientIdMessage) {
		report(APIEvent::Type::UnexpectedResponse, APIEvent::Severity::Error);
		return std::nullopt;
	}

	return clientIdMessage->clientId;
}
