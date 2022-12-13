#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <memory>

#include "icsneo/icsneocpp.h"

int main(int argc, char** argv) {
	std::vector<std::string> args(argv, argv + argc);
	if (args.size() != 2) {
		std::cerr << "usage: " << args.front() << " <device serial>" << std::endl;
		return -1;
	}

	const auto& deviceSerial = args[1];

	const auto findDevice = [](const auto& serial) -> std::shared_ptr<icsneo::Device> {
		for (const auto& dev : icsneo::FindAllDevices()) {
			if (serial == dev->getSerial())
				return dev;
		}
		return nullptr;
	};

	std::cout << "Finding device... " << std::flush;
	const auto device = findDevice(deviceSerial);
	if (!device) {
		std::cerr << "FAIL" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Opening device... " << std::flush;
	if (!device->open()) {
		std::cerr << "FAIL" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::cout << "Going online... " << std::flush;
	if (!device->goOnline()) {
		std::cerr << "FAIL" << std::endl;
		return -1;
	}
	std::cout << "OK" << std::endl;

	std::atomic<bool> stop = false;

	std::thread thread([&] {
		std::cin.ignore();
		std::cout << "Stopping..." << std::endl;
		stop = true;
	});

	auto txMessage = std::make_shared<icsneo::CANMessage>();
	txMessage->network = icsneo::Network::NetID::HSCAN;
	txMessage->arbid = 0x1C5001C5;
	txMessage->data.insert(txMessage->data.begin(), sizeof(size_t), 0);
	txMessage->isExtended = true;
	txMessage->isCANFD = true;

	std::cout << "Streaming CAN messages out, enter anything to stop..." << std::endl;

	size_t& value = *(size_t*)txMessage->data.data();

	while (!stop) {
		device->transmit(txMessage);
		value++;
	}

	thread.join();

	return 0;
}
