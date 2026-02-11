/**
 * libicsneo Analog Output example
 * 
 * Demonstrates how to configure and control analog outputs on supported devices
 * 
 * Usage: libicsneo-analog-out <pin> <voltage> [deviceSerial] [--yes]
 * 
 * Arguments:
 * pin: Pin number (1-3 for RAD Galaxy)
 * voltage: Voltage level (0-5)
 * deviceSerial: 6 character string for device serial (optional)
 * --yes: Skip confirmation prompt
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <string_view>
#include <cstdlib>

#include "icsneo/icsneocpp.h"

static const std::string usage = "Usage: libicsneo-analog-out <pin> <voltage> [deviceSerial] [--yes]\n\n"
								"Arguments:\n"
								"pin: Pin number (1-3 for RAD Galaxy)\n"
								"voltage: Voltage level (0-5)\n"
								"deviceSerial: 6 character string for device serial (optional)\n"
								"--yes: Skip confirmation prompt\n";

int main(int argc, const char** argv) {
	std::vector<std::string_view> args(argv, argv + argc);
	
	// Parse arguments
	if(args.size() < 3) {
		std::cerr << "Error: Missing required arguments\n" << std::endl;
		std::cerr << usage;
		return -1;
	}
	
	char* endPtr;
	long pinNum = std::strtol(args[1].data(), &endPtr, 10);
	if(endPtr != args[1].data() + args[1].size() || pinNum < 1 || pinNum > 3) {
		std::cerr << "Error: Invalid pin number (must be 1-3)" << std::endl;
		return -1;
	}
	
	long voltageLevel = std::strtol(args[2].data(), &endPtr, 10);
	if(endPtr != args[2].data() + args[2].size() || voltageLevel < 0 || voltageLevel > 5) {
		std::cerr << "Error: Invalid voltage level (must be 0-5)" << std::endl;
		return -1;
	}
	
	icsneo::MiscIOAnalogVoltage voltage = static_cast<icsneo::MiscIOAnalogVoltage>(voltageLevel);
	uint8_t pin = static_cast<uint8_t>(pinNum);
	
	// Check for optional arguments
	bool skipConfirm = false;
	std::string_view serial;
	for(size_t i = 3; i < args.size(); i++) {
		if(args[i] == "--yes") {
			skipConfirm = true;
		} else if(serial.empty() && args[i].size() == 6) {
			serial = args[i];
		}
	}
	
	// Confirmation prompt
	if(!skipConfirm) {
		std::cout << "WARNING: This will set analog output pin " << static_cast<int>(pin) 
		          << " to " << voltageLevel << "V" << std::endl;
		std::cout << "Make sure nothing sensitive is connected to this pin." << std::endl;
		std::cout << "Continue? (yes/no): ";
		std::string response;
		std::getline(std::cin, response);
		if(response != "yes") {
			std::cout << "Aborted." << std::endl;
			return 0;
		}
	}
	
	std::shared_ptr<icsneo::Device> device = nullptr;
	
	if(!serial.empty()) {
		// Find device by serial
		auto devices = icsneo::FindAllDevices();
		for(auto& dev : devices) {
			if(dev->getSerial() == serial) {
				device = dev;
				break;
			}
		}
		
		if(!device) {
			std::cerr << "Device with serial " << serial << " not found" << std::endl;
			return -1;
		}
	} else {
		// Use first available device
		auto devices = icsneo::FindAllDevices();
		if(devices.empty()) {
			std::cerr << "No devices found" << std::endl;
			return -1;
		}
		device = devices[0];
	}
	
	std::cout << "Using device: " << device->describe() << std::endl;
	
	if(!device->open()) {
		std::cerr << "Failed to open device" << std::endl;
		return -1;
	}
	
	auto settings = device->settings;
	if(!settings) {
		std::cerr << "Device settings not available" << std::endl;
		device->close();
		return -1;
	}
	
	std::cout << "Refreshing device settings..." << std::endl;
	if(!settings->refresh()) {
		std::cerr << "Failed to refresh settings" << std::endl;
		device->close();
		return -1;
	}
	
	// Enable analog output on specified pin
	std::cout << "Enabling analog output on pin " << static_cast<int>(pin) << "..." << std::endl;
	if(!settings->setMiscIOAnalogOutputEnabled(pin, true)) {
		std::cerr << "Failed to enable analog output on pin " << static_cast<int>(pin) << std::endl;
		device->close();
		return -1;
	}
	
	// Set pin to specified voltage
	std::cout << "Setting pin " << static_cast<int>(pin) << " to " << voltageLevel << "V..." << std::endl;
	if(!settings->setMiscIOAnalogOutput(pin, voltage)) {
		std::cerr << "Failed to set voltage on pin " << static_cast<int>(pin) << std::endl;
		device->close();
		return -1;
	}
	
	// Apply settings
	std::cout << "Applying settings..." << std::endl;
	if(!settings->apply()) {
		std::cerr << "Failed to apply settings" << std::endl;
		device->close();
		return -1;
	}
	
	std::cout << "Analog output configured successfully!" << std::endl;
	std::cout << "Pin " << static_cast<int>(pin) << ": Enabled at " << voltageLevel << "V" << std::endl;

	device->close();
	return 0;
}
