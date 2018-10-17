#ifndef __NEOOBD2PRO_H_
#define __NEOOBD2PRO_H_

#include "device/include/device.h"
#include "device/include/devicetype.h"
#include "platform/include/stm32.h"

namespace icsneo {

class NeoOBD2PRO : public Device {
public:
	// Serial numbers are NP****
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::OBD2_PRO;
	static constexpr const uint16_t PRODUCT_ID = 0x1103;
	NeoOBD2PRO(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::unique_ptr<ICommunication>(new STM32(getWritableNeoDevice()));
		auto packetizer = std::make_shared<Packetizer>();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		com = std::make_shared<Communication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
		getWritableNeoDevice().type = DEVICE_TYPE;
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