#ifndef __VALUECAN3_H_
#define __VALUECAN3_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"
#include "icsneo/device/valuecan3/valuecan3settings.h"

namespace icsneo {

class ValueCAN3 : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::VCAN3;
	static constexpr const uint16_t PRODUCT_ID = 0x0601;
	static std::vector<std::shared_ptr<Device>> Find() {
		std::vector<std::shared_ptr<Device>> found;

		for(auto neodevice : FTDI::FindByProduct(PRODUCT_ID))
			found.emplace_back(new ValueCAN3(neodevice));

		return found;
	}

private:
	ValueCAN3(neodevice_t neodevice) : Device(neodevice) {
		initialize<FTDI, ValueCAN3Settings>();
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}
};

}

#endif