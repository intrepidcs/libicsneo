#ifndef __RADGALAXY_H_
#define __RADGALAXY_H_

#include "device/include/device.h"
#include "platform/include/pcap.h"

namespace icsneo {

class RADGalaxy : public Device {
public:
	// Serial numbers start with RG
	static constexpr const char* PRODUCT_NAME = "RADGalaxy";
	static constexpr const uint16_t PRODUCT_ID = 0x0003;
	RADGalaxy(neodevice_t neodevice) : Device(neodevice) {
		com = std::make_shared<Communication>(std::make_shared<PCAP>(getWritableNeoDevice()));
		setProductName(PRODUCT_NAME);
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : PCAP::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<RADGalaxy>(neodevice));

		return found;
	}
};

}

#endif