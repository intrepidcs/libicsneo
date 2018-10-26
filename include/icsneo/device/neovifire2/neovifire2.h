#ifndef __NEOVIFIRE2_H_
#define __NEOVIFIRE2_H_

#include "icsneo/device/device.h"
#include "icsneo/device/devicetype.h"
#include "icsneo/platform/ftdi.h"

namespace icsneo {

class NeoVIFIRE2 : public Device {
public:
	static constexpr DeviceType::Enum DEVICE_TYPE = DeviceType::FIRE2;
	static constexpr const char* SERIAL_START = "CY";

protected:
	NeoVIFIRE2(neodevice_t neodevice) : Device(neodevice) {
		getWritableNeoDevice().type = DEVICE_TYPE;
	}

	virtual void setupEncoder(Encoder* encoder) override {
		Device::setupEncoder(encoder);
		encoder->supportCANFD = true;
	}
};

}

#endif