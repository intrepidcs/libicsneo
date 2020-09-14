#ifndef __VALUECAN4_H_
#define __VALUECAN4_H_

#ifdef __cplusplus

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"

namespace icsneo {

class ValueCAN4 : public Device {
public:
	// All ValueCAN 4 devices share a USB PID
	static constexpr const uint16_t USB_PRODUCT_ID = 0x1101;

protected:
	virtual void setupEncoder(Encoder& encoder) override {
		Device::setupEncoder(encoder);
		encoder.supportCANFD = true;
	}

	ValueCAN4(neodevice_t neodevice) : Device(neodevice) {}
};

}

#endif // __cplusplus

#endif