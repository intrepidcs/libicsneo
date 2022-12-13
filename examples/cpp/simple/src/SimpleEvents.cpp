#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "icsneo/icsneocpp.h"

int main(int argc, char** argv) {
	std::vector<std::string> args(argv, argv + argc);
	if(args.size() != 2) {
		std::cerr << "usage: " << args.front() << " <device serial>" << std::endl;
		return -1;
	}

	const auto& deviceSerial = args[1];

	const auto findDevice = [](const auto& serial) -> std::shared_ptr<icsneo::Device> {
		for(const auto& dev : icsneo::FindAllDevices()) {
			if(serial == dev->getSerial())
				return dev;
		}
		return nullptr;
	};

	std::cout << "Finding device... " << std::flush;
	const auto device = findDevice(deviceSerial);
	if(!device) {
		std::cerr << "FAIL" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Opening device... " << std::flush;
	if(!device->open()) {
		std::cerr << "FAIL" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Going online... " << std::flush;
	if(!device->goOnline()) {
		std::cerr << "FAIL" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Getting events from server..." << std::endl;
	icsneo::EventFilter ef = {};
	auto events = icsneo::GetEvents(ef, 0UL);
	if(events.size() == 0)
		std::cout << "Event return is empty :(" << std::endl;

	for(auto& event: events)
		std::cout << event.describe() << std::endl;

	return 0;
}
