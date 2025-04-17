#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#include "icsneo/icsneocpp.h"
#include "icsneo/communication/message/apperrormessage.h"
#include "icsneo/communication/message/message.h"
/*
 * App errors are responses from the device indicating internal runtime errors
 * NOTE: To trigger the app error in this example, disable the DW CAN 01 network on the device 
 * (e.g. with neoVI Explorer)
 */
int main() {
	std::cout << "Running libicsneo " << icsneo::GetVersion() << std::endl;
	std::cout << "\nFinding devices... " << std::flush;
	auto devices = icsneo::FindAllDevices();
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

		// Create an app error message filter, including "internal" messages
		auto filter = std::make_shared<icsneo::MessageFilter>(icsneo::Message::Type::AppError);
		filter->includeInternalInAny = true;

		// ...and register a callback with it.
		// Add your error handling here
		auto handler = device->addMessageCallback(std::make_shared<icsneo::MessageCallback>(filter, [](std::shared_ptr<icsneo::Message> message) {
			auto msg = std::static_pointer_cast<icsneo::AppErrorMessage>(message);
			if(icsneo::Network::NetID::RED_App_Error == msg->network.getNetID()) {
				std::cout << std::endl << "App error reported:" << std::endl;
				std::cout << msg->getAppErrorString() << std::endl << std::endl;
			}
		}));

		std::cout << "Going online... ";
		ret = device->goOnline();
		if(!ret) {
			std::cout << "FAIL" << std::endl;
			device->close();
			continue;
		}
		std::cout << "OK" << std::endl;

		// Prepare a CAN message
		std::cout << std::endl << "Transmitting a CAN frame... ";
		auto txMessage = std::make_shared<icsneo::CANMessage>();
		txMessage->network = icsneo::Network::NetID::DWCAN_01;
		txMessage->arbid = 0x22;
		txMessage->data.insert(txMessage->data.end(), {0xaa, 0xbb, 0xcc});
		// The DLC will come from the length of the data vector
		txMessage->isExtended = false;
		txMessage->isCANFD = false;

		// Transmit a CAN message on DW CAN 01, even though DW CAN 01 is disabled on the device!
		// Expect to see an app error caught in the callback defined above
		ret = device->transmit(txMessage); 
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

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