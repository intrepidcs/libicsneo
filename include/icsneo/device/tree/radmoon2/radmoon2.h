#ifndef __RADMOON2_H_
#define __RADMOON2_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radmoon2/radmoon2base.h"
#include "icsneo/device/tree/radmoon2/radmoon2settings.h"

namespace icsneo {

class RADMoon2 : public RADMoon2Base {
public:
	// Serial numbers start with RM
	// USB PID is 0x1202, standard driver is FTDI3
	ICSNEO_FINDABLE_DEVICE(RADMoon2, DeviceType::RADMoon2, "RM");

	uint8_t getPhyAddrOrPort() const override { return 6; };

protected:
	RADMoon2(neodevice_t neodevice, const driver_factory_t& makeDriver) : RADMoon2Base(neodevice) {
		initialize<RADMoon2Settings>(makeDriver);
	}

	void setupDecoder(Decoder& decoder) override {
		Device::setupDecoder(decoder);
		decoder.timestampResolution = 10; // Timestamps are in 10ns increments instead of the usual 25ns
	}

	void setupPacketizer(Packetizer& packetizer) override {
		Device::setupPacketizer(packetizer);
		packetizer.disableChecksum = true;
		packetizer.align16bit = false;
	}
};

}

#endif // __cplusplus

#endif