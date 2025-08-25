#ifndef __RADCOMET_H_
#define __RADCOMET_H_

#ifdef __cplusplus

#include "icsneo/device/tree/radcomet/radcometbase.h"
#include "icsneo/device/tree/radcomet/radcometsettings.h"

namespace icsneo {

class RADComet : public RADCometBase {
public:

	// Serial numbers start with RC
	// USB PID is 0x1207, standard driver is DXX
	// Ethernet MAC allocation is 0x1D, standard driver is Raw
	ICSNEO_FINDABLE_DEVICE_BY_SERIAL_RANGE(RADComet, DeviceType::RADComet, "RC0000", "RC0299");

	std::string getProductName() const override {
		return "RAD-Comet";
	}

protected:
	RADComet(neodevice_t neodevice, const driver_factory_t& makeDriver) : RADCometBase(neodevice) {
		initialize<RADCometSettings>(makeDriver);
	}

};

}

#endif // __cplusplus

#endif
