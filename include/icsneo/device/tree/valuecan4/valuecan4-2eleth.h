#ifndef __VALUECAN4_2EL_ETH_H_
#define __VALUECAN4_2EL_ETH_H_

#ifdef __cplusplus

#include "icsneo/device/tree/valuecan4/valuecan4-2el.h"
#include "icsneo/platform/pcap.h"
#include <string>

namespace icsneo {

class ValueCAN4_2EL_ETH : public ValueCAN4_2EL {
public:
	static constexpr const uint16_t ETH_PRODUCT_ID = 0x000B;

	static std::vector<std::shared_ptr<Device>> Find(const std::vector<PCAP::PCAPFoundDevice>& pcapDevices) {
		std::vector<std::shared_ptr<Device>> found;
		
		for(auto& foundDev : pcapDevices) {
			auto fakedev = std::shared_ptr<ValueCAN4_2EL_ETH>(new ValueCAN4_2EL_ETH({}));
			for (auto& payload : foundDev.discoveryPackets)
				fakedev->com->packetizer->input(payload);
			for (auto& packet : fakedev->com->packetizer->output()) {
				std::shared_ptr<Message> msg;
				if (!fakedev->com->decoder->decode(msg, packet))
					continue; // We failed to decode this packet

				if(!msg || msg->network.getNetID() != Network::NetID::Main51)
					continue; // Not a message we care about
				auto sn = std::dynamic_pointer_cast<SerialNumberMessage>(msg);
				if(!sn)
					continue; // Not a serial number message
				
				if(sn->deviceSerial.length() < 2)
					continue;
				if(sn->deviceSerial.substr(0, 2) != SERIAL_START)
					continue; // Not a ValueCAN4-2EL
				
				auto device = foundDev.device;
				device.serial[sn->deviceSerial.copy(device.serial, sizeof(device.serial))] = '\0';
				found.push_back(std::shared_ptr<ValueCAN4_2EL_ETH>(new ValueCAN4_2EL_ETH(std::move(device))));
				break;
			}
		}

		return found;
	}

	static const std::vector<Network>& GetSupportedNetworks() {
		static std::vector<Network> supportedNetworks = {
			Network::NetID::HSCAN,
			Network::NetID::HSCAN2,

			// No Network::NetID::Ethernet, since we're communicating over it instead
		};
		return supportedNetworks;
	}

protected:
	virtual void setupSupportedRXNetworks(std::vector<Network>& rxNetworks) override {
		for(auto& netid : GetSupportedNetworks())
			rxNetworks.emplace_back(netid);
	}

	// The supported TX networks are the same as the supported RX networks for this device
	virtual void setupSupportedTXNetworks(std::vector<Network>& txNetworks) override { setupSupportedRXNetworks(txNetworks); }

private:
	ValueCAN4_2EL_ETH(neodevice_t neodevice) : ValueCAN4_2EL(neodevice) {
		initialize<CDCACM, ValueCAN4_2ELSettings>();
		productId = ETH_PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif