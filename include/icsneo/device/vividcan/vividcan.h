#ifndef __VIVIDCAN_H_
#define __VIVIDCAN_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/stm32.h"

namespace icsneo {

class VividCAN : public Device {
public:
	// Serial numbers start with VV
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VividCAN;
	static constexpr const uint16_t PRODUCT_ID = 0x1102;
	VividCAN(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::unique_ptr<ICommunication>(new STM32(getWritableNeoDevice()));
		auto packetizer = std::make_shared<Packetizer>();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		com = std::make_shared<Communication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}

	bool goOnline() { return false; }

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<VividCAN>(neodevice));

		return found;
	}
};

}

#endif