#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#include "icsneo/icsneocpp.h"
#include "icsneo/communication/message/linmessage.h"

/* Note: This example requires LIN 1 and LIN 2 channels to be connected on the device */

char getCharInput(std::vector<char> allowed) {
	bool found = false;
	std::string input;

	while(!found) {
		std::cin >> input;

		if(input.length() == 1) {
			for(char compare : allowed) {
				if(compare == input.c_str()[0]) {
					found = true;
					break;
				}
			}
		}

		if(!found) {
			std::cout << "Input did not match expected options. Please try again." << std::endl;
			std::cout << "<X or x to quit>" << std::endl;
		}
	}

	return input.c_str()[0];
}

int main() {
	// Print version
	std::cout << "Running libicsneo " << icsneo::GetVersion() << std::endl;
	std::cout << "\nFinding devices... " << std::flush;
	auto devices = icsneo::FindAllDevices(); // This is type std::vector<std::shared_ptr<icsneo::Device>>
	// You now hold the shared_ptrs for these devices, you are considered to "own" these devices from a memory perspective
	std::cout << "OK, " << devices.size() << " device" << (devices.size() == 1 ? "" : "s") << " found" << std::endl;

	// List off the devices
	for(auto& device : devices)
		std::cout << '\t' << device->describe() << " @ Handle " << device->getNeoDevice().handle << std::endl;
	std::cout << std::endl;

	for(auto& device : devices) {
		std::cout << "Connecting to " << device->describe() << "... ";
		bool ret = device->open();
		if(!ret) { // Failed to open
			std::cout << "FAIL" << std::endl;
			std::cout << icsneo::GetLastError() << std::endl << std::endl;
			continue;
		}
		std::cout << "OK" << std::endl;

		// The concept of going "online" tells the connected device to start listening, i.e. ACKing traffic and giving it to us
		std::cout << "\tGoing online... ";
		ret = device->goOnline();
		if(!ret) {
			std::cout << "FAIL" << std::endl;
			device->close();
			continue;
		}
		std::cout << "OK" << std::endl;

		// A real application would just check the result of icsneo_goOnline() rather than calling this
		// This function is intended to be called later on if needed
		std::cout << "\tChecking online status... ";
		ret = device->isOnline();
		if(!ret) {
			std::cout << "FAIL\n" << std::endl;
			device->close();
			continue;
		}
		std::cout << "OK" << std::endl;

		auto handler = device->addMessageCallback(std::make_shared<icsneo::MessageCallback>([&](std::shared_ptr<icsneo::Message> message) {
			if(icsneo::Message::Type::Frame == message->type) {
				auto frame = std::static_pointer_cast<icsneo::Frame>(message);
				if(icsneo::Network::Type::LIN == frame->network.getType()) {
					auto msg = std::static_pointer_cast<icsneo::LINMessage>(message);
					std::cout << msg->network << " RX frame | ID: 0x" << std::hex << static_cast<int>(msg->ID) << " | ";
					std::cout << "Protected ID: 0x" << static_cast<int>(msg->protectedID) << "\n" << "Data: ";
					for(uint8_t& each : msg->data) {
						std::cout << "0x" << static_cast<int>(each) << " ";
					}
					std::cout << "\nChecksum type: " << (msg->isEnhancedChecksum ? "Enhanced" : "Classic");
					std::cout << "\nChecksum: 0x" << static_cast<int>(msg->checksum) << "\n";
					std::cout << "Is checksum valid: " << ((!msg->errFlags.ErrChecksumMatch) ? "yes" : "no") << "\n\n";
				}
			}
		}));

		// We can transmit messages
		std::cout << "\tTransmitting a LIN responder data frame... ";
		auto lin_r = std::make_shared<icsneo::LINMessage>();
		lin_r->network = icsneo::Network::NetID::LIN2;
		lin_r->ID = 0x11;
		lin_r->type = icsneo::LINMessage::Type::LIN_UPDATE_RESPONDER;
		lin_r->data = {0xaa, 0xbb, 0xcc, 0xdd, 0x11, 0x22, 0x33, 0x44};
		ret = device->transmit(lin_r); // This will return false if the device does not support LIN
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "\tTransmitting a LIN commander frame... ";
		auto lin_c = std::make_shared<icsneo::LINMessage>();
		lin_c->network = icsneo::Network::NetID::LIN;
		lin_c->ID = 0x11;
		lin_c->type = icsneo::LINMessage::Type::LIN_HEADER_ONLY;
		ret = device->transmit(lin_c);
		std::cout << (ret ? "OK" : "FAIL") << std::endl << std::endl;

		std::cout << "\tTransmitting a LIN commander frame with responder data... ";
		auto lin_d = std::make_shared<icsneo::LINMessage>();
		lin_d->network = icsneo::Network::NetID::LIN;
		lin_d->ID = 0x22;
		lin_d->isEnhancedChecksum = true;
		lin_d->type = icsneo::LINMessage::Type::LIN_COMMANDER_MSG;
		lin_d->data = {0x11, 0x22, 0x33, 0x44, 0xaa, 0xbb, 0xcc, 0xdd};
		ret = device->transmit(lin_d);
		std::cout << (ret ? "OK" : "FAIL") << std::endl << std::endl;
		std::cout << "<X or x to quit>\n\n";

		// Go offline, stop sending and receiving traffic
		auto shutdown = [&](){
			device->removeMessageCallback(handler); 
			std::cout << "\tGoing offline... ";
			ret = device->goOffline();
			std::cout << (ret ? "OK" : "FAIL") << std::endl;
			std::cout << "\tDisconnecting... ";
			ret = device->close();
			std::cout << (ret ? "OK\n" : "FAIL\n") << std::endl;
		};

		while(true) {
			char input = getCharInput(std::vector<char> {'X', 'x'});
			switch(input) {
				case 'X':
				case 'x':
					shutdown();
					printf("Exiting program\n");
					return 0;
				default:
					break;
			}
		}
	}
	return 0;
}