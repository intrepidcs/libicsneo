// 10BASE-T1S Symbol Decoding Example
// Demonstrates T1S bus symbol decoding and analysis

#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <map>
#include <string>
#include <atomic>

#include "icsneo/icsneocpp.h"

enum class T1SSymbol : uint8_t {
	SSD = 0x04,
	ESDOK = 0x07,
	BEACON = 0x08,
	ESD = 0x0D,
	ESDERR = 0x11,
	SYNC = 0x18,
	ESDJAB = 0x19,
	SILENCE = 0x1F
};

std::string getSymbolName(uint8_t value) {
	switch(static_cast<T1SSymbol>(value)) {
		case T1SSymbol::SSD:      return "SSD";
		case T1SSymbol::ESDOK:    return "ESDOK";
		case T1SSymbol::BEACON:   return "BEACON";
		case T1SSymbol::ESD:      return "ESD";
		case T1SSymbol::ESDERR:   return "ESDERR";
		case T1SSymbol::SYNC:     return "SYNC";
		case T1SSymbol::ESDJAB:   return "ESDJAB";
		case T1SSymbol::SILENCE:  return "SILENCE";
		default:
			if (value <= 0x0F) {
				std::stringstream ss;
				ss << "DATA(0x" << std::hex << std::uppercase << (int)value << ")";
				return ss.str();
			}
			std::stringstream ss;
			ss << "UNKNOWN(0x" << std::hex << std::uppercase << std::setw(2) 
			   << std::setfill('0') << (int)value << std::setfill(' ') << ")";
			return ss.str();
	}
}

struct T1SStatistics {
	std::atomic<uint64_t> symbolCount{0};
	std::atomic<uint64_t> beaconCount{0};
	std::atomic<uint64_t> wakeCount{0};
	std::atomic<uint64_t> burstCount{0};
	std::atomic<uint64_t> dataFrameCount{0};
	std::map<std::string, uint64_t> symbolStats;
	
	void reset() {
		symbolCount = 0;
		beaconCount = 0;
		wakeCount = 0;
		burstCount = 0;
		dataFrameCount = 0;
		symbolStats.clear();
	}
	
	void print() const {
		std::cout << std::setfill(' ');
		
		std::cout << "\n" << std::string(70, '=') << std::endl;
		std::cout << "T1S SYMBOL DECODING STATISTICS" << std::endl;
		std::cout << std::string(70, '=') << std::endl;
		
		std::cout << "Total Symbols:              " << symbolCount << std::endl;
		std::cout << "Total Beacons:              " << beaconCount << std::endl;
		std::cout << "Total Wake Signals:         " << wakeCount << std::endl;
		std::cout << "Total Bursts:               " << burstCount << std::endl;
		std::cout << "Total Data Frames:          " << dataFrameCount << std::endl;
		
		if (!symbolStats.empty()) {
			std::cout << "\n" << std::string(70, '-') << std::endl;
			std::cout << "Symbol Type Breakdown:" << std::endl;
			std::cout << std::string(70, '-') << std::endl;
			std::vector<std::pair<std::string, uint64_t>> sortedStats(
				symbolStats.begin(), symbolStats.end());
			std::sort(sortedStats.begin(), sortedStats.end(),
				[](const auto& a, const auto& b) { return a.second > b.second; });
			
			for (const auto& [name, count] : sortedStats) {
				std::cout << "  " << std::left << std::setw(20) << name 
				          << std::right << std::setw(10) << count << std::endl;
			}
		}
		std::cout << std::string(70, '=') << std::endl;
	}
};

bool getUserConfirmation(const std::string& prompt) {
	std::string input;
	std::cout << prompt << " (y/n): " << std::flush;
	std::getline(std::cin, input);
	if (!input.empty()) {
		char c = std::tolower(input[0]);
		return (c == 'y');
	}
	return false;
}

bool configureT1SDecoding(std::shared_ptr<icsneo::Device>& device, icsneo::Network::NetID network, 
                          bool enableSymbols, bool enableBeacons) {
	std::cout << "\nConfiguring T1S decoding on network " << icsneo::Network(network) << "..." << std::endl;
	if (!device->settings->setT1SBusDecodingAllFor(network, enableSymbols)) {
		std::cerr << "  ✗ Failed to set T1S symbol decoding" << std::endl;
		return false;
	}
	if (enableSymbols) {
		std::cout << "  ✓ Enabled decoding of all T1S symbols" << std::endl;
	} else {
		std::cout << "  • T1S symbol decoding disabled" << std::endl;
	}
	
	if (!device->settings->setT1SBusDecodingBeaconsFor(network, enableBeacons)) {
		std::cerr << "  ✗ Failed to set T1S beacon decoding" << std::endl;
		return false;
	}
	if (enableBeacons) {
		std::cout << "  ✓ Enabled T1S beacon decoding" << std::endl;
	} else {
		std::cout << "  • T1S beacon decoding disabled" << std::endl;
	}
	
	if (!device->settings->apply(true)) {
		std::cerr << "  ✗ Failed to apply settings to device" << std::endl;
		std::cerr << "  " << icsneo::GetLastError() << std::endl;
		return false;
	}
	std::cout << "  ✓ Settings applied successfully" << std::endl;
	
	return true;
}

