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
	NeoVIFIRE2ETH(neodevice_t neodevice) : NeoVIFIRE2(neodevice) {
		com = MakeCommunicaiton(std::unique_ptr<ICommunication>(new PCAP(getWritableNeoDevice())));
		settings = std::unique_ptr<IDeviceSettings>(new NeoVIFIRE2Settings(com));
		settings->readonly = true;
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;
		
		for(auto& foundDev : PCAP::FindAll()) {
			auto packetizer = std::make_shared<Packetizer>();
			auto decoder = std::unique_ptr<Decoder>(new Decoder());
			for(auto& payload : foundDev.discoveryPackets)
				packetizer->input(payload);
			for(auto& packet : packetizer->output()) {
				std::shared_ptr<Message> msg;
				if(!decoder->decode(msg, packet))
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
};

}

#endif