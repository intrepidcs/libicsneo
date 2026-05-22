#include <iostream>
#include <iomanip>
#include <sstream>

#include "icsneo/icsneocpp.h"

static std::string formatMAC(const std::array<uint8_t, icsneo::MACAddressLength> addr) {
	std::ostringstream oss;
	for(size_t i = 0; i < icsneo::MACAddressLength; i++) {
		if(i > 0) {
			oss << ':';
		}
		oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned>(addr[i]);
	}
	return oss.str();
}

int main() {
	// ===== Device Selection =====
	std::cout << "Searching for devices...\n";
	auto devices = icsneo::FindAllDevices();

	if(devices.empty()) {
		std::cout << "No devices found.\n";
		return 1;
	}

	for(size_t i = 0; i < devices.size(); i++) {
		std::cout << "  [" << (i + 1) << "] " << devices[i]->describe() << "\n";
	}

	int choice;
	std::cout << "Select device (1-" << devices.size() << "): ";
	if(!(std::cin >> choice) || choice < 1 || choice > (int)devices.size()) {
		std::cout << "Invalid selection.\n";
		return 1;
	}

	auto& device = devices[choice - 1];

	std::cout << "\nOpening " << device->describe() << "... ";
	if(!device->open()) {
		std::cout << "FAIL\n" << icsneo::GetLastError() << "\n";
		return 1;
	}
	std::cout << "OK\n\n";

	// ===== Serial Number =====
	std::cout << "Serial:     " << device->getSerial() << "\n";

	// ===== PCB Serial Number =====
	auto pcbSerial = device->getPCBSerial();
	if(pcbSerial.has_value()) {
		std::cout << "PCB Serial: ";
		for(auto b : *pcbSerial)
			std::cout << (char)b;
		std::cout << "\n";
	} else {
		std::cout << "PCB Serial: Not supported\n";
	}

	auto macs = device->getMACAddresses();
	if(!macs.empty()) {
		std::cout << "MACs:       " << macs.size()
		          << (macs.size() == 1 ? " entry" : " entries") << "\n";
		for(auto macPair : macs) {
			std::cout << "  " << std::left << std::setw(20)
			          << icsneo::Network::GetNetIDString(static_cast<icsneo::Network::NetID>(macPair.first))
			          << "  " << formatMAC(macPair.second) << "\n";
		}
	} else {
		std::cout << "MACs:       Not supported\n";
	}

	// ===== Cleanup =====
	std::cout << "\nClosing device... ";
	std::cout << (device->close() ? "OK" : "FAIL") << "\n";

	return 0;
}
