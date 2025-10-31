#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#include "icsneo/icsneocpp.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996) // STL time functions
#endif

int main() {
	// Print version
	std::cout << "Running libicsneo " << icsneo::GetVersion() << std::endl;

	std::cout<< "Supported devices:" << std::endl;
	for(auto& dev : icsneo::GetSupportedDevices())
		std::cout << '\t' << dev.getGenericProductName() << std::endl;

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

		std::cout << "\tGetting DW CAN 01 Baudrate... ";
		int64_t baud = device->settings->getBaudrateFor(icsneo::Network::NetID::DWCAN_01);
		if(baud < 0)
			std::cout << "FAIL" << std::endl;
		else
			std::cout << "OK, " << (baud/1000) << "kbit/s" << std::endl;

		std::cout << "\tSetting DW CAN 01 to operate at 125kbit/s... ";
		ret = device->settings->setBaudrateFor(icsneo::Network::NetID::DWCAN_01, 125000);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		// Changes to the settings do not take affect until you call settings->apply()!
		// When you get the baudrate here, you're reading what the device is currently operating on
		std::cout << "\tGetting DW CAN 01 Baudrate... (expected to be unchanged) ";
		baud = device->settings->getBaudrateFor(icsneo::Network::NetID::DWCAN_01);
		if(baud < 0)
			std::cout << "FAIL" << std::endl;
		else
			std::cout << "OK, " << (baud/1000) << "kbit/s" << std::endl;

		std::cout << "\tGetting DW CAN 01 FD Baudrate... ";
		baud = device->settings->getFDBaudrateFor(icsneo::Network::NetID::DWCAN_01);
		if(baud < 0)
			std::cout << "FAIL" << std::endl;
		else
			std::cout << "OK, " << (baud/1000) << "kbit/s" << std::endl;

		std::cout << "\tSetting DW CAN 01 FD to operate at 8Mbit/s... ";
		ret = device->settings->setFDBaudrateFor(icsneo::Network::NetID::DWCAN_01, 8000000);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "\tGetting DW CAN 01 FD Baudrate... (expected to be unchanged) ";
		baud = device->settings->getFDBaudrateFor(icsneo::Network::NetID::DWCAN_01);
		if(baud < 0)
			std::cout << "FAIL" << std::endl;
		else
			std::cout << "OK, " << (baud/1000) << "kbit/s" << std::endl;

		// Setting settings temporarily does not need to be done before committing to device EEPROM
		// It's done here to test both functionalities
		// Setting temporarily will keep these settings until another send/commit is called or a power cycle occurs
		std::cout << "\tSetting settings temporarily... ";
		ret = device->settings->apply(true);
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		// Now that we have applied, we expect that our operating baudrates have changed
		std::cout << "\tGetting DW CAN 01 Baudrate... ";
		baud = device->settings->getBaudrateFor(icsneo::Network::NetID::DWCAN_01);
		if(baud < 0)
			std::cout << "FAIL" << std::endl;
		else
			std::cout << "OK, " << (baud/1000) << "kbit/s" << std::endl;

		std::cout << "\tGetting DW CAN 01 (FD) Baudrate... ";
		baud = device->settings->getFDBaudrateFor(icsneo::Network::NetID::DWCAN_01);
		if(baud < 0)
			std::cout << "FAIL" << std::endl;
		else
			std::cout << "OK, " << (baud/1000) << "kbit/s" << std::endl;

		std::cout << "\tSetting settings permanently... ";
		ret = device->settings->apply();
		std::cout << (ret ? "OK\n\n" : "FAIL\n\n");
		
		const auto getRTC = [&]() {
			std::cout << "\tGetting RTC... ";
			const auto rtc = device->getRTC();
			if(!rtc) {
				std::cout << "FAIL" << std::endl;
				return;
			}
			const auto time = std::chrono::system_clock::to_time_t(*rtc);
			const auto timeInfo = std::gmtime(&time);
			std::cout << "OK, " << std::put_time(timeInfo, "%Y-%m-%d %H:%M:%S") << std::endl;
		};
		// Set the real time clock on the device using the system clock
		// First, let's see if we can get the time from the device (if it has an RTC)
		getRTC();

		// Now, set the time using the system's clock so that we can check it again to ensure it's set
		std::cout << "\tSetting RTC to system_clock::now()... ";
		std::cout << (device->setRTC(std::chrono::system_clock::now()) ? "OK" : "FAIL") << std::endl;

		// Get the time again after setting
		getRTC();

		// The concept of going "online" tells the connected device to start listening, i.e. ACKing traffic and giving it to us
		std::cout << "\n\tGoing online... ";
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

		// Now we can either register a handler (or multiple) for messages coming in
		// or we can enable message polling, and then call device->getMessages periodically

		// We're actually going to do both here, so first enable message polling
		device->enableMessagePolling();
		device->setPollingMessageLimit(100000); // Feel free to set a limit if you like, the default is a conservative 20k
		// Keep in mind that 20k messages comes quickly at high bus loads!

		// We can transmit messages
		std::cout << "\n\tTransmitting an extended CAN FD frame... ";
		auto txMessage5 = std::make_shared<icsneo::CANMessage>();
		txMessage5->network = icsneo::Network::NetID::DWCAN_01;
		txMessage5->arbid = 0x1C5001C5;
		txMessage5->data.insert(txMessage5->data.end(), {0xaa, 0xbb, 0xcc});
		// The DLC will come from the length of the data vector
		txMessage5->isExtended = true;
		txMessage5->isCANFD = true;
		ret = device->transmit(txMessage5); // This will return false if the device does not support CAN FD, or does not have DWCAN_01
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		// We can also register a handler
		std::cout << "\tStreaming messages in for 3 seconds... " << std::endl;
		// MessageCallbacks are powerful, and can filter on things like ArbID for you. See the documentation
		auto handler = device->addMessageCallback(std::make_shared<icsneo::MessageCallback>([](std::shared_ptr<icsneo::Message> message) {
			switch(message->type) {
				case icsneo::Message::Type::Frame: {
					// A message of type Frame is guaranteed to be a Frame, so we can static cast safely
					auto frame = std::static_pointer_cast<icsneo::Frame>(message);
					switch(frame->network.getType()) {
						case icsneo::Network::Type::CAN: {
							// A message of type CAN is guaranteed to be a CANMessage, so we can static cast safely
							auto canMessage = std::static_pointer_cast<icsneo::CANMessage>(message);

							std::cout << "\t\t" << frame->network << ' ';
							if(canMessage->isCANFD) {
								std::cout << "FD ";
								if(!canMessage->baudrateSwitch)
									std::cout << "(No BRS) ";
							}

							// Print the Arbitration ID
							std::cout << "0x" << std::hex << std::setw(canMessage->isExtended ? 8 : 3)
								<< std::setfill('0') << canMessage->arbid;

							// Print the DLC
							std::cout << std::dec << " [" << canMessage->data.size() << "] ";

							// Print the data
							for(auto& databyte : canMessage->data)
								std::cout << std::hex << std::setw(2) << (uint32_t)databyte << ' ';

							// Print the timestamp
							std::cout << std::dec << '(' << canMessage->timestamp << " ns since 1/1/2007)\n";
							break;
						}
						case icsneo::Network::Type::Ethernet:
						case icsneo::Network::Type::AutomotiveEthernet: {
							auto ethMessage = std::static_pointer_cast<icsneo::EthernetMessage>(message);

							std::cout << "\t\t" << ethMessage->network << " Frame - " << std::dec
								<< ethMessage->data.size() << " bytes on wire\n";
							std::cout << "\t\t  Timestamped:\t"<< ethMessage->timestamp << " ns since 1/1/2007\n";

							// The MACAddress may be printed directly or accessed with the `data` member
							std::cout << "\t\t  Source:\t" << ethMessage->getSourceMAC() << "\n";
							std::cout << "\t\t  Destination:\t" << ethMessage->getDestinationMAC();

							// Print the data
							for(size_t i = 0; i < ethMessage->data.size(); i++) {
								if(i % 8 == 0)
									std::cout << "\n\t\t  " << std::hex << std::setw(4) << std::setfill('0') << i << '\t';
								std::cout << std::hex << std::setw(2) << (uint32_t)ethMessage->data[i] << ' ';
							}

							std::cout << std::dec << std::endl;
							break;
						}
						case icsneo::Network::Type::ISO9141: {
							// Note that the default settings on some devices have ISO9141 disabled by default in favor of LIN
							// and that this example loads the device defaults at the very end.
							// A message of type ISO9414 is guaranteed to be an ISO9141Message, so we can static cast safely
							auto isoMessage = std::static_pointer_cast<icsneo::ISO9141Message>(message);

							std::cout << "\t\t" << isoMessage->network << ' ';

							// Print the header bytes
							std::cout << '(' << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)isoMessage->header[0] << ' ';
							std::cout << std::setfill('0') << std::setw(2) << (uint32_t)isoMessage->header[1] << ' ';
							std::cout << std::setfill('0') << std::setw(2) << (uint32_t)isoMessage->header[2] << ") ";

							// Print the data length
							std::cout << std::dec << " [" << isoMessage->data.size() << "] ";

							// Print the data
							for(auto& databyte : isoMessage->data)
								std::cout << std::hex << std::setfill('0') << std::setw(2) << (uint32_t)databyte << ' ';

							// Print the timestamp
							std::cout << std::dec << '(' << isoMessage->timestamp << " ns since 1/1/2007)\n";
							break;
						}
						default:
							// Ignoring non-network messages
							break;
					}
					break;
				} // end of icsneo::Message::Type::Frame
				case icsneo::Message::Type::CANErrorCount: {
					// A message of type CANErrorCount is guaranteed to be a CANErrorCount, so we can static cast safely
					auto cec = std::static_pointer_cast<icsneo::CANErrorMessage>(message);

					// Print the error counts
					std::cout << "\t\t" << cec->network << " error counts changed, REC=" << std::to_string(cec->receiveErrorCount)
						<< " TEC=" << std::to_string(cec->transmitErrorCount) << " (" << (cec->busOff ? "" : "Not ") << "Bus Off) ";

					// Print the timestamp
					std::cout << std::dec << '(' << cec->timestamp << " ns since 1/1/2007)\n";
					break;
				}
				default:
					// Ignoring other types of messages
					break;
			}
		}));
		std::this_thread::sleep_for(std::chrono::seconds(3));
		device->removeMessageCallback(handler); // Removing the callback means it will not be called anymore

		// Since we're using message polling, we can also get the messages which have come in for the past 3 seconds that way
		// We could simply call getMessages and it would return a vector of message pointers to us
		//auto messages = device->getMessages();

		// For speed when calling repeatedly, we can also preallocate and continually reuse a vector
		std::vector<std::shared_ptr<icsneo::Message>> messages;
		messages.reserve(100000);
		device->getMessages(messages);
		std::cout << "\t\tGot " << messages.size() << " messages while polling" << std::endl;

		// If we wanted to make sure it didn't grow and reallocate, we could also pass in a limit
		// If there are more messages than the limit, we can call getMessages repeatedly
		//device->getMessages(messages, 100);

		// You are now the owner (or one of the owners, if multiple handlers are registered) of the shared_ptrs to the messages
		// This means that when you let them go out of scope or reuse the vector, the messages will be freed automatically

		// We can transmit messages
		std::cout << "\tTransmitting an extended CAN FD frame... ";
		auto txMessage = std::make_shared<icsneo::CANMessage>();
		txMessage->network = icsneo::Network::NetID::DWCAN_01;
		txMessage->arbid = 0x1C5001C5;
		txMessage->data.insert(txMessage->data.end(), {0xaa, 0xbb, 0xcc});
		// The DLC will come from the length of the data vector
		txMessage->isExtended = true;
		txMessage->isCANFD = true;
		ret = device->transmit(txMessage); // This will return false if the device does not support CAN FD, or does not have DWCAN_01
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "\tTransmitting an ethernet frame on AE 02... ";
		auto ethTxMessage = std::make_shared<icsneo::EthernetMessage>();
		ethTxMessage->network = icsneo::Network::NetID::AE_02;
		ethTxMessage->data.insert(ethTxMessage->data.end(), {
			0x00, 0xFC, 0x70, 0x00, 0x01, 0x02, /* Destination MAC */
			0x00, 0xFC, 0x70, 0x00, 0x01, 0x01, /* Source MAC */
			0x00, 0x00, /* Ether Type */
			0x01, 0xC5, 0x01, 0xC5 /* Payload (will automatically be padded on transmit unless you set `ethTxMessage->noPadding`) */
		});
		ret = device->transmit(ethTxMessage); // This will return false if the device does not support AE 02
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::vector<icsneo::MiscIO> emisc = device->getEMiscIO();
		if(!emisc.empty()) {
			std::cout << "\tReading EMisc values..." << std::endl;
			for(const auto& io : emisc) {
				std::optional<double> val = device->getAnalogIO(icsneo::IO::EMisc, io.number);
				std::cout << "\t\tEMISC" << io.number;
				if(val.has_value())
					std::cout << " - OK (" << val.value() << "V)" << std::endl;
				else
					std::cout << " - FAIL, it may need to be enabled in neoVI Explorer (" << icsneo::GetLastError() << ")" << std::endl;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		// Go offline, stop sending and receiving traffic
		std::cout << "\tGoing offline... ";
		ret = device->goOffline();
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		// Apply default settings
		std::cout << "\tSetting default settings... ";
		ret = device->settings->applyDefaults(); // This will also write to the device
		std::cout << (ret ? "OK" : "FAIL") << std::endl;

		std::cout << "\tDisconnecting... ";
		ret = device->close();
		std::cout << (ret ? "OK\n" : "FAIL\n") << std::endl;
	}

	std::cout << "Press any key to continue..." << std::endl;
	std::cin.get();
	return 0;
}