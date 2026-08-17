#include "icsneo/api/heartbeat.h"
#include "icsneo/api/lifetime.h"
#include "icsneo/device/device.h"

using namespace icsneo;

Heartbeat::Heartbeat(Device& device) :
	device(device), mode(device.isOnline() ? Mode::Passive : Mode::Active), thread(&Heartbeat::run, this) {
}

Heartbeat::~Heartbeat() {
	{
		std::lock_guard lk(mutex);
		stop = true;
	}
	cv.notify_one();
	thread.join();
}

void Heartbeat::run() {
	EventManager::GetInstance().downgradeErrorsOnCurrentThread();

	auto filter = std::make_shared<MessageFilter>();
	filter->includeInternalInAny = true;
	bool status = false;
	auto cbHandle = device.com->addMessageCallback(std::make_shared<MessageCallback>(filter, [&](std::shared_ptr<Message> msg) {
		// TODO: remove DeviceStatus after 2027-08-17, as ResetStatus should be widely supported
		const bool isHeartbeat =
			(msg->type == Message::Type::ResetStatus) ||
			(msg->type == Message::Type::RawMessage && std::static_pointer_cast<RawMessage>(msg)->network == Network::NetID::DeviceStatus);
		if(!isHeartbeat)
			return;
		{
			std::lock_guard lk(mutex);
			status = true;
		}
		cv.notify_one();
	}));
	Lifetime cbLifetime([&] {
		device.com->removeMessageCallback(cbHandle);
	});
	while(true) {
		std::unique_lock lk(mutex);
		if(mode == Mode::Active) {
			if(cv.wait_for(lk, std::chrono::milliseconds(100), [&] { return stop; })) {
				break;
			}
			device.com->sendCommand(Command::RequestStatusUpdate);
		}
		if(!cv.wait_for(lk, std::chrono::seconds(2), [&] { return status || stop; })) {
			// we disconnect instead of close because it indicates to the user that a cleanup is still required (our thread join)
			device.report(APIEvent::Type::DeviceDisconnected, APIEvent::Severity::Error);
			device.com->driver->setIsDisconnected(true);
			break;
		}
		if(stop)
			break;
		status = false;
	}
}
