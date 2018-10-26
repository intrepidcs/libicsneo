#ifndef __VALUECAN4_H_
#define __VALUECAN4_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/stm32.h"

namespace icsneo {

class ValueCAN4 : public Device {
public:
	static constexpr const uint16_t PRODUCT_ID = 0x1101;

protected:
	virtual void setupEncoder(Encoder* encoder) override {
		encoder->supportCANFD = true;
	}

	ValueCAN4(neodevice_t neodevice) : Device(neodevice) {
		productId = PRODUCT_ID;
	}
};

}

#endif