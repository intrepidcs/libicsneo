#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#include "icsneo/icsneocpp.h"

const std::string rada2bSerial = "Your RADA2B serial number.";

int main() {
	std::cout << "Start example\n";
	auto devices = icsneo::FindAllDevices();

	std::shared_ptr<icsneo::Device> rada2b;
	for(auto& device : devices) {
		if(device->getSerial() == rada2bSerial) {
			rada2b = device;
		}
	}

	rada2b->open();
	rada2b->goOnline();
	std::cout << rada2b->describe() << "\n";

	auto handler1 = rada2b->addMessageCallback(std::make_shared<icsneo::A2BWAVOutput>("examples/cpp/a2b/src/out.wav")); // Starts writing A2B PCM data to out.wav

	auto handler2 = rada2b->addMessageCallback(
		std::make_shared<icsneo::MessageCallback>(
			[](std::shared_ptr<icsneo::Message> message) {
				std::cout << "Got in callback " << std::endl;
				if(message->type == icsneo::Message::Type::Frame) {
					std::shared_ptr<icsneo::Frame> frame = std::static_pointer_cast<icsneo::Frame>(message);

					if(frame->network.getType() == icsneo::Network::Type::A2B) {
						std::shared_ptr<icsneo::A2BMessage> msg = std::static_pointer_cast<icsneo::A2BMessage>(frame);
						std::cout << "Got A2B Message" << std::endl;
					}
				}
			}));

	std::this_thread::sleep_for(std::chrono::seconds(5)); // captures 5 seconds of A2B data.

	rada2b->removeMessageCallback(handler1);
	rada2b->removeMessageCallback(handler2);

	std::cout << "End A2B example\n";

	rada2b->goOffline();
	rada2b->close();

	return 0;
}