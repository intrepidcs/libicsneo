#ifndef __RADGALAXY_H_
#define __RADGALAXY_H_

#include "device/include/device.h"
#include "platform/include/pcap.h"
#include "communication/include/packetizer.h"
#include "communication/include/decoder.h"

namespace icsneo {

class RADGalaxy : public Device {
public:
	// Serial numbers start with RG
	static constexpr const char* PRODUCT_NAME = "RADGalaxy";
	static constexpr const uint16_t PRODUCT_ID = 0x0003;
	RADGalaxy(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::make_shared<PCAP>(getWritableNeoDevice());
		auto packetizer = std::make_shared<Packetizer>();
		packetizer->disableChecksum = true;
		packetizer->align16bit = false;
		auto decoder = std::make_shared<Decoder>();
		com = std::make_shared<Communication>(transport, packetizer, decoder);
		setProductName(PRODUCT_NAME);
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : PCAP::FindByProduct(PRODUCT_ID)) {
			{
				strncpy(neodevice.serial, SERIAL_FIND_ON_OPEN, sizeof(neodevice.serial));
				neodevice.serial[sizeof(neodevice.serial) - 1] = '\0';
				auto device = std::make_shared<RADGalaxy>(neodevice);
				if(!device->open()) // We will get the serial number on open
					continue; // If the open failed, we won't display the device as an option to connect to
				strncpy(neodevice.serial, device->getNeoDevice().serial, sizeof(neodevice.serial));
				neodevice.serial[sizeof(neodevice.serial) - 1] = '\0';
			}
			found.push_back(std::make_shared<RADGalaxy>(neodevice));
		}

		return found;
	}
};

}

#endif