#ifndef __RADSUPERMOON_H_
#define __RADSUPERMOON_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"

namespace icsneo {

class RADSupermoon : public Device {
public:
	// Serial numbers start with VV
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADSupermoon;
	static constexpr const uint16_t PRODUCT_ID = 0x1201;
	RADSupermoon(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::unique_ptr<ICommunication>(new FTDI(getWritableNeoDevice()));
		auto packetizer = std::make_shared<Packetizer>();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		com = std::make_shared<Communication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
		getWritableNeoDevice().type = DEVICE_TYPE;
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