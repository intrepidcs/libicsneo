#ifndef __RADSTAR2_H_
#define __RADSTAR2_H_

#include "device/include/device.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class RADStar2 : public Device {
public:
	// Serial numbers start with RS
	static constexpr const char* PRODUCT_NAME = "RADStar 2";
	static constexpr const uint16_t PRODUCT_ID = 0x0005;
	RADStar2(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::make_shared<FTDI>(getWritableNeoDevice());
		auto packetizer = std::make_shared<Packetizer>();
		auto decoder = std::make_shared<Decoder>();
		com = std::make_shared<Communication>(transport, packetizer, decoder);
		setProductName(PRODUCT_NAME);
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<RADStar2>(neodevice));

		return found;
	}
};

};

#endif