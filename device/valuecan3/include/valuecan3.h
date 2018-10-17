#ifndef __VALUECAN3_H_
#define __VALUECAN3_H_

#include "device/include/device.h"
#include "device/include/devicetype.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class ValueCAN3 : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN3;
	static constexpr const uint16_t PRODUCT_ID = 0x0601;
	ValueCAN3(neodevice_t neodevice) : Device(neodevice) {
		auto transport = std::unique_ptr<ICommunication>(new FTDI(getWritableNeoDevice()));
		auto packetizer = std::make_shared<Packetizer>();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		com = std::make_shared<Communication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}

	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.push_back(std::make_shared<ValueCAN3>(neodevice));

		return found;
	}
};

}

#endif