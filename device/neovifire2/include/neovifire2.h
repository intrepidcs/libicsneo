#ifndef __NEOVIFIRE2_H_
#define __NEOVIFIRE2_H_

#include "device/include/device.h"
#include "device/include/devicetype.h"
#include "platform/include/ftdi.h"

namespace icsneo {

class NeoVIFIRE2 : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::FIRE2;
	NeoVIFIRE2(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
	}
};

}

#endif