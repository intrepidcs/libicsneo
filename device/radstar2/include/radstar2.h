#ifndef __RADSTAR2_H_
#define __RADSTAR2_H_

#include "device/include/device.h"
#include "device/include/devicetype.h"

namespace icsneo {

class RADStar2 : public Device {
public:
	// Serial numbers start with RS
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADStar2;
	static constexpr const uint16_t PRODUCT_ID = 0x0005;
	static constexpr const char* SERIAL_START = "RS";
	RADStar2(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}
};

}

#endif