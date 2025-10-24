#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <numeric>
#include <unordered_set>

#include "icsneo/icsneocpp.h"

#ifdef _MSC_VER
#pragma warning(disable : 4996) // STL time functions
#endif

static std::vector<std::shared_ptr<icsneo::Device>> findDevices(icsneo::DeviceType type)
{
	std::vector<std::shared_ptr<icsneo::Device>> ret;
	for(auto&& dev : icsneo::FindAllDevices()) {
		if(dev->getType() == type) {
			ret.push_back(dev);
		}
	}
	return ret;
}

static std::vector<icsneo::Network> supported_nets;
static std::unordered_map<icsneo::Network::NetID, icsneo::NetworkMutexEvent> network_mutex_states;
constexpr uint32_t MUTEX_PRIORITY = 12345678;
constexpr uint32_t MUTEX_TTL_MS = 2500;

static void on_mutex_event(std::shared_ptr<icsneo::Message> msg) {
	if(msg->type == icsneo::Message::Type::NetworkMutex) {
		auto nmm = std::static_pointer_cast<icsneo::NetworkMutexMessage>(msg);
		if(nmm->event.has_value() && nmm->networks.size())
		{
			auto network = *nmm->networks.begin();
			if(std::find_if(supported_nets.begin(), supported_nets.end(), [network](const icsneo::Network& net){ return net.getNetID() == network; }) != supported_nets.end())
			{
				std::cout << icsneo::Network::GetNetIDString(static_cast<icsneo::Network::NetID>(network)) << ": Mutex " << icsneo::NetworkMutexMessage::GetNetworkMutexEventString(nmm->event.value()) << std::endl;
				network_mutex_states[network] = *nmm->event;
			}
		}
	}
}

static void do_mutexed_operation(std::shared_ptr<icsneo::Device> device, icsneo::Network::NetID network, std::function<void()>&& operation)
{
	// Demonstrate taking a mutex on a single / group of networks
	auto lt = device->lockNetworks({network}, MUTEX_PRIORITY, MUTEX_TTL_MS, icsneo::NetworkMutexType::TxExclusive, on_mutex_event);
	if(lt.has_value()) {
		while(network_mutex_states.find(network) == network_mutex_states.end() || 
			network_mutex_states[network] != icsneo::NetworkMutexEvent::Acquired)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}

		operation();

		device->unlockNetworks({network});
		while(network_mutex_states.find(network) != network_mutex_states.end() && 
			network_mutex_states[network] == icsneo::NetworkMutexEvent::Acquired)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		device->removeMessageCallback(*lt);
	} else {
		std::cout << "Failed to acquire mutex for " << icsneo::Network::GetNetIDString(network) << std::endl;
	}
}

