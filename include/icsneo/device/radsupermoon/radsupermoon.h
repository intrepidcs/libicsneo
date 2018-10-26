#ifndef __RADSUPERMOON_H_
#define __RADSUPERMOON_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"

namespace icsneo {

class RADSupermoon : public Device {
public:
	// RSM does not connect at all yet (needs FTDI D3xx driver, not the 2xx compatible one)
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADSupermoon;
	static constexpr const uint16_t PRODUCT_ID = 0x1201;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.emplace_back(new RADSupermoon(neodevice));

		return found;
	}

private:
	RADSupermoon(neodevice_t neodevice) : Device(neodevice) {
		initialize<FTDI>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}
};

}

#endif