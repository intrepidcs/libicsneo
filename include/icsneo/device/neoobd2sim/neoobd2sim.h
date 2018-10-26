#ifndef __NEOOBD2SIM_H_
#define __NEOOBD2SIM_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/stm32.h"

namespace icsneo {

class NeoOBD2SIM : public Device {
public:
	// Serial numbers are OS****
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::OBD2_SIM;
	static constexpr const uint16_t PRODUCT_ID = 0x1100;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : STM32::FindByProduct(PRODUCT_ID))
			found.emplace_back(new NeoOBD2SIM(neodevice));

		return found;
	}

private:
	NeoOBD2SIM(neodevice_t neodevice) : Device(neodevice) {
		initialize<STM32>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}
};

}

#endif