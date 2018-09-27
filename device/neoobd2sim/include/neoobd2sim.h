#ifndef __NEOOBD2SIM_H_
#define __NEOOBD2SIM_H_

#include "device/include/device.h"
#include "device/include/devicetype.h"
#include "platform/include/stm32.h"

namespace icsneo {

class NeoOBD2SIM : public Device {
public:
	// Serial numbers are OS****
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::OBD2_SIM;
	static constexpr const uint16_t PRODUCT_ID = 0x1100;
	NeoOBD2SIM(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::make_shared<STM32>(getWritableNeoDevice());
		auto packetizer = std::make_shared<Packetizer>();
		auto decoder = std::make_shared<Decoder>();
		com = std::make_shared<Communication>(transport, packetizer, decoder);
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<NeoOBD2SIM>(neodevice));

		return found;
	}
};

}

#endif