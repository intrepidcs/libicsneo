#ifndef __NEOVIFIRE2ETH_H_
#define __NEOVIFIRE2ETH_H_

#include "icsneo/device/neovifire2/neovifire2.h"
#include "icsneo/platform/pcap.h"
#include "icsneo/device/neovifire2/neovifire2settings.h"
#include <memory>

namespace icsneo {

class NeoVIFIRE2ETH : public NeoVIFIRE2 {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x0004;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;
		
		for(auto& foundDev : PCAP::FindAll()) {
			auto fakedev = std::shared_ptr<NeoVIFIRE2ETH>(new NeoVIFIRE2ETH({}));
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
					continue; // Not a FIRE 2
				
				foundDev.device.serial[sn->deviceSerial.copy(foundDev.device.serial, sizeof(foundDev.device.serial))] = '\0';
				found.push_back(std::make_shared<NeoVIFIRE2ETH>(foundDev.device));
				break;
			}
		}

		return found;
	}

	NeoVIFIRE2ETH(neodevice_t neodevice) : NeoVIFIRE2(neodevice) {
		initialize<PCAP, NeoVIFIRE2Settings>();
		productId = PRODUCT_ID;
	}

protected:
	virtual void setupSettings(IDeviceSettings* ssettings) {
		// TODO Check firmware version, old firmwares will reset Ethernet settings on settings send
		ssettings->readonly = true;
	}
};

}

#endif