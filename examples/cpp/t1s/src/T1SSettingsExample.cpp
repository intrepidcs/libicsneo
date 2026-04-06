#include <iostream>
#include <iomanip>
#include <vector>
#include <optional>
#include <string>
#include <limits>
#include <sstream>
#include <algorithm>

#include "icsneo/icsneocpp.h"

template<typename T>
std::string optToString(const std::optional<T>& opt) {
	if (!opt.has_value())
		return "N/A";
	if constexpr (std::is_same_v<T, bool>)
		return opt.value() ? "true" : "false";
	else
		return std::to_string(opt.value());
}

bool getUserConfirmation(const std::string& prompt) {
	std::string input;
	std::cout << prompt << " (y/n): " << std::flush;
	std::getline(std::cin, input);
	if (!input.empty()) {
		char c = static_cast<char>(std::tolower(input[0]));
		return (c == 'y');
	}
	return false;
}

std::vector<icsneo::Network::NetID> selectNetworks(const std::vector<icsneo::Network::NetID>& availableNetworks) {
	std::vector<icsneo::Network::NetID> selectedNetworks;
	
	std::cout << "\n" << std::string(70, '=') << std::endl;
	std::cout << "Select T1S Networks to Configure" << std::endl;
	std::cout << std::string(70, '=') << std::endl;
	
	for (size_t i = 0; i < availableNetworks.size(); i++) {
		std::cout << "  [" << (i + 1) << "] " << icsneo::Network(availableNetworks[i]) << std::endl;
	}
	
	std::cout << "\nEnter network numbers to configure (e.g., '1,3' or '1-3' or 'all'): " << std::flush;
	std::string input;
	std::getline(std::cin, input);
	
	if (input.empty())
		return selectedNetworks;
	
	std::transform(input.begin(), input.end(), input.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	
	if (input == "all") {
		return availableNetworks;
	}
	
	std::stringstream ss(input);
	std::string token;
	while (std::getline(ss, token, ',')) {
		token.erase(0, token.find_first_not_of(" \t"));
		token.erase(token.find_last_not_of(" \t") + 1);
		
		size_t dashPos = token.find('-');
		if (dashPos != std::string::npos) {
			try {
				int start = std::stoi(token.substr(0, dashPos));
				int end = std::stoi(token.substr(dashPos + 1));
				for (int i = start; i <= end; i++) {
					if (i >= 1 && i <= (int)availableNetworks.size()) {
						selectedNetworks.push_back(availableNetworks[i - 1]);
					}
				}
			} catch (...) {}
		} else {
			try {
				int num = std::stoi(token);
				if (num >= 1 && num <= (int)availableNetworks.size()) {
					selectedNetworks.push_back(availableNetworks[num - 1]);
				}
			} catch (...) {}
		}
	}
	
	return selectedNetworks;
}

uint8_t getUint8Input(const std::string& prompt, uint8_t defaultValue) {
	std::string input;
	std::cout << prompt << " [" << (int)defaultValue << "]: " << std::flush;
	std::getline(std::cin, input);
	if (input.empty())
		return defaultValue;
	try {
		int val = std::stoi(input);
		if (val >= 0 && val <= 255)
			return static_cast<uint8_t>(val);
	} catch (...) {}
	return defaultValue;
}

uint16_t getUint16Input(const std::string& prompt, uint16_t defaultValue) {
	std::string input;
	std::cout << prompt << " [" << (int)defaultValue << "]: " << std::flush;
	std::getline(std::cin, input);
	if (input.empty())
		return defaultValue;
	try {
		int val = std::stoi(input);
		if (val >= 0 && val <= 65535)
			return static_cast<uint16_t>(val);
	} catch (...) {}
	return defaultValue;
}

void displayT1SSettings(const std::shared_ptr<icsneo::Device>& device, icsneo::Network::NetID netId) {
	std::cout << "\t" << icsneo::Network(netId) << " T1S Settings:" << std::endl;
	
	std::cout << "\t  PLCA Enabled:       " << optToString(device->settings->isT1SPLCAEnabledFor(netId)) << std::endl;
	std::cout << "\t  Local ID:           " << optToString(device->settings->getT1SLocalIDFor(netId)) << std::endl;
	std::cout << "\t  Max Nodes:          " << optToString(device->settings->getT1SMaxNodesFor(netId)) << std::endl;
	std::cout << "\t  TX Opp Timer:       " << optToString(device->settings->getT1STxOppTimerFor(netId)) << std::endl;
	std::cout << "\t  Max Burst:          " << optToString(device->settings->getT1SMaxBurstFor(netId)) << std::endl;
	std::cout << "\t  Burst Timer:        " << optToString(device->settings->getT1SBurstTimerFor(netId)) << std::endl;
	
	auto termEnabled = device->settings->isT1STerminationEnabledFor(netId);
	if (termEnabled.has_value())
		std::cout << "\t  Termination:        " << optToString(termEnabled) << std::endl;
	
	auto localIdAlt = device->settings->getT1SLocalIDAlternateFor(netId);
	if (localIdAlt.has_value()) {
		std::cout << "\t  Local ID Alternate: " << optToString(localIdAlt) << std::endl;
		std::cout << "\t  Bus Dec Beacons:    " << optToString(device->settings->isT1SBusDecodingBeaconsEnabledFor(netId)) << std::endl;
		std::cout << "\t  Bus Dec All:        " << optToString(device->settings->isT1SBusDecodingAllEnabledFor(netId)) << std::endl;
		
		auto multiIdMask = device->settings->getT1SMultiIDEnableMaskFor(netId);
		if (multiIdMask.has_value()) {
			std::cout << "\t  Multi-ID Mask:      0x" << std::hex << std::setw(2) << std::setfill('0') 
			          << (int)multiIdMask.value() << std::dec << std::endl;
			std::cout << "\t  Multi-IDs:          ";
			for (uint8_t i = 0; i < 7; i++) {
				if (i > 0) std::cout << ", ";
				auto multiId = device->settings->getT1SMultiIDFor(netId, i);
				std::cout << "[" << (int)i << "]=" << optToString(multiId);
			}
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}

void configureT1SNetwork(std::shared_ptr<icsneo::Device>& device, icsneo::Network::NetID netId) {
	std::cout << "\n" << std::string(70, '=') << std::endl;
	std::cout << "Configuring T1S Network: " << icsneo::Network(netId) << std::endl;
	std::cout << std::string(70, '=') << std::endl;
	
	std::cout << "\n--- Basic PLCA Settings ---" << std::endl;
	bool plcaEnabled = getUserConfirmation("Enable PLCA");
	device->settings->setT1SPLCAFor(netId, plcaEnabled);
	
	uint8_t localId = getUint8Input("Local ID (0-255)", 1);
	device->settings->setT1SLocalIDFor(netId, localId);
	
	uint8_t maxNodes = getUint8Input("Max Nodes (0-255)", 8);
	device->settings->setT1SMaxNodesFor(netId, maxNodes);
	
	uint8_t txOppTimer = getUint8Input("TX Opportunity Timer (0-255)", 20);
	device->settings->setT1STxOppTimerFor(netId, txOppTimer);
	
	uint8_t maxBurst = getUint8Input("Max Burst (0-255)", 128);
	device->settings->setT1SMaxBurstFor(netId, maxBurst);
	
	uint8_t burstTimer = getUint8Input("Burst Timer (0-255)", 64);
	device->settings->setT1SBurstTimerFor(netId, burstTimer);
	
	if (device->settings->isT1STerminationEnabledFor(netId).has_value()) {
		std::cout << "\n--- Termination Settings ---" << std::endl;
		bool termEnabled = getUserConfirmation("Enable Termination");
		device->settings->setT1STerminationFor(netId, termEnabled);
	}
	
	if (device->settings->getT1SLocalIDAlternateFor(netId).has_value()) {
		std::cout << "\n--- Extended Settings ---" << std::endl;
		uint8_t localIdAlt = getUint8Input("Local ID Alternate (0-255)", 0);
		device->settings->setT1SLocalIDAlternateFor(netId, localIdAlt);
		
		bool busDecBeacons = getUserConfirmation("Enable Bus Decoding (Beacons)");
		device->settings->setT1SBusDecodingBeaconsFor(netId, busDecBeacons);
		
		bool busDecAll = getUserConfirmation("Enable Bus Decoding (All Symbols)");
		device->settings->setT1SBusDecodingAllFor(netId, busDecAll);
		
		if (getUserConfirmation("Configure Multi-ID settings?")) {
			uint8_t multiIdMask = getUint8Input("Multi-ID Enable Mask (0x00-0xFF, hex)", 0x00);
			device->settings->setT1SMultiIDEnableMaskFor(netId, multiIdMask);
			
			std::cout << "Configure Multi-IDs (7 slots):" << std::endl;
			for (uint8_t i = 0; i < 7; i++) {
				uint8_t multiId = getUint8Input("  Multi-ID [" + std::to_string(i) + "]", 0);
				device->settings->setT1SMultiIDFor(netId, i, multiId);
			}
		}
	}
	
	std::cout << "\n✓ Configuration complete for " << icsneo::Network(netId) << std::endl;
}

int main() {
	std::cout << "\n" << std::string(70, '=') << std::endl;
	std::cout << "10BASE-T1S SETTINGS CONFIGURATION EXAMPLE" << std::endl;
	std::cout << std::string(70, '=') << std::endl;
	std::cout << "libicsneo " << icsneo::GetVersion() << std::endl;
	std::cout << std::string(70, '=') << std::endl;
	
	std::cout << "\nFinding devices... " << std::flush;
	auto devices = icsneo::FindAllDevices();
	std::cout << "OK, " << devices.size() << " device" << (devices.size() == 1 ? "" : "s") << " found" << std::endl;
	
	if (devices.empty()) {
		std::cout << "No devices found!" << std::endl;
		return 1;
	}
	
	for(auto& device : devices)
		std::cout << "  " << device->describe() << std::endl;
	
	std::shared_ptr<icsneo::Device> selectedDevice;
	for(auto& device : devices) {
		if (device->getType() == icsneo::DeviceType::RADComet3) {
			selectedDevice = device;
			break;
		}
	}
	
	if (!selectedDevice && !devices.empty())
		selectedDevice = devices[0];
	
	if (!selectedDevice) {
		std::cout << "No suitable device found!" << std::endl;
		return 1;
	}
	
	std::cout << "\nSelected device: " << selectedDevice->describe() << std::endl;
	std::cout << "Serial: " << selectedDevice->getSerial() << std::endl;
	
	std::cout << "\nOpening device... " << std::flush;
	if (!selectedDevice->open()) {
		std::cout << "✗ Failed" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		return 1;
	}
	std::cout << "✓" << std::endl;
	
	std::vector<icsneo::Network::NetID> candidateNetworks = {
		icsneo::Network::NetID::AE_01, icsneo::Network::NetID::AE_02,
		icsneo::Network::NetID::AE_03, icsneo::Network::NetID::AE_04,
		icsneo::Network::NetID::AE_05, icsneo::Network::NetID::AE_06,
		icsneo::Network::NetID::AE_07, icsneo::Network::NetID::AE_08,
		icsneo::Network::NetID::AE_09, icsneo::Network::NetID::AE_10
	};
	
	std::vector<icsneo::Network::NetID> t1sNetworks;
	for (auto netId : candidateNetworks) {
		auto localId = selectedDevice->settings->getT1SLocalIDFor(netId);
		if (localId.has_value())
			t1sNetworks.push_back(netId);
	}
	
	if (t1sNetworks.empty()) {
		std::cout << "No T1S networks found on this device" << std::endl;
		selectedDevice->close();
		return 1;
	}
	
	std::cout << "\nFound " << t1sNetworks.size() << " T1S network" 
	          << (t1sNetworks.size() == 1 ? "" : "s") << ":" << std::endl;
	for (size_t i = 0; i < t1sNetworks.size(); i++)
		std::cout << "  [" << (i + 1) << "] " << icsneo::Network(t1sNetworks[i]) << std::endl;
	
	std::cout << "\n" << std::string(70, '-') << std::endl;
	std::cout << "Current T1S Settings:" << std::endl;
	std::cout << std::string(70, '-') << std::endl;
	for (auto netId : t1sNetworks)
		displayT1SSettings(selectedDevice, netId);
	
	auto networksToConfig = selectNetworks(t1sNetworks);
	
	if (networksToConfig.empty()) {
		std::cout << "\nNo networks selected for configuration." << std::endl;
		std::cout << "Closing device... " << std::flush;
		selectedDevice->close();
		std::cout << "✓" << std::endl;
		return 0;
	}
	
	std::cout << "\nConfiguring " << networksToConfig.size() << " network"
	          << (networksToConfig.size() == 1 ? "" : "s") << "..." << std::endl;
	
	for (auto netId : networksToConfig)
		configureT1SNetwork(selectedDevice, netId);
	
	std::cout << "\n" << std::string(70, '=') << std::endl;
	bool saveToEEPROM = getUserConfirmation("Save settings to EEPROM (permanent)?");
	std::cout << std::string(70, '=') << std::endl;
	
	std::cout << "\nApplying settings" << (saveToEEPROM ? " to EEPROM" : " temporarily") << "... " << std::flush;
	bool success = selectedDevice->settings->apply(!saveToEEPROM);
	if (!success) {
		std::cout << "✗ Failed" << std::endl;
		std::cout << icsneo::GetLastError() << std::endl;
		selectedDevice->close();
		return 1;
	}
	std::cout << "✓" << std::endl;
	
	std::cout << "\n" << std::string(70, '-') << std::endl;
	std::cout << "Updated T1S Settings:" << std::endl;
	std::cout << std::string(70, '-') << std::endl;
	for (auto netId : t1sNetworks)
		displayT1SSettings(selectedDevice, netId);
	
	std::cout << "Closing device... " << std::flush;
	selectedDevice->close();
	std::cout << "✓" << std::endl;

	return 0;
}
