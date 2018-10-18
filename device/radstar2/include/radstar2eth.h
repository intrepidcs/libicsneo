#ifndef __RADSTAR2ETH_H_
#define __RADSTAR2ETH_H_

#include "device/radstar2/include/radstar2.h"
#include "communication/include/network.h"
#include "communication/message/include/serialnumbermessage.h"
#include "platform/include/pcap.h"

namespace icsneo {

class RADStar2ETH : public RADStar2 {
public:
	static std::shared_ptr<Packetizer> MakePacketizer() {
		auto packetizer = std::make_shared<Packetizer>();
		packetizer->disableChecksum = true;
		packetizer->align16bit = false;
		return packetizer;
	}

	// Serial numbers start with RS
	RADStar2ETH(neodevice_t neodevice) : RADStar2(neodevice) {
		auto transport = std::unique_ptr<ICommunication>(new PCAP(getWritableNeoDevice()));
		auto packetizer = MakePacketizer();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		com = std::make_shared<Communication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;
		
		for(auto& foundDev : PCAP::FindAll()) {
			auto packetizer = MakePacketizer();
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
					continue; // Not a RADStar2
				
				foundDev.device.serial[sn->deviceSerial.copy(foundDev.device.serial, sizeof(foundDev.device.serial))] = '\0';
				found.push_back(std::make_shared<RADStar2ETH>(foundDev.device));
				break;
			}
		}

		return found;
	}
};

}

#endif