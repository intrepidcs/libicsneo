#ifndef __NEOOBD2PRO_H_
#define __NEOOBD2PRO_H_

#include "device/include/device.h"
#include "platform/include/stm32.h"

namespace icsneo {

class NeoOBD2PRO : public Device {
public:
	// Serial numbers are NP****
	static constexpr const char* PRODUCT_NAME = "neoOBD2 PRO";
	static constexpr const uint16_t PRODUCT_ID = 0x1103;
	NeoOBD2PRO(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::make_shared<STM32>(getWritableNeoDevice());
		auto packetizer = std::make_shared<Packetizer>();
		auto decoder = std::make_shared<Decoder>();
		com = std::make_shared<Communication>(transport, packetizer, decoder);
		setProductName(PRODUCT_NAME);
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<NeoOBD2PRO>(neodevice));

		return found;
	}
};

}

#endif