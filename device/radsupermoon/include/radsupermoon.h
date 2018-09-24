#ifndef __RADSUPERMOON_H_
#define __RADSUPERMOON_H_

#include "device/include/device.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class RADSupermoon : public Device {
public:
	// Serial numbers start with VV
	static constexpr const char* PRODUCT_NAME = "RADSupermoon";
	static constexpr const uint16_t PRODUCT_ID = 0x1201;
	RADSupermoon(neodevice_t neodevice) : Device(neodevice) {
		com = std::make_shared<Communication>(std::make_shared<FTDI>(getWritableNeoDevice()));
		com->setAlign16Bit(false);
		setProductName(PRODUCT_NAME);
		productId = PRODUCT_ID;
	}
	// RSM does not connect at all yet (needs FTDI D3xx driver, not the 2xx compatible one)

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<RADSupermoon>(neodevice));

		return found;
	}
};

}

#endif