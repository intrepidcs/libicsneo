#ifndef __VALUECAN4_H_
#define __VALUECAN4_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/stm32.h"

namespace icsneo {

class ValueCAN4 : public Device {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x1101;
	ValueCAN4(neodevice_t neodevice) : Device(neodevice) {
		productId = PRODUCT_ID;
	}

protected:
	static std::shared_ptr<Communication> MakeCommunication(neodevice_t& nd) {
		auto transport = std::unique_ptr<ICommunication>(new STM32(nd));
		auto packetizer = std::make_shared<Packetizer>();
		auto encoder = std::unique_ptr<Encoder>(new Encoder(packetizer));
		encoder->supportCANFD = true;
		auto decoder = std::unique_ptr<Decoder>(new Decoder());
		return std::make_shared<Communication>(std::move(transport), packetizer, std::move(encoder), std::move(decoder));
	}
};

}

#endif