void setupSymbolMonitoring(std::shared_ptr<icsneo::Device>& device, 
                          icsneo::Network::NetID network,
                          T1SStatistics& stats) {
	
	auto callback = std::make_shared<icsneo::MessageCallback>(
		icsneo::MessageFilter(network),
		[&stats](std::shared_ptr<icsneo::Message> message) {
			if (message->type != icsneo::Message::Type::Frame)
				return;
			
			auto frame = std::static_pointer_cast<icsneo::Frame>(message);
			auto netType = frame->network.getType();
			if (netType != icsneo::Network::Type::Ethernet && netType != icsneo::Network::Type::AutomotiveEthernet)
				return;
			
			auto ethMsg = std::static_pointer_cast<icsneo::EthernetMessage>(frame);
			
			if (!ethMsg->isT1S)
				return;
			
			double timestamp_ms = ethMsg->timestamp / 1000000.0;
			
			if (ethMsg->isT1SSymbol) {
				size_t numSymbols = ethMsg->data.size();
				
				std::cout << std::fixed << std::setprecision(3)
				          << "[" << std::setw(12) << timestamp_ms << " ms] "
				          << "T1S Symbols";
				
				if (numSymbols > 0) {
					std::cout << " (" << numSymbols << " symbol" << (numSymbols > 1 ? "s" : "") << ")";
				}
				std::cout << " | Node ID: " << (int)ethMsg->t1sNodeId << std::endl;
				
				for (size_t i = 0; i < numSymbols; i++) {
					uint8_t symbolValue = ethMsg->data[i];
					std::string symbolName = getSymbolName(symbolValue);
					
					stats.symbolCount++;
					stats.symbolStats[symbolName]++;
					
					if (symbolValue == static_cast<uint8_t>(T1SSymbol::BEACON)) {
						stats.beaconCount++;
					}
					
					std::cout << "  [" << i << "] " << std::left << std::setw(10) << symbolName << std::right
					          << " = 0x" << std::hex << std::uppercase << std::setw(2) 
					          << std::setfill('0') << (int)symbolValue << std::setfill(' ')
					          << std::dec << std::endl;
				}
				
				if (numSymbols == 0 && ethMsg->t1sSymbolType != 0) {
					uint8_t symbolValue = ethMsg->t1sSymbolType;
					std::string symbolName = getSymbolName(symbolValue);
					
					stats.symbolCount++;
					stats.symbolStats[symbolName]++;
					
					if (symbolValue == static_cast<uint8_t>(T1SSymbol::BEACON)) {
						stats.beaconCount++;
					}
					
					std::cout << "  " << std::left << std::setw(10) << symbolName << std::right
					          << " = 0x" << std::hex << std::uppercase << std::setw(2) 
					          << std::setfill('0') << (int)symbolValue << std::setfill(' ')
					          << std::dec << " (from t1sSymbolType field)" << std::endl;
				}
			}
			else if (ethMsg->isT1SBurst) {
				stats.burstCount++;
				std::cout << std::fixed << std::setprecision(3)
				          << "[" << std::setw(12) << timestamp_ms << " ms] "
				          << "BURST | "
				          << "Node ID: " << (int)ethMsg->t1sNodeId << " | "
				          << "Burst Count: " << (int)ethMsg->t1sBurstCount << std::endl;
			}
			else if (ethMsg->isT1SWake) {
				stats.wakeCount++;
				std::cout << std::fixed << std::setprecision(3)
				          << "[" << std::setw(12) << timestamp_ms << " ms] "
				          << "WAKE signal detected | "
				          << "Node ID: " << (int)ethMsg->t1sNodeId << std::endl;
			}
			else {
				stats.dataFrameCount++;
				std::cout << std::fixed << std::setprecision(3)
				          << "[" << std::setw(12) << timestamp_ms << " ms] "
				          << "T1S Data Frame | "
				          << "Length: " << ethMsg->data.size() << " bytes | "
				          << "Node ID: " << (int)ethMsg->t1sNodeId << std::endl;
				
				if (!ethMsg->data.empty()) {
					std::cout << "                    Data: ";
					size_t preview_len = std::min(ethMsg->data.size(), size_t(16));
					for (size_t i = 0; i < preview_len; i++) {
						std::cout << std::hex << std::uppercase << std::setw(2) 
						          << std::setfill('0') << (int)ethMsg->data[i] << " ";
					}
					if (ethMsg->data.size() > 16)
						std::cout << "...";
					std::cout << std::setfill(' ') << std::dec << std::endl;
				}
			}
		}
	);
	
	device->addMessageCallback(callback);
}

