#ifndef __RADGALAXY_H_
#define __RADGALAXY_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/pcap.h"
#include "icsneo/communication/packetizer.h"
#include "icsneo/communication/decoder.h"

namespace icsneo {

class RADGalaxy : public Device {
public:
	// Serial numbers start with RG
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADGalaxy;
	static constexpr const uint16_t PRODUCT_ID = 0x0003;
	static constexpr const char* SERIAL_START = "RG";
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;
		
		for(auto& foundDev : PCAP::FindAll()) {
			auto fakedev = std::shared_ptr<RADGalaxy>(new RADGalaxy({}));
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
					continue; // Not a RADGalaxy
				
				foundDev.device.serial[sn->deviceSerial.copy(foundDev.device.serial, sizeof(foundDev.device.serial))] = '\0';
				found.emplace_back(new RADGalaxy(foundDev.device));
				break;
			}
		}

		return found;
	}

	RADGalaxy(neodevice_t neodevice) : Device(neodevice) {
		initialize<PCAP>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}

protected:
	void setupPacketizer(Packetizer* packetizer) override {
		packetizer->disableChecksum = true;
		packetizer->align16bit = false;
	}
};

}

#endif