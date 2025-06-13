#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#include "icsneo/icsneocpp.h"
#include "icsneo/communication/message/livedatamessage.h"
#include "icsneo/communication/livedata.h"

int main() {
	// Print version
	std::cout << "Running libicsneo " << icsneo::GetVersion() << std::endl;
	std::cout << "\nFinding devices... " << std::flush;
	auto devices = icsneo::FindAllDevices(); // This is type std::vector<std::shared_ptr<icsneo::Device>>
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

		// Create a subscription message for the GPS signals
		std::cout << "\tSending a live data subscribe command... ";
		auto msg = std::make_shared<icsneo::LiveDataCommandMessage>();
		msg->appendSignalArg(icsneo::LiveDataValueType::GPS_LATITUDE);
		msg->appendSignalArg(icsneo::LiveDataValueType::GPS_LONGITUDE);
		msg->appendSignalArg(icsneo::LiveDataValueType::GPS_ACCURACY);
		msg->appendSignalArg(icsneo::LiveDataValueType::DAQ_OVERRIDE);
		msg->appendSignalArg(icsneo::LiveDataValueType::MANUAL_TRIGGER);
		msg->cmd = icsneo::LiveDataCommand::SUBSCRIBE;
		msg->handle = icsneo::LiveDataUtil::getNewHandle();
		msg->updatePeriod = std::chrono::milliseconds(100);
		msg->expirationTime = std::chrono::milliseconds(0);
		// Transmit the subscription message
		ret = device->subscribeLiveData(msg);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		// Register a handler that uses the data after it arrives every ~100ms
		std::cout << "\tStreaming messages for 3 seconds... " << std::endl << std::endl;
		auto filter = std::make_shared<icsneo::MessageFilter>(icsneo::Message::Type::LiveData);
		auto handler = device->addMessageCallback(std::make_shared<icsneo::MessageCallback>(filter, [&msg](std::shared_ptr<icsneo::Message> message) {
			auto ldMsg = std::dynamic_pointer_cast<icsneo::LiveDataMessage>(message);
			switch(ldMsg->cmd) {
				case icsneo::LiveDataCommand::STATUS: {
					auto msg2 = std::dynamic_pointer_cast<icsneo::LiveDataStatusMessage>(message);
					std::cout << "[Handle] " << ldMsg->handle << std::endl;
					std::cout << "[Requested Command] " << msg2->requestedCommand << std::endl;
					std::cout << "[Status] " << msg2->status << std::endl << std::endl;
					break;
				}
				case icsneo::LiveDataCommand::RESPONSE: {
					auto valueMsg = std::dynamic_pointer_cast<icsneo::LiveDataValueMessage>(message);
					if((valueMsg->handle == msg->handle) && (valueMsg->values.size() == msg->args.size()))
					{
						std::cout << "[Handle] " << msg->handle << std::endl;
						std::cout << "[Values] " << valueMsg->numArgs << std::endl;
						for(uint32_t i = 0; i < valueMsg->numArgs; ++i) {
							std::cout << "[" << msg->args[i]->valueType << "] ";
							auto scaledValue = icsneo::LiveDataUtil::liveDataValueToDouble(*valueMsg->values[i]);
							std::cout << scaledValue << std::endl;
						}
						std::cout << std::endl;
					}
					break;
				}
				default: // Ignoring other commands
					break;
			}
		}));
		// Run handler for three seconds to observe the signal data
		std::this_thread::sleep_for(std::chrono::seconds(3));
		auto setValMsg = std::make_shared<icsneo::LiveDataSetValueMessage>();
		setValMsg->cmd = icsneo::LiveDataCommand::SET_VALUE;
		setValMsg->handle = msg->handle;
		bool onOff = true;
		for (unsigned int i = 0; i < 10; ++i)
		{
			setValMsg->args.clear();
			setValMsg->values.clear();
			double val;
			if (onOff)
			{
				val = 1;
			}
			else
			{
				val = 0;
			}
			onOff = !onOff;
			icsneo::LiveDataValue ldValue;
			if (icsneo::LiveDataUtil::liveDataDoubleToValue(ldValue, val) < 0)
			{
				break;
			}
			setValMsg->appendSetValue(icsneo::LiveDataValueType::DAQ_OVERRIDE, ldValue);
			device->setValueLiveData(setValMsg);
			// Test setting values
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}
		// Unsubscribe from the GPS signals and run handler for one more second
		// Unsubscription only requires a valid in-use handle, in this case from our previous subscription
		ret = device->unsubscribeLiveData(msg->handle);
		// The handler should no longer print values
		std::this_thread::sleep_for(std::chrono::seconds(1));
		device->removeMessageCallback(handler);
		std::cout << "\tDisconnecting... ";
		ret = device->close();
		std::cout << (ret ? "OK\n" : "FAIL\n") << std::endl;
	}
	return 0;
}