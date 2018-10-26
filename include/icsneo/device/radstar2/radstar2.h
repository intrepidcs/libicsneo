#ifndef __RADSTAR2_H_
#define __RADSTAR2_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"

namespace icsneo {

class RADStar2 : public Device {
public:
	// Serial numbers start with RS
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::RADStar2;
	static constexpr const uint16_t PRODUCT_ID = 0x0005;
	static constexpr const char* SERIAL_START = "RS";

protected:
	virtual void setupPacketizer(Packetizer* packetizer) override {
		packetizer->disableChecksum = true;
		packetizer->align16bit = false;
	}
	
	RADStar2(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
		productId = PRODUCT_ID;
	}
};

}

#endif