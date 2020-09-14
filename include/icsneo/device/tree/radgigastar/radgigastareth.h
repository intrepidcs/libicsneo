#ifndef __RADGIGASTAR_ETH_H_
#define __RADGIGASTAR_ETH_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radgigastar/radgigastar.h"
#include "icsneo/platform/pcap.h"

namespace icsneo {

class RADGigastarETH : public RADGigastar {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x000F;
	static std::vector<std::shared_ptr<Device>> Find(const std::vector<PCAP::PCAPFoundDevice>& pcapDevices) {
		std::vector<std::shared_ptr<Device>> found;
		
		for(auto& foundDev : pcapDevices) {
			auto fakedev = std::shared_ptr<RADGigastarETH>(new RADGigastarETH({}));
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
					continue; // Not a RADGigastar
				
				auto device = foundDev.device;
				device.serial[sn->deviceSerial.copy(device.serial, sizeof(device.serial))] = '\0';
				found.push_back(std::shared_ptr<RADGigastarETH>(new RADGigastarETH(std::move(device))));
				break;
			}
		}

		return found;
	}

private:
	RADGigastarETH(neodevice_t neodevice) : RADGigastar(neodevice) {
		initialize<PCAP, RADGigastarSettings>();
		productId = PRODUCT_ID;
	}
};

}

#endif // __cplusplus

#endif