int main() {
	// Print version
	std::cout << "libicsneo version " << icsneo::GetVersion() << std::endl;

	std::cout << "\nFinding devices... " << std::flush;
	auto devices = findDevices(icsneo::DeviceType::FIRE3);
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
		
		supported_nets = device->getSupportedTXNetworks();

		device->goOnline();

		// Demonstrate lock and unlock all networks
		std::cout << "Requesting global exclusive network mutex" << std::endl;
		auto lt = device->lockAllNetworks(MUTEX_PRIORITY, MUTEX_TTL_MS, icsneo::NetworkMutexType::TxExclusive, on_mutex_event);
		if(lt.has_value()) {
			// wait until we have the mutex on all networks
			auto all_nets = device->getSupportedTXNetworks();
			for(auto net = all_nets.begin(); net != all_nets.end();) {
				auto state = network_mutex_states.find(net->getNetID());
				if(state != network_mutex_states.end() && state->second == icsneo::NetworkMutexEvent::Acquired) {
					net = all_nets.erase(net);
				} else {
					++net;
				}
			}
			std::cout << "Acquired mutex for all supported networks on " << device->describe() << std::endl;

			// request release
			if(device->unlockAllNetworks()) {
				std::cout << "Requested release of all networks" << std::endl;
			} else {
				std::cout << "Failed to request release of all networks" << std::endl;
			}

			// wait until we have released the mutex on all networks
			all_nets = device->getSupportedTXNetworks();
			for(auto net = all_nets.begin(); net != all_nets.end();) {
				auto state = network_mutex_states.find(net->getNetID());
				if(state == network_mutex_states.end() || state->second == icsneo::NetworkMutexEvent::Released) {
					net = all_nets.erase(net);
				} else {
					++net;
				}
			}
			std::cout << "released mutex on all supported networks" << std::endl;
			
			device->removeMessageCallback(*lt);
		} else {
			std::cout << "Failed to acquire global mutex" << std::endl;
		}
		
		std::cout << "Doing mutexed operation on DWCAN 01" << std::endl;
		do_mutexed_operation(device, icsneo::Network::NetID::DWCAN_01, [&](){
			auto tx_message = std::make_shared<icsneo::CANMessage>();
			tx_message->arbid = 0x12;
			tx_message->isCANFD = true;
			tx_message->baudrateSwitch = true;
			tx_message->data.resize(64);
			tx_message->network = icsneo::Network::NetID::DWCAN_01;
			constexpr auto txInterval = std::chrono::microseconds(500);
			auto nextRefresh = std::chrono::steady_clock::now() + std::chrono::milliseconds(MUTEX_TTL_MS / 2);
			auto nextTx = std::chrono::steady_clock::now() + txInterval;
			size_t numTx = 0;
			while(numTx < 5000) {
				if(std::chrono::steady_clock::now() < nextTx) {
					continue;
				}
				nextTx = std::chrono::steady_clock::now() + txInterval;
				std::iota(tx_message->data.begin(), tx_message->data.end(), (uint8_t)(numTx & 0xFF));
				if(device->transmit(tx_message)) {
					++numTx;
				}
				// check the mutex periodically - every 200 messages for demonstrative purposes
				if(std::chrono::steady_clock::now() > nextRefresh) {
					auto status = device->getNetworkMutexStatus(icsneo::Network::NetID::DWCAN_01);
					if(!status) {
						std::cout << "failed to poll status of network mutex for DWCAN 01" << std::endl;
					} else {
						std::cout << "DWCAN 01 mutex TTL: " << *status->ttlMs << " ms" << std::endl;
					}
					std::cout << "refreshing mutex on DWCAN 01, current tx count: " << numTx << std::endl;
					(void)device->lockNetworks({icsneo::Network::NetID::DWCAN_01}, MUTEX_PRIORITY, MUTEX_TTL_MS, icsneo::NetworkMutexType::TxExclusive, nullptr);
					nextRefresh = std::chrono::steady_clock::now() + std::chrono::milliseconds(MUTEX_TTL_MS / 2);
				}
			}
		});
		std::cout << "Completed mutexed operation on DWCAN 01" << std::endl;

		
		std::cout << "Doing mutexed operation on Ethernet 03" << std::endl;
		do_mutexed_operation(device, icsneo::Network::NetID::ETHERNET_03, [&](){
			auto tx_message = std::make_shared<icsneo::EthernetMessage>();
			tx_message->data.resize(512);
			// set mac addresses and ethertype
			std::iota(tx_message->data.begin(), tx_message->data.begin() + 14, static_cast<uint8_t>(0x01));
			tx_message->network = icsneo::Network::NetID::ETHERNET_03;
			constexpr auto txInterval = std::chrono::microseconds(250);
			auto nextRefresh = std::chrono::steady_clock::now() + std::chrono::milliseconds(MUTEX_TTL_MS / 2);
			auto nextTx = std::chrono::steady_clock::now() + txInterval;
			size_t numTx = 0;
			while(numTx < 50000) {
				if(std::chrono::steady_clock::now() < nextTx) {
					continue;
				}
				nextTx = std::chrono::steady_clock::now() + txInterval;
				// keep the mac addresses and ethertype constant
				std::iota(tx_message->data.begin() + 14, tx_message->data.end(), (uint8_t)(numTx & 0xFF));
				if(device->transmit(tx_message)) {
					++numTx;
				}
				if(std::chrono::steady_clock::now() > nextRefresh) {
					auto status = device->getNetworkMutexStatus(icsneo::Network::NetID::ETHERNET_03);
					if(!status) {
						std::cout << "failed to poll status of network mutex for Ethernet 03" << std::endl;
					} else {
						std::cout << "Ethernet 03 mutex TTL: " << *status->ttlMs << " ms" << std::endl;
					}
					std::cout << "refreshing mutex on Ethernet 03, current tx count: " << numTx << std::endl;
					(void)device->lockNetworks({icsneo::Network::NetID::ETHERNET_03}, MUTEX_PRIORITY, MUTEX_TTL_MS, icsneo::NetworkMutexType::TxExclusive, nullptr);
					nextRefresh = std::chrono::steady_clock::now() + std::chrono::milliseconds(MUTEX_TTL_MS / 2);
				}
			}
		});
		std::cout << "Completed mutexed operation on Ethernet 03" << std::endl;

		device->close();
	}
	return 0;
}