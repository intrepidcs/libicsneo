/**
 * libicsneo SPI message example
 * 
 * Read a register on NetID::SPI_01 and display the result to stdout
 * 
 * Usage: libicsneo-spi [deviceSerial] [address] [numWords]
 * 
 * Arguments:
 * deviceSerial: 6 character string for device serial
 * address: 16 bit register address
 * numWords: number of words to read
 */


#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <string_view>
#include <utility>
#include <tuple>

#include "icsneo/icsneocpp.h"
#include "icsneo/communication/message/spimessage.h"

static const std::string usage = "Usage: libicsneo-spi [deviceSerial] [address] [numWords]\n\n"
								"Arguments:\n"
								"deviceSerial: 6 character string for device serial\n"
								"address: hex number representing 16 bit register number\n"
								"numWords: number of words to read\n";

static std::tuple<std::string_view, uint16_t, uint16_t> parseArgs(std::vector<std::string_view>& args, bool& fail) {

	if(args.size() != 4) {
		std::cerr << "Invalid argument count" << std::endl;
		std::cerr << usage;
		fail = true;
		return {};
	}

	std::string_view serial = args[1];
	if(serial.size() != 6) {
		std::cerr << "Invalid serial length" << std::endl;
		std::cerr << usage;
		fail = true;
		return {};
	}

	char* endPtr;
	uint16_t address = static_cast<uint16_t>(std::strtoul(args[2].data(), &endPtr, 16));
	if(endPtr != (args[2].data() + args[2].size())) {
		std::cerr << "Failed to convert address to hex" << std::endl;
		std::cerr << usage;
		fail = true;
		return {};
	}

	uint16_t numWords = static_cast<uint16_t>(std::strtoul(args[3].data(), &endPtr, 16));
	if(endPtr != (args[3].data() + args[3].size())) {
		std::cerr << "Failed to convert numWords to decimal integer" << std::endl;
		std::cerr << usage;
		fail = true;
		return {};
	}
	fail = false;
	return {serial, address, numWords};
}

int main(int argc, const char** argv) {
	std::vector<std::string_view> args(argv, argv + argc);
	bool fail;
	auto tup = parseArgs(args, fail);
	if(fail) {
		return -1;
	}	
	auto serial = std::get<0>(tup);
	auto address = std::get<1>(tup);
	auto numWords = std::get<2>(tup);

	std::shared_ptr<icsneo::Device> device = nullptr;
	for(const auto& dev : icsneo::FindAllDevices()) {
		if(dev->getSerial() == serial) {
			device = dev;
			break;
		}
	}
	if(!device) {
		std::cerr << "Failed to find device" << std::endl;
		std::cerr << usage;
		return -1;
	}
	if(!device->open()) {
		std::cerr << "Failed to open device" << std::endl;
		return -1;
	}
	if(!device->goOnline()) {
		std::cerr << "Failed to go online" << std::endl;
		return -1;
	}

	std::cout << "Reading register " << std::setfill('0') << std::setw(4) << std::right << std::hex << address << std::endl;

	// Make message
	std::shared_ptr<icsneo::SPIMessage> msg = std::make_shared<icsneo::SPIMessage>();
	msg->network = icsneo::Network::NetID::SPI_01; 
	msg->direction = icsneo::SPIMessage::Direction::Read;
	msg->address = address;
	msg->payload.resize(numWords, 0); // Resize payload to desired length

	// Add callback
	bool receivedMessage = false;
	auto handle = device->addMessageCallback(std::make_shared<icsneo::MessageCallback>([&](std::shared_ptr<icsneo::Message> msg) {
		if(receivedMessage) {
			return;
		}
		if(msg->type == icsneo::Message::Type::Frame) {
			auto frame = std::dynamic_pointer_cast<icsneo::Frame>(msg);
			if(!frame) {
				return;
			}
			if(frame->network == icsneo::Network::NetID::SPI_01) {
				auto spiMsg = std::dynamic_pointer_cast<icsneo::SPIMessage>(frame);
				if(spiMsg->address == address) {
					std::cout << "Received " << spiMsg->payload.size() << " words" << std::endl;
					for(uint32_t word : spiMsg->payload) {
						std::cout << std::setfill('0') << std::setw(8) << std::right << std::hex << word << ' ';
					}
					std::cout << std::endl;
					receivedMessage = true;
				}
			}
		}
	}));
	device->transmit(msg);
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	if(!receivedMessage) {
		std::cout << "Did not receive response from device." << std::endl;
	}
	device->removeMessageCallback(handle);
	return 0;
}
