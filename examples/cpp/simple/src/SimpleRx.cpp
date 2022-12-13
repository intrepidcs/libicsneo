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

	std::cout << "Streaming CAN messages in, enter anything to stop..." << std::endl;
	auto handler = device->addMessageCallback(std::make_shared<icsneo::MessageCallback>([&](std::shared_ptr<icsneo::Message> message) {
		if(message->type == icsneo::Message::Type::Frame) {
			auto frame = std::static_pointer_cast<icsneo::Frame>(message);
			if(frame->network.getType() == icsneo::Network::Type::CAN) {
				auto canMessage = std::static_pointer_cast<icsneo::CANMessage>(message);
				std::cout << '\r';
				for(auto& databyte : canMessage->data)
					std::cout << std::hex << std::setw(2) << (uint32_t)databyte << ' ';
				std::cout << std::flush;
			}
		}
	}));

	std::cin.ignore();

	device->removeMessageCallback(handler);

	return 0;
}