int main() {
	const icsneo::Network::NetID MONITOR_NETWORK = icsneo::Network::NetID::AE_02;
	const int MONITOR_DURATION_SECONDS = 30;
	
	std::cout << "\n" << std::string(70, '=') << std::endl;
	std::cout << "10BASE-T1S SYMBOL DECODING EXAMPLE" << std::endl;
	std::cout << std::string(70, '=') << std::endl;
	std::cout << "libicsneo " << icsneo::GetVersion() << std::endl;
	std::cout << std::string(70, '=') << std::endl;
	
	std::cout << "\nFinding devices... " << std::flush;
	auto devices = icsneo::FindAllDevices();
	std::cout << "OK, " << devices.size() << " device" << (devices.size() == 1 ? "" : "s") << " found" << std::endl;
	
	if (devices.empty()) {
		std::cerr << "No devices found!" << std::endl;
		return 1;
	}
	
	// List devices
	for (const auto& device : devices) {
		std::cout << "  " << device->describe() << std::endl;
	}
	
	std::shared_ptr<icsneo::Device> device;
	for (auto& dev : devices) {
		if (dev->getType() == icsneo::DeviceType::RADComet3) {
			device = dev;
			break;
		}
	}
	
	if (!device && !devices.empty())
		device = devices[0];
	
	if (!device) {
		std::cerr << "No suitable device found!" << std::endl;
		return 1;
	}
	
	std::cout << "\nSelected device: " << device->describe() << std::endl;
	std::cout << "Serial: " << device->getSerial() << std::endl;
	
	std::cout << "\n" << std::string(70, '-') << std::endl;
	std::cout << "T1S DECODING CONFIGURATION" << std::endl;
	std::cout << std::string(70, '-') << std::endl;
	bool enableSymbols = getUserConfirmation("Enable T1S symbol decoding (all symbols)");
	bool enableBeacons = getUserConfirmation("Enable T1S beacon decoding");
	std::cout << std::string(70, '-') << std::endl;
	
	std::cout << "\nOpening device... " << std::flush;
	if (!device->open()) {
		std::cerr << "✗ Failed" << std::endl;
		std::cerr << "  " << icsneo::GetLastError() << std::endl;
		return 1;
	}
	std::cout << "✓" << std::endl;
	
	std::cout << "Enabling message polling... " << std::flush;
	if (!device->enableMessagePolling()) {
		std::cerr << "✗ Failed" << std::endl;
		std::cerr << "  " << icsneo::GetLastError() << std::endl;
		device->close();
		return 1;
	}
	device->setPollingMessageLimit(100000);
	std::cout << "✓" << std::endl;
	
	if (!configureT1SDecoding(device, MONITOR_NETWORK, enableSymbols, enableBeacons)) {
		device->close();
		return 1;
	}
	
	std::cout << "Going online... " << std::flush;
	if (!device->goOnline()) {
		std::cerr << "✗ Failed" << std::endl;
		std::cerr << "  " << icsneo::GetLastError() << std::endl;
		device->close();
		return 1;
	}
	std::cout << "✓" << std::endl;
	
	T1SStatistics stats;
	setupSymbolMonitoring(device, MONITOR_NETWORK, stats);
	
	std::cout << "\n" << std::string(70, '-') << std::endl;
	std::cout << "Monitoring T1S traffic for " << MONITOR_DURATION_SECONDS << " seconds..." << std::endl;
	std::cout << std::string(70, '-') << std::endl;
	
	auto startTime = std::chrono::steady_clock::now();
	std::vector<std::shared_ptr<icsneo::Message>> messages;
	messages.reserve(10000);
	
	while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(MONITOR_DURATION_SECONDS)) {
		device->getMessages(messages);
		messages.clear();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	std::cout << "\n" << std::string(70, '-') << std::endl;
	std::cout << "Closing device... " << std::flush;
	device->close();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	std::cout << "✓" << std::endl;
	
	stats.print();
	
	return 0;
}
