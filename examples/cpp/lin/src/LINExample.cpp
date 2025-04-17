#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#include "icsneo/icsneocpp.h"
#include "icsneo/communication/message/linmessage.h"

/* Note: This example requires LIN 1 and LIN 2 channels to be connected on the device */
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

	for(auto device : devices) {
		std::cout << "Connecting to " << device->describe() << "... ";
		bool ret = device->open();
		if(!ret) { // Failed to open
			std::cout << "FAIL" << std::endl;
			std::cout << icsneo::GetLastError() << std::endl << std::endl;
			continue;
		}
		std::cout << "OK" << std::endl << std::endl;

		int64_t baud = 19200;

		std::cout << "Enable LIN 01 commander resistor... ";
		ret = device->settings->setCommanderResistorFor(icsneo::Network::NetID::LIN_01, true);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "Disable LIN 02 commander resistor... ";
		ret = device->settings->setCommanderResistorFor(icsneo::Network::NetID::LIN_02, false);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "Setting LIN 01 to operate at " << baud << "bit/s... ";
		ret = device->settings->setBaudrateFor(icsneo::Network::NetID::LIN_01, baud);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "Setting LIN 02 to operate at " << baud << "bit/s... ";
		ret = device->settings->setBaudrateFor(icsneo::Network::NetID::LIN_02, baud);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "Setting LIN 01 mode to NORMAL... ";
		ret = device->settings->setLINModeFor(icsneo::Network::NetID::LIN_01, NORMAL_MODE);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "Setting LIN 02 mode to NORMAL... ";
		ret = device->settings->setLINModeFor(icsneo::Network::NetID::LIN_02, NORMAL_MODE);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "Applying settings... ";
		ret = device->settings->apply();
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "Getting LIN Baudrate... ";
		int64_t readBaud = device->settings->getBaudrateFor(icsneo::Network::NetID::LIN_01);
		if(readBaud < 0)
			std::cout << "FAIL" << std::endl;
		else
			std::cout << "OK, " << (readBaud) << "bit/s" << std::endl;

		std::cout << "Getting LIN 02 Baudrate... ";
		readBaud = device->settings->getBaudrateFor(icsneo::Network::NetID::LIN_02);
		if(readBaud < 0)
			std::cout << "FAIL" << std::endl;
		else
			std::cout << "OK, " << (readBaud) << "bit/s" << std::endl << std::endl;

		// The concept of going "online" tells the connected device to start listening, i.e. ACKing traffic and giving it to us
		std::cout << "Going online... ";
		ret = device->goOnline();
		if(!ret) {
			std::cout << "FAIL" << std::endl;
			device->close();
			continue;
		}
		std::cout << "OK" << std::endl;

		// A real application would just check the result of icsneo_goOnline() rather than calling this
		// This function is intended to be called later on if needed
		std::cout << "Checking online status... ";
		ret = device->isOnline();
		if(!ret) {
			std::cout << "FAIL\n" << std::endl;
			device->close();
			continue;
		}
		std::cout << "OK" << std::endl << std::endl;

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
		std::cout << "Transmitting a LIN 02 responder data frame... ";
		auto lin_r = std::make_shared<icsneo::LINMessage>();
		lin_r->network = icsneo::Network::NetID::LIN_02;
		lin_r->ID = 0x11;
		lin_r->linMsgType = icsneo::LINMessage::Type::LIN_UPDATE_RESPONDER;
		lin_r->data = {0xaa, 0xbb, 0xcc, 0xdd, 0x11, 0x22, 0x33, 0x44};
		ret = device->transmit(lin_r); // This will return false if the device does not support LIN
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "Transmitting a LIN 01 commander header... ";
		auto lin_c = std::make_shared<icsneo::LINMessage>();
		lin_c->network = icsneo::Network::NetID::LIN_01;
		lin_c->ID = 0x11;
		lin_c->linMsgType = icsneo::LINMessage::Type::LIN_HEADER_ONLY;
		ret = device->transmit(lin_c);
		std::cout << (ret ? "OK" : "FAIL") << std::endl << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		std::cout << "Transmitting a LIN 01 commander frame with responder data... ";
		auto lin_d = std::make_shared<icsneo::LINMessage>();
		lin_d->network = icsneo::Network::NetID::LIN_01;
		lin_d->ID = 0x22;
		lin_d->isEnhancedChecksum = true;
		lin_d->linMsgType = icsneo::LINMessage::Type::LIN_COMMANDER_MSG;
		lin_d->data = {0x11, 0x22, 0x33, 0x44, 0xaa, 0xbb, 0xcc, 0xdd};
		ret = device->transmit(lin_d);
		std::cout << (ret ? "OK" : "FAIL") << std::endl << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// Go offline, stop sending and receiving traffic
		device->removeMessageCallback(handler);
		std::cout << "Going offline... ";
		ret = device->goOffline();
		std::cout << (ret ? "OK" : "FAIL") << std::endl;
		std::cout << "Disconnecting... ";
		ret = device->close();
		std::cout << (ret ? "OK\n" : "FAIL\n") << std::endl;
	}
	return 0;
}