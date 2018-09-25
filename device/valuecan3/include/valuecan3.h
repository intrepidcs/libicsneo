#ifndef __VALUECAN3_H_
#define __VALUECAN3_H_

#include "device/include/device.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class ValueCAN3 : public Device {
public:
	static constexpr const char* PRODUCT_NAME = "ValueCAN 3";
	static constexpr const uint16_t PRODUCT_ID = 0x0601;
	ValueCAN3(neodevice_t neodevice) : Device(neodevice) {
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
			found.push_back(std::make_shared<ValueCAN3>(neodevice));

		return found;
	}
};

};

#endif