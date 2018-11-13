#ifndef __RADSTAR2ETH_H_
#define __RADSTAR2ETH_H_

#include "icsneo/device/radstar2/radstar2.h"
#include "icsneo/communication/network.h"
#include "icsneo/communication/message/serialnumbermessage.h"
#include "icsneo/platform/pcap.h"

namespace icsneo {

class RADStar2ETH : public RADStar2 {
public:
	// Serial numbers start with RS
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;
		
		for(auto& foundDev : PCAP::FindAll()) {
			auto fakedev = std::shared_ptr<RADStar2ETH>(new RADStar2ETH({}));
			for(auto& payload : foundDev.discoveryPackets)
				fakedev->com->packetizer->input(payload);
			for(auto& packet : fakedev->com->packetizer->output()) {
				std::shared_ptr<Message> msg;
				if(!fakedev->com->decoder->decode(msg, packet))
					continue; // We failed to decode this packet

				if(!msg || msg->network.getNetID() != Network::NetID::Main51)
					continue; // Not a message we care about
				auto sn = std::dynamic_pointer_cast<SerialNumberMessage>(msg);
				if(!sn)
					continue; // Not a serial number message
				
				if(sn->deviceSerial.length() < 2)
					continue;
				if(sn->deviceSerial.substr(0, 2) != SERIAL_START)
					continue; // Not a RADStar2
				
				foundDev.device.serial[sn->deviceSerial.copy(foundDev.device.serial, sizeof(foundDev.device.serial))] = '\0';
				found.push_back(std::make_shared<RADStar2ETH>(foundDev.device));
				break;
			}
		}

		return found;
	}

	RADStar2ETH(neodevice_t neodevice) : RADStar2(neodevice) {
		initialize<PCAP>();
	}
};

}

#endif