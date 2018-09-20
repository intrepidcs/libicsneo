#ifndef __NEOVIFIRE2_H_
#define __NEOVIFIRE2_H_

#include "device/include/device.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class NeoVIFIRE2 : public Device {
public:
	static constexpr const char* PRODUCT_NAME = "neoVI FIRE 2";
	NeoVIFIRE2(neodevice_t neodevice) : Device(neodevice) {
		setProductName(PRODUCT_NAME);
	}
};

};

#endif