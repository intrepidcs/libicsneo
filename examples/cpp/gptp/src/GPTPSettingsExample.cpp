#include <iostream>
#include <iomanip>
#include <vector>
#include <optional>
#include <string>
#include <limits>

#include "icsneo/icsneocpp.h"

/* Maps a Network::NetID to its gPTP port index (0 = not a gPTP port). */
static uint8_t netidToGPTPPort(icsneo::Network::NetID netid) {
	using N = icsneo::Network::NetID;
	switch(netid) {
		case N::AE_01:       return 1;
		case N::AE_02:       return 2;
		case N::AE_03:       return 3;
		case N::AE_04:       return 4;
		case N::AE_05:       return 5;
		case N::AE_06:       return 6;
		case N::AE_07:       return 7;
		case N::AE_08:       return 8;
		case N::AE_09:       return 9;
		case N::AE_10:       return 10;
		case N::AE_11:       return 11;
		case N::AE_12:       return 12;
		case N::ETHERNET_01: return 13;
		case N::ETHERNET_02: return 14;
		case N::ETHERNET_03: return 15;
		case N::AE_13:       return 16;
		case N::AE_14:       return 17;
		case N::AE_15:       return 18;
		case N::AE_16:       return 19;
		default:             return 0;
	}
}

static std::string gptpProfileStr(RADGPTPProfile p) {
	switch(p) {
		case RAD_GPTP_PROFILE_STANDARD:   return "Standard";
		case RAD_GPTP_PROFILE_AUTOMOTIVE: return "Automotive";
		default:                          return "Unknown";
	}
}

static std::string gptpRoleStr(RADGPTPRole r) {
	switch(r) {
		case RAD_GPTP_ROLE_DISABLED: return "Disabled";
		case RAD_GPTP_ROLE_PASSIVE:  return "Passive";
		case RAD_GPTP_ROLE_MASTER:   return "Master";
		case RAD_GPTP_ROLE_SLAVE:    return "Slave";
		default:                     return "Unknown";
	}
}

template<typename T>
static std::string optStr(const std::optional<T>& opt) {
	if(!opt.has_value()) return "N/A";
	if constexpr(std::is_same_v<T, bool>)
		return opt.value() ? "enabled" : "disabled";
	else
		return std::to_string(static_cast<int>(opt.value()));
}

static long promptLong(const std::string& prompt, long defaultVal, long min, long max) {
	std::cout << prompt << " [" << defaultVal << "]: " << std::flush;
	std::string input;
	std::getline(std::cin, input);
	if(input.empty())
		return defaultVal;
	try {
		long val = std::stol(input);
		if(val >= min && val <= max)
			return val;
	} catch(...) {}
	std::cout << "Invalid input, using " << defaultVal << ".\n";
	return defaultVal;
}

static void printGPTPPorts(const std::shared_ptr<icsneo::Device>& device) {
	for(const auto& net : device->getSupportedTXNetworks()) {
		uint8_t port = netidToGPTPPort(net.getNetID());
		if(port == 0) continue;
		std::cout << "    [" << (int)port << "] " << net << "\n";
	}
}

static void printCurrentSettings(const std::shared_ptr<icsneo::Device>& device) {
	std::cout << "\nCurrent gPTP settings:\n";
	auto profile = device->settings->getGPTPProfile();
	auto role    = device->settings->getGPTPRole();
	auto port    = device->settings->getGPTPEnabledPort();
	auto synton  = device->settings->isGPTPClockSyntonizationEnabled();

	if(profile.has_value())
		std::cout << "  Profile:             " << gptpProfileStr(profile.value()) << "\n";
	if(role.has_value())
		std::cout << "  Role:                " << gptpRoleStr(role.value()) << "\n";
	if(port.has_value())
		std::cout << "  Enabled port:        " << (int)port.value() << (port.value() == 0 ? " (disabled)" : "") << "\n";
	if(synton.has_value())
		std::cout << "  Clock syntonization: " << optStr(synton) << "\n";
}

int main() {
	std::cout << "Finding devices... " << std::flush;
	auto devices = icsneo::FindAllDevices();
	std::cout << devices.size() << " found\n";

	if(devices.empty()) {
		auto err = icsneo::GetLastError();
		if(err.getType() != icsneo::APIEvent::Type::NoErrorFound)
			std::cout << err << "\n";
		return 1;
	}

	std::vector<std::shared_ptr<icsneo::Device>> gptp_devices;
	for(auto& d : devices) {
		if(d->supportsGPTP())
			gptp_devices.push_back(d);
	}

	if(gptp_devices.empty()) {
		std::cout << "No gPTP-capable devices found.\n";
		return 1;
	}

	std::cout << "\nAvailable gPTP-capable devices:\n";
	for(size_t i = 0; i < gptp_devices.size(); ++i)
		std::cout << "  [" << (i + 1) << "] " << gptp_devices[i]->describe() << "\n";

	long choice = promptLong("Select device", 1, 1, (long)gptp_devices.size());
	auto device = gptp_devices[choice - 1];

	std::cout << "\nOpening " << device->describe() << "... " << std::flush;
	if(!device->open()) {
		std::cout << "failed\n" << icsneo::GetLastError() << "\n";
		return 1;
	}
	std::cout << "OK\n";

	if(!device->settings->refresh()) {
		std::cout << "Failed to refresh settings\n";
		device->close();
		return 1;
	}

	printCurrentSettings(device);

	std::cout << "\nAvailable gPTP ports (0 = disabled):\n";
	std::cout << "    [0] Disabled\n";
	printGPTPPorts(device);

	std::cout << "\nConfigure gPTP:\n";

	long profile = promptLong("  Profile (0=Standard, 1=Automotive)", 1, 0, 1);
	device->settings->setGPTPProfile(static_cast<RADGPTPProfile>(profile));

	long role = promptLong("  Role (0=Disabled, 1=Passive, 2=Master, 3=Slave)", 3, 0, 3);
	device->settings->setGPTPRole(static_cast<RADGPTPRole>(role));

	long port = promptLong("  Enabled port index", 0, 0, 19);
	device->settings->setGPTPEnabledPort(static_cast<uint8_t>(port));

	long syntonization = promptLong("  Clock syntonization (0=disabled, 1=enabled)", 0, 0, 1);
	device->settings->setGPTPClockSyntonizationEnabled(syntonization != 0);

	long permanent = promptLong("\nSave to EEPROM? (0=temporary, 1=permanent)", 0, 0, 1);
	if(!device->settings->apply(permanent == 0)) {
		std::cout << "Failed to apply settings\n" << icsneo::GetLastError() << "\n";
		device->close();
		return 1;
	}

	printCurrentSettings(device);

	std::cout << "\nClosing " << device->describe() << "\n";
	device->close();
	return 0;
}
