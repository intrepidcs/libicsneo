#ifndef __RADMOON2ZL_H_
#define __RADMOON2ZL_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radmoon2/radmoon2base.h"
#include "icsneo/device/tree/radmoon2/radmoon2settings.h"

namespace icsneo {

class RADMoon2ZL : public RADMoon2Base {
public:
	// Serial numbers start with RN
	// USB PID is 0x110C, standard driver is CDCACM
	ICSNEO_FINDABLE_DEVICE(RADMoon2ZL, DeviceType::RADMoon2, "RN");

	uint8_t getPhyAddrOrPort() const override { return 1; }

	bool supportsTC10() const override { return true; }

protected:
	RADMoon2ZL(neodevice_t neodevice, const driver_factory_t& makeDriver) : RADMoon2Base(neodevice) {
		initialize<RADMoon2Settings>(makeDriver);
	}
};

}

#endif // __cplusplus

#endif