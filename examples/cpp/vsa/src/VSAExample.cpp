#include <iostream>
#include <random>

#include "icsneo/icsneocpp.h"

void onEvent(std::shared_ptr<icsneo::APIEvent> event) {
	std::cout << event->describe() << std::endl;
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		std::cout << "Usage: libicsneocpp-vsa-example <device serial>" << std::endl;
		return -1;
	}

	std::string deviceSerial = argv[1];

	// register an event callback so we can see all logged events
	icsneo::EventManager::GetInstance().addEventCallback(icsneo::EventCallback(onEvent));

	auto devices = icsneo::FindAllDevices();
	if(devices.empty()) {
		std::cout << "error: no devices found" << std::endl;
		return -1;
	}

	std::shared_ptr<icsneo::Device> device;
	for(auto&& d : devices) {
		if(deviceSerial == d->getSerial()) {
			device = d;
		}
	}
	if(!device) {
		std::cout << "error: failed to find a device with serial number: " << deviceSerial << std::endl;
		return -1;
	}

	std::cout << "info: found " << device->describe() << std::endl;

	if(!device->open()) {
		std::cout << "error: unable to open device" << std::endl;
	}

	device->stopScript();

	uint64_t firstOffset;
	std::shared_ptr<icsneo::VSA> firstRecord;
	if(!device->findFirstVSARecord(firstOffset, firstRecord)) {
		std::cout << "error: unable to find first VSA record" << std::endl;
		return -1;
	}
	std::cout << "info: found first VSA record at " << firstOffset << std::endl;

	uint64_t origLastOffset;
	std::shared_ptr<icsneo::VSA> origLastRecord;
	if(!device->findLastVSARecord(origLastOffset, origLastRecord)) {
		std::cout << "error: unable to find last VSA record" << std::endl;
		return -1;
	}
	std::cout << "info: found last VSA record at " << origLastOffset << std::endl;

	uint64_t canFrameCount = 0;
	uint64_t ethFrameCount = 0;
	device->addMessageCallback(std::make_shared<icsneo::MessageCallback>([&](std::shared_ptr<icsneo::Message> msg) {
		if(msg->type != icsneo::Message::Type::Frame) {
			return;
		}
		const auto frame = std::static_pointer_cast<icsneo::Frame>(msg);
		if(frame->network.getType() == icsneo::Network::Type::CAN) {
			++canFrameCount;
		} else if(frame->network.getType() == icsneo::Network::Type::Ethernet) {
			++ethFrameCount;
		}
	}));
	icsneo::VSAExtractionSettings settings;
	{
		auto& filter = settings.filters.emplace_back();
		filter.readRange.first = origLastRecord->getTimestampICSClock() - std::chrono::seconds(20);
		filter.readRange.second = origLastRecord->getTimestampICSClock() - std::chrono::seconds(10);
	}
	{
		auto& filter = settings.filters.emplace_back();
		filter.readRange.first = firstRecord->getTimestampICSClock() + std::chrono::seconds(0);
		filter.readRange.second = firstRecord->getTimestampICSClock() + std::chrono::seconds(10);
	}
	std::cout << "info: reading two blocks of VSA, 10s from the start and 10s from the end..." << std::endl;
	if(!device->readVSA(settings)) {
		std::cout << "error: unable to read VSA" << std::endl;
		return -1;
	}
	std::cout << "info: processed " << canFrameCount << " CAN frames and " << ethFrameCount << " Ethernet frames" << std::endl;

	return 0;